#include "PreRTS.h"

#include "Common/GlobalData.h"
#include "W3DDevice/GameClient/HeightMap.h"
#include "W3DDevice/GameClient/W3DDisplay.h"
#include "w3d_shader_manager_cpu_types.h"
#include "W3DDevice/GameClient/W3DShaderManager.h"
#include "W3DDevice/GameClient/W3DShroud.h"
#include "W3DDevice/GameClient/W3DTerrainVisual.h"
#include "WW3D2/camera.h"
#include "original_gpu_edge.h"
#include "tree_shroud_projection_cpu.h"
#include "zh/renderer/recording_device.h"
#include "zh/platform/bgfx_device.h"

#include <cstdlib>
#include <cstdio>
#include <memory>
#include <stdexcept>
#include <array>
#include <algorithm>
#include <limits>
#include <cstring>
#include <type_traits>
#include <utility>

struct W3DShroudGeneratedProbeAccess {
    static void rejectNextCommit(W3DShroud& owner) { owner.m_failContentCommit=TRUE; }
};
template<class Owner,class=void> struct HasPublicCandidateWithdrawal:std::false_type {};
template<class Owner> struct HasPublicCandidateWithdrawal<Owner,std::void_t<decltype(
    std::declval<Owner&>().withdraw_candidate_texture(static_cast<TextureBaseClass*>(nullptr),
        zh::renderer::TextureHandle{}))>>:std::true_type {};
static_assert(!HasPublicCandidateWithdrawal<zh::original_runtime::OriginalGpuEdge>::value,
    "shroud candidate withdrawal must not expose a public caller surface");

namespace {
void require(bool value, const char *message)
{
	if (!value) throw std::runtime_error(message);
}

template <typename Operation>
bool rejected(Operation operation)
{
	try {
		operation();
	} catch (const std::runtime_error &) {
		return true;
	}
	return false;
}

std::array<unsigned char,4> expectedPixel(W3DShroudLevel level,unsigned color,Bool fog)
{
    level=std::max(level,TheGlobalData->m_shroudAlpha);
    if (fog) return {0,0,0,static_cast<unsigned char>(((255-level)>>4)*17)};
    const auto channel=[&](unsigned byte,unsigned shift,unsigned maximum) {
        const unsigned scaled=level==255 ? 255 : static_cast<unsigned>(
            static_cast<Real>(level)*(static_cast<Real>(byte)/255.0f));
        return static_cast<unsigned char>(((scaled>>shift)*255+maximum/2)/maximum);
    };
    return {channel(color&255,3,31),channel((color>>8)&255,2,63),channel((color>>16)&255,3,31),255};
}
void exactPixels(zh::renderer::RecordingGpuDevice& device,
    zh::original_runtime::OriginalGpuEdge& edge,W3DShroud* shroud,W3DShroudLevel border,Bool fog)
{
    const auto pixels=device.texture_bytes(edge.texture_handle(shroud->getShroudTexture()));
    const unsigned color=static_cast<unsigned>(TheGlobalData->m_shroudColor.getAsInt());
    require(pixels.size()==static_cast<std::size_t>(shroud->getTextureWidth())*shroud->getTextureHeight()*4,
        "original shroud complete pixel extent mismatch");
    for(Int y=0;y<shroud->getTextureHeight();++y) for(Int x=0;x<shroud->getTextureWidth();++x) {
        const auto level=x && y && x<=shroud->getNumShroudCellsX() && y<=shroud->getNumShroudCellsY()
            ? shroud->getShroudLevel(x-1,y-1) : border;
        const auto expected=expectedPixel(level,color,fog);
        require(!std::memcmp(pixels.data()+(y*shroud->getTextureWidth()+x)*4,expected.data(),4),
            "original shroud quantized cell/border pixel mismatch");
    }
}

bool sameStage(const DX8Wrapper::SourceStateSnapshot& first,const DX8Wrapper::SourceStateSnapshot& second)
{
    if(first.stages!=second.stages || first.render!=second.render || first.transforms.size()!=second.transforms.size()) return false;
    auto a=first.transforms.begin(),b=second.transforms.begin();
    for(;a!=first.transforms.end();++a,++b)
        if(a->first!=b->first || std::memcmp(&a->second,&b->second,sizeof(Matrix4x4))) return false;
    return true;
}
class ShroudBounds final:public W3DShroud {
public:
    void epoch(unsigned long long value) {m_contentEpoch=value;}
    void dimensions(Int x,Int y) {m_numCellsX=x;m_numCellsY=y;m_dstTextureWidth=x+2;m_dstTextureHeight=y+2;}
    void generation(unsigned long long value) {m_contentGeneration=value;}
    void cellWidth(Real value) {m_cellWidth=value;}
};
void contentBounds(zh::renderer::RecordingGpuDevice& device,
    zh::original_runtime::OriginalGpuEdge& edge,WorldHeightMap* map)
{
    ShroudBounds bounded;bounded.init(map,MAP_XY_FACTOR*2,MAP_XY_FACTOR*3);
    const auto x=bounded.getNumShroudCellsX(),y=bounded.getNumShroudCellsY();
    const auto initial=bounded.getShroudLevel(0,0);
    const auto epoch=bounded.contentEpoch();
    const auto counts=device.resource_counts();const auto revision=edge.source_revision();
    bounded.epoch(std::numeric_limits<unsigned long long>::max());
    require(rejected([&]{bounded.setShroudLevel(0,0,255);}) && bounded.getShroudLevel(0,0)==initial
        && !bounded.acceptedContentEpoch(),"original shroud epoch overflow mutated cell");
    bounded.epoch(epoch);CameraClass camera;
    for(unsigned boundary=0;boundary<4;++boundary) {
        if(boundary==0) bounded.dimensions(16382,y); // pitch 65536: native bound+1.
        if(boundary==1) bounded.dimensions(8190,4095); // complete BGRA payload >64MiB.
        if(boundary==2) bounded.cellWidth(std::numeric_limits<float>::infinity());
        if(boundary==3) bounded.cellWidth(std::numeric_limits<float>::quiet_NaN());
        require(rejected([&]{bounded.render(&camera);}) && !bounded.getShroudTexture()
            && device.resource_counts()==counts && edge.source_revision()==revision
            && bounded.contentEpoch()==epoch && !bounded.acceptedContentEpoch(),
            "original bounded shroud preflight mutated publication/epoch");
        bounded.dimensions(x,y);bounded.cellWidth(MAP_XY_FACTOR*2);
    }
    const auto color=TheWritableGlobalData->m_shroudColor;
    TheWritableGlobalData->m_shroudColor.red=std::numeric_limits<float>::quiet_NaN();
    require(rejected([&]{bounded.render(&camera);}) && !bounded.getShroudTexture()
        && device.resource_counts()==counts && edge.source_revision()==revision,
        "original invalid shroud color changed state");
    TheWritableGlobalData->m_shroudColor=color;
    bounded.render(&camera);
    require(bounded.hasAcceptedContent(),"original bounded shroud valid retry failed");
    bounded.generation(edge.generation()+1);
    require(!bounded.hasAcceptedContent(),"original stale shroud content generation accepted");
    bounded.generation(edge.generation());
    require(bounded.hasAcceptedContent(),"original exact shroud generation retry failed");
    bounded.cellWidth(std::numeric_limits<float>::quiet_NaN());
    require(!bounded.hasAcceptedContent(),"original malformed accepted shroud extent stayed ready");
    bounded.cellWidth(MAP_XY_FACTOR*2);
    bounded.getShroudTexture()->Apply_Gpu_Texture(WW3D_FORMAT_A8R8G8B8,1,1);
    require(!bounded.hasAcceptedContent(),"original changed resident source extent stayed ready");
    bounded.getShroudTexture()->Apply_Gpu_Texture(WW3D_FORMAT_A8R8G8B8,bounded.getTextureWidth(),bounded.getTextureHeight());
    require(bounded.hasAcceptedContent(),"original exact resident source extent retry failed");
    bounded.reset();
    require(!bounded.contentEpoch() && !bounded.acceptedContentEpoch() && !bounded.hasAcceptedContent()
        && device.resource_counts()==counts,"original shroud reset baseline retained epoch/native owner");
}
void bindingControls(zh::renderer::RecordingGpuDevice& device,
    zh::original_runtime::OriginalGpuEdge& edge,W3DShroud* shroud)
{
    Matrix4x4 view(true);
    view[0][0]=0;view[0][1]=-1;view[1][0]=1;view[1][1]=0;
    view[0][3]=3;view[1][3]=-4;view[2][3]=2;
    // Independently multiply the native row-vector inverse * offset * scale,
    // then transpose. Unlike shipping full-map upload, this checks shifted origin.
    const double origin_x=31,origin_y=-23;
    Matrix4x4 native_inverse(true),native_offset(true),native_scale(true);
    native_inverse[0][0]=0;native_inverse[0][1]=-1;native_inverse[1][0]=1;native_inverse[1][1]=0;
    native_inverse[3][0]=4;native_inverse[3][1]=3;native_inverse[3][2]=-2;
    native_offset[3][0]=-origin_x+shroud->getCellWidth();
    native_offset[3][1]=-origin_y+shroud->getCellHeight();
    native_scale[0][0]=1.0f/(shroud->getCellWidth()*shroud->getTextureWidth());
    native_scale[1][1]=1.0f/(shroud->getCellHeight()*shroud->getTextureHeight());
    const auto native_formula=((native_inverse*native_offset)*native_scale).Transpose();
    const auto shifted=zh::original_runtime::detail::tree_shroud_projection(view,shroud->getCellWidth(),
        shroud->getCellHeight(),shroud->getTextureWidth(),shroud->getTextureHeight(),origin_x,origin_y);
    for(unsigned row=0;row<4;++row) for(unsigned col=0;col<4;++col)
        require(std::fabs(native_formula[row][col]-shifted[row][col])<1e-6f,
            "original shifted shroud algebra differs from native row formula");
    const float half_max=std::numeric_limits<float>::max()/2;
    const float beyond_half=std::nextafter(half_max,std::numeric_limits<float>::infinity());
    const auto maximum=zh::original_runtime::detail::tree_shroud_projection(Matrix4x4(true),half_max,1,2,1,0,0);
    const auto maximum_offset=zh::original_runtime::detail::tree_shroud_projection(Matrix4x4(true),half_max,1,1,1,-half_max,0);
    require(std::isfinite(maximum[0][0]) && maximum[0][0]>0 && std::isfinite(maximum_offset[0][3]),
        "original exact maximum native Real shroud intermediates rejected");
    require(rejected([&]{zh::original_runtime::detail::tree_shroud_projection(Matrix4x4(true),beyond_half,1,2,1,0,0);})
        && rejected([&]{zh::original_runtime::detail::tree_shroud_projection(Matrix4x4(true),half_max,1,1,1,-beyond_half,0);}),
        "original native Real product/offset bound+1 admitted");
    DX8Wrapper::Set_Transform(D3DTS_VIEW,view);
    DX8Wrapper::Set_Texture(1,shroud->getShroudTexture());
    shroud->getShroudTexture()->Apply(1);
    const auto pixels=device.texture_bytes(edge.texture_handle(shroud->getShroudTexture()));
    const auto original=DX8Wrapper::Inspect_Source_State();
    const auto revision=edge.source_revision();const auto resources=device.resource_counts();
    const auto refs=shroud->getShroudTexture()->Num_Refs();
    for(unsigned fault=0;fault<5;++fault) {
        if(fault==0) device.fail_next_transaction_checkpoint();
        if(fault==1) edge.fail_next_source_stage_commit();
        if(fault==2) edge.fail_source_stage_allocation_after(0);
        if(fault==3) device.fail_transaction_operation_after(0);
        if(fault==4) device.fail_next_sampler_create();
        require(rejected([&]{W3DShaderManager::setShroudTex(1);})
            && sameStage(original,DX8Wrapper::Inspect_Source_State())
            && edge.source_revision()==revision && device.resource_counts()==resources
            && shroud->getShroudTexture()->Num_Refs()==refs && !edge.queued_source_reference_count()
            && !edge.reserved_source_reference_count() && shroud->hasAcceptedContent()
            && device.texture_bytes(edge.texture_handle(shroud->getShroudTexture()))==pixels,
            "original shroud binding fault changed accepted pixels/stage/refs");
    }
    require(rejected([&]{W3DShaderManager::setShroudTex(0);})
        && sameStage(original,DX8Wrapper::Inspect_Source_State()) && edge.source_revision()==revision,
        "original shroud wrong-stage rejection changed state");
    for(unsigned invalid=0;invalid<3;++invalid) {
        Matrix4x4 bad=view;
        if(invalid==0) for(unsigned column=0;column<4;++column) bad[2][column]=0;
        if(invalid==1) bad[1][2]=std::numeric_limits<float>::quiet_NaN();
        if(invalid==2) bad[3][1]=std::numeric_limits<float>::infinity();
        DX8Wrapper::Set_Transform(D3DTS_VIEW,bad);
        const auto state=DX8Wrapper::Inspect_Source_State();const auto serial=edge.source_revision();
        require(rejected([&]{W3DShaderManager::setShroudTex(1);})
            && sameStage(state,DX8Wrapper::Inspect_Source_State()) && edge.source_revision()==serial
            && device.resource_counts()==resources && !edge.queued_source_reference_count(),
            "original singular/nonfinite shroud binding mutated state");
    }
    DX8Wrapper::Set_Transform(D3DTS_VIEW,view);
    require(W3DShaderManager::setShroudTex(1) && DX8Wrapper::Peek_Texture(1)==shroud->getShroudTexture()
        && edge.queued_source_reference_count()==1 && edge.drain_source_references(edge.generation())
        && !edge.queued_source_reference_count(),"original exact shroud binding/drain retry failed");
    const auto state=DX8Wrapper::Inspect_Source_State();const auto& stage=state.stages[1];
    require(stage.at(D3DTSS_TEXCOORDINDEX)==D3DTSS_TCI_CAMERASPACEPOSITION
        && stage.at(D3DTSS_TEXTURETRANSFORMFLAGS)==D3DTTFF_COUNT2
        && stage.at(D3DTSS_COLORARG1)==D3DTA_TEXTURE && stage.at(D3DTSS_COLORARG2)==D3DTA_CURRENT
        && stage.at(D3DTSS_ALPHAARG1)==D3DTA_TEXTURE && stage.at(D3DTSS_ALPHAARG2)==D3DTA_CURRENT
        && stage.at(D3DTSS_COLOROP)==D3DTOP_MODULATE && stage.at(D3DTSS_ALPHAOP)==D3DTOP_SELECTARG2,
        "original exact shroud native stage sequence mismatch");
    Matrix4x4 expected(true);
    const float sx=1.0f/(shroud->getCellWidth()*shroud->getTextureWidth());
    const float sy=1.0f/(shroud->getCellHeight()*shroud->getTextureHeight());
    expected[0][0]=0;expected[0][1]=sx;expected[0][3]=(4+shroud->getCellWidth())*sx;
    expected[1][0]=-sy;expected[1][1]=0;expected[1][3]=(3+shroud->getCellHeight())*sy;
    expected[2][3]=-2;
    const auto& actual=state.transforms.at(D3DTS_TEXTURE0+1);
    for(unsigned row=0;row<4;++row) for(unsigned col=0;col<4;++col)
        require(std::fabs(expected[row][col]-actual[row][col])<1e-6f,
            "original shroud transpose-equivalent matrix mismatch");
    auto* visual=TheTerrainVisual;TheTerrainVisual=reinterpret_cast<TerrainVisual*>(1);
    require(rejected([&]{W3DShaderManager::setShroudTex(1);}),"original stale visual publication accepted");
    TheTerrainVisual=visual;
    auto* terrain=TheTerrainRenderObject;TheTerrainRenderObject=reinterpret_cast<BaseHeightMapRenderObjClass*>(1);
    require(rejected([&]{W3DShaderManager::setShroudTex(1);}),"original stale terrain publication accepted");
    TheTerrainRenderObject=terrain;
    auto* height=TheHeightMap;TheHeightMap=NULL;
    require(rejected([&]{W3DShaderManager::setShroudTex(1);}),"original missing height publication accepted");
    TheHeightMap=height;
    require(device.texture_bytes(edge.texture_handle(shroud->getShroudTexture()))==pixels,
        "original binding changed shroud pixels");
    shroud->setShroudLevel(0,0,170);
    const auto dirty=DX8Wrapper::Inspect_Source_State();const auto dirty_serial=edge.source_revision();
    require(rejected([&]{W3DShaderManager::setShroudTex(1);}) && sameStage(dirty,DX8Wrapper::Inspect_Source_State())
        && edge.source_revision()==dirty_serial,"original dirty shroud content bound");
    device.fail_next_texture_upload();
    CameraClass camera;
    require(rejected([&]{shroud->render(&camera);}),"original dirty bound upload fault accepted");
    require(sameStage(dirty,DX8Wrapper::Inspect_Source_State()) && edge.source_revision()==dirty_serial
        && device.texture_bytes(edge.texture_handle(shroud->getShroudTexture()))==pixels,
        "original bound shroud upload rejection changed accepted stage/pixels");
    shroud->render(&camera);
    require(W3DShaderManager::setShroudTex(1) && edge.drain_source_references(edge.generation()),
        "original combined content/binding retry failed");
    DX8Wrapper::Set_Texture(1,NULL);TextureBaseClass::Apply_Null(1);
}

void physicalProof(const char* map_path)
{
    // linux_main initialized shipping process services before GameMain/probe.
    // Destroy the device/worker here, before those services leave their scope.
    for(unsigned generation=0;generation<2;++generation) {
        TheWritableGlobalData->m_shroudColor.setFromInt(0x7db3e1);
        zh::renderer::BgfxOptions options;options.shader_root=ZH_BGFX_SHADER_DIR;
        zh::renderer::BgfxGpuDevice device(options);
        {
            zh::original_runtime::OriginalGpuEdge edge(device);
            auto display=std::make_unique<W3DDisplay>();display->init();
            auto* saved_visual=TheTerrainVisual;
            auto visual=std::make_unique<W3DTerrainVisual>();TheTerrainVisual=visual.get();visual->init();
            require(visual->load(AsciiString(map_path)),"physical shroud generated map load");
            auto* shroud=TheTerrainRenderObject->getShroud();
            shroud->reset();shroud->init(TheTerrainRenderObject->getMap(),MAP_XY_FACTOR*2,MAP_XY_FACTOR*3);
            shroud->setBorderShroudLevel(17);shroud->fillShroudData(39);
            shroud->setShroudLevel(0,0,255);shroud->setShroudLevel(1,0,123);
            CameraClass camera;
            const auto initial=device.live_resource_count();
            for(unsigned fault=0;fault<4;++fault) {
                if(fault==0) device.fail_next_transaction_checkpoint();
                if(fault==1) device.fail_transaction_operation_after(0);
                if(fault==2) device.fail_transaction_native_publication_after(0);
                if(fault==3) W3DShroudGeneratedProbeAccess::rejectNextCommit(*shroud);
                const auto frames=device.native_frame_advance_count();
                const auto destroys=device.native_retirement_destroy_count();
                require(rejected([&]{shroud->render(&camera);}) && !shroud->getShroudTexture()
                    && !shroud->acceptedContentEpoch() && device.live_resource_count()==initial
                    && device.native_frame_advance_count()==frames
                    && device.native_retirement_destroy_count()==destroys,
                    "physical first shroud fault published/advanced native owner");
                require(device.wait_idle(),"physical first shroud canceled drain");
            }
            shroud->render(&camera);
            auto* identity=shroud->getShroudTexture();const auto handle=edge.texture_handle(identity);
            const auto accepted_epoch=shroud->acceptedContentEpoch();
            DX8Wrapper::Set_Transform(D3DTS_VIEW,Matrix4x4(true));
            require(W3DShaderManager::setShroudTex(1) && edge.drain_source_references(edge.generation())
                && DX8Wrapper::Peek_Texture(1)==identity && edge.pending_stage(1).texture==handle,
                "physical exact resident shroud binding");
            const auto accepted=device.readback_rgba(edge.pending_stage(1).texture);
            const auto color=static_cast<unsigned>(TheGlobalData->m_shroudColor.getAsInt());
            require(accepted.size()==static_cast<std::size_t>(shroud->getTextureWidth())*shroud->getTextureHeight()*4,
                "physical shroud pixel extent");
            for(Int y=0;y<shroud->getTextureHeight();++y) for(Int x=0;x<shroud->getTextureWidth();++x) {
                const auto level=x && y && x<=shroud->getNumShroudCellsX() && y<=shroud->getNumShroudCellsY()
                    ? shroud->getShroudLevel(x-1,y-1) : 17;
                const auto expected=expectedPixel(static_cast<W3DShroudLevel>(level),color,FALSE);
                const auto offset=(y*shroud->getTextureWidth()+x)*4;
                require(accepted[offset]==expected[2] && accepted[offset+1]==expected[1]
                    && accepted[offset+2]==expected[0] && accepted[offset+3]==expected[3],
                    "physical nonuniform quantized shroud/border bytes");
            }
            require(accepted[0]!=accepted[(shroud->getTextureWidth()+1)*4]
                && accepted[(shroud->getTextureWidth()+1)*4]!=accepted[(shroud->getTextureWidth()+2)*4],
                "physical shroud lacked distinct border/interior levels");
            shroud->setShroudLevel(1,0,170);
            for(unsigned fault=0;fault<4;++fault) {
                if(fault==0) device.fail_next_transaction_checkpoint();
                if(fault==1) device.fail_transaction_operation_after(0);
                if(fault==2) device.fail_transaction_native_publication_after(0);
                if(fault==3) W3DShroudGeneratedProbeAccess::rejectNextCommit(*shroud);
                const auto frames=device.native_frame_advance_count();
                const auto destroys=device.native_retirement_destroy_count();
                const auto revision=edge.source_revision();const auto stage=DX8Wrapper::Inspect_Source_State();
                require(rejected([&]{shroud->render(&camera);}) && identity==shroud->getShroudTexture()
                    && edge.texture_handle(identity)==handle && shroud->acceptedContentEpoch()==accepted_epoch
                    && !shroud->hasAcceptedContent() && edge.source_revision()==revision
                    && sameStage(stage,DX8Wrapper::Inspect_Source_State())
                    && device.native_frame_advance_count()==frames
                    && device.native_retirement_destroy_count()==destroys,
                    "physical resident shroud upload rollback identity/epoch/stage");
                require(device.readback_rgba(handle)==accepted,"physical resident shroud fault changed accepted pixels");
            }
            shroud->render(&camera);
            require(shroud->hasAcceptedContent() && edge.texture_handle(identity)==handle
                && device.readback_rgba(handle)!=accepted,"physical shroud COW retry failed");
            const auto updated=device.readback_rgba(handle);
            edge.fail_next_source_stage_commit();
            const auto state=DX8Wrapper::Inspect_Source_State();const auto revision=edge.source_revision();
            require(rejected([&]{W3DShaderManager::setShroudTex(1);})
                && sameStage(state,DX8Wrapper::Inspect_Source_State()) && edge.source_revision()==revision
                && device.readback_rgba(handle)==updated,"physical shroud binding commit rollback");
            require(W3DShaderManager::setShroudTex(1) && edge.drain_source_references(edge.generation()),
                "physical shroud binding retry");
            DX8Wrapper::Set_Texture(1,NULL);TextureBaseClass::Apply_Null(1);
            visual->reset();visual.reset();TheTerrainVisual=saved_visual;
            edge.release_source_buffers();display.reset();
        }
        require(device.wait_idle() && !device.live_resource_count(),"physical shroud teardown retained owner");
    }
    std::puts("original terrain shroud physical: nonuniform=1 rollback=1 generations=2 resources=0 draws=0");
}
}

extern "C" void zh_probe_terrain_shroud_projection()
{
	const char *map_path = std::getenv("ZH_M22_TERRAIN_SHROUD_PROJECTION_MAP");
	require(map_path, "original terrain shroud projection map missing");
	const Real saved_partition = TheWritableGlobalData->m_partitionCellSize;
    const auto saved_color=TheWritableGlobalData->m_shroudColor;
    const auto saved_alpha=TheWritableGlobalData->m_shroudAlpha;
#if defined(_DEBUG) || defined(_INTERNAL)
    const Bool saved_fog=TheWritableGlobalData->m_fogOfWarOn;
#endif
	TheWritableGlobalData->m_partitionCellSize = MAP_XY_FACTOR;
    TheWritableGlobalData->m_shroudColor.setFromInt(0x7db3e1);
    TheWritableGlobalData->m_shroudAlpha=7;
#if defined(_DEBUG) || defined(_INTERNAL)
    TheWritableGlobalData->m_fogOfWarOn=FALSE;
#endif
    std::array<unsigned char,4> debugPixel{};
    W3DShroud::encodeContentPixel(17,TRUE,debugPixel.data());
    require(debugPixel==std::array<unsigned char,4>{0,0,0,238},
        "original normalized debug RGB direct-cast/nibble-alpha mismatch");
	zh::renderer::RecordingGpuDevice device;
	for (Int generation = 0; generation != 2; ++generation) {
        TheWritableGlobalData->m_shroudColor.setFromInt(0x7db3e1);
		zh::original_runtime::OriginalGpuEdge edge(device);
		auto display = std::make_unique<W3DDisplay>();
		display->init();
		TerrainVisual *saved_visual = TheTerrainVisual;
		auto visual = std::make_unique<W3DTerrainVisual>();
		TheTerrainVisual = visual.get();
		visual->init();
		require(visual->load(AsciiString(map_path)) && TheTerrainRenderObject &&
			TheTerrainRenderObject->getShroud(),
			"original terrain shroud projection map load failed");
		W3DShroud *shroud = TheTerrainRenderObject->getShroud();
        shroud->reset();shroud->init(TheTerrainRenderObject->getMap(),MAP_XY_FACTOR*2,MAP_XY_FACTOR*3);
		CameraClass camera;
        contentBounds(device,edge,TheTerrainRenderObject->getMap());
		const zh::renderer::ResourceCounts baseline = device.resource_counts();
		require(!shroud->getShroudTexture(),
			"original terrain shroud projection existed before render");
        shroud->setBorderShroudLevel(17);
        shroud->fillShroudData(39);
        shroud->setShroudLevel(0,0,255);
        shroud->setShroudLevel(1,0,123);
        const auto first_epoch=shroud->contentEpoch();
        const auto revision=edge.source_revision();
        const auto stage0=DX8Wrapper::Peek_Texture(0);
        const auto stage1=DX8Wrapper::Peek_Texture(1);

		require(rejected([&]() { shroud->render(NULL); }) && !shroud->getShroudTexture(),
			"original terrain shroud null camera published a texture");
		device.fail_next_texture_create();
		require(rejected([&]() { shroud->render(&camera); }) && !shroud->getShroudTexture() &&
			device.resource_counts() == baseline,
			"original terrain shroud allocation failure retained projection resource");
        device.fail_next_texture_upload();
        require(rejected([&]{shroud->render(&camera);}) && !shroud->getShroudTexture()
            && device.resource_counts()==baseline && shroud->contentEpoch()==first_epoch
            && !shroud->acceptedContentEpoch() && edge.source_revision()==revision
            && DX8Wrapper::Peek_Texture(0)==stage0 && DX8Wrapper::Peek_Texture(1)==stage1,
            "original first shroud upload fault changed accepted state");
        device.fail_next_transaction_checkpoint();
        require(rejected([&]{shroud->render(&camera);}) && !shroud->getShroudTexture()
            && device.resource_counts()==baseline && !shroud->hasAcceptedContent(),
            "original first shroud checkpoint failure published content");
        W3DShroudGeneratedProbeAccess::rejectNextCommit(*shroud);
        require(rejected([&]{shroud->render(&camera);}) && !shroud->getShroudTexture()
            && device.resource_counts()==baseline && edge.source_revision()==revision
            && !shroud->acceptedContentEpoch() && !shroud->hasAcceptedContent(),
            "original first shroud commit rejection retained candidate publication");
		W3DShroudMaterialPassClass material;
		require(rejected([&]() { material.Install_Materials(); }),
			"original terrain shroud material accepted no projection texture");

		shroud->render(&camera);
		TextureClass *first_texture = shroud->getShroudTexture();
		require(first_texture && device.resource_counts().textures == baseline.textures + 1,
			"original terrain shroud did not publish one projection texture");
        require(shroud->hasAcceptedContent() && shroud->acceptedContentEpoch()==first_epoch,
            "original shroud content epoch not accepted");
        exactPixels(device,edge,shroud,17,FALSE);
        const auto operations=device.operation_counts();
        shroud->render(&camera);
        const auto after=device.operation_counts();
        require(after.commands==operations.commands && after.creates==operations.creates
            && after.uploads==operations.uploads && after.failures==operations.failures
            && edge.source_revision()==revision,
            "original clean shroud render consumed device/source state");
        const auto accepted_handle=edge.texture_handle(first_texture);
        const auto accepted_pixels=device.texture_bytes(accepted_handle);
        shroud->setShroudLevel(1,0,155);
        require(!shroud->hasAcceptedContent() && shroud->acceptedContentEpoch()==first_epoch,
            "original dirty shroud kept current readiness");
        const auto dirty_epoch=shroud->contentEpoch();
        for(unsigned fault=0;fault<4;++fault) {
            if(fault==0) device.fail_next_texture_upload();
            if(fault==1) device.fail_next_transaction_checkpoint();
            if(fault==2) device.fail_transaction_operation_after(0);
            if(fault==3) W3DShroudGeneratedProbeAccess::rejectNextCommit(*shroud);
            require(rejected([&]{shroud->render(&camera);}) && shroud->getShroudTexture()==first_texture
                && edge.texture_handle(first_texture)==accepted_handle
                && device.texture_bytes(accepted_handle)==accepted_pixels
                && shroud->contentEpoch()==dirty_epoch && shroud->acceptedContentEpoch()==first_epoch
                && !shroud->hasAcceptedContent() && edge.source_revision()==revision
                && DX8Wrapper::Peek_Texture(0)==stage0 && DX8Wrapper::Peek_Texture(1)==stage1,
                "original resident shroud fault changed pixels/stage/epoch");
        }
        shroud->render(&camera);
        require(shroud->getShroudTexture()==first_texture && edge.texture_handle(first_texture)==accepted_handle
            && shroud->hasAcceptedContent() && shroud->acceptedContentEpoch()==dirty_epoch,
            "original shroud retry changed resident identity");
        exactPixels(device,edge,shroud,17,FALSE);
        TheWritableGlobalData->m_shroudColor.setFromInt(0xe19b53);
        require(!shroud->hasAcceptedContent(),"original changed shroud color remained ready");
        shroud->render(&camera);exactPixels(device,edge,shroud,17,FALSE);
#if defined(_DEBUG) || defined(_INTERNAL)
        TheWritableGlobalData->m_fogOfWarOn=TRUE;
        require(!shroud->hasAcceptedContent(),"original debug shroud profile remained ready");
        shroud->render(&camera);exactPixels(device,edge,shroud,17,TRUE);
        const auto debug_pixels=device.texture_bytes(accepted_handle);
        require(debug_pixels[0]==0 && debug_pixels[1]==0 && debug_pixels[2]==0 && debug_pixels[3]>0,
            "original normalized debug RGB direct cast/alpha semantics changed");
        TheWritableGlobalData->m_fogOfWarOn=FALSE;
        shroud->render(&camera);exactPixels(device,edge,shroud,17,FALSE);
#endif
        bindingControls(device,edge,shroud);
		shroud->setShroudFilter(FALSE);
		require(rejected([&]() { shroud->render(&camera); }) &&
			shroud->getShroudTexture() == first_texture,
			"original terrain shroud filter rejection changed projection owner");
		shroud->setShroudFilter(TRUE);
		material.Install_Materials();
		material.UnInstall_Materials();

		shroud->ReleaseResources();
		require(!shroud->getShroudTexture() && device.resource_counts() == baseline,
			"original terrain shroud release retained projection resource");
        require(rejected([&]{W3DShaderManager::setShroudTex(1);}) && !edge.queued_source_reference_count(),
            "original released shroud bound a stale texture");
		require(rejected([&]() { material.Install_Materials(); }),
			"original terrain shroud material accepted released resource");
		require(shroud->ReAcquireResources() && shroud->getShroudTexture(),
			"original terrain shroud re-acquire failed");
        require(!shroud->hasAcceptedContent() && !shroud->acceptedContentEpoch(),
            "original reacquired empty shroud claimed current content");
        require(rejected([&]{W3DShaderManager::setShroudTex(1);}),
            "original reacquired empty shroud bound stale content");
		shroud->render(&camera);
		material.Install_Materials();
		material.UnInstall_Materials();

		visual->reset();
		visual.reset();
		require(!TheTerrainVisual && !TheHeightMap && !TheTerrainRenderObject,
			"original terrain shroud visual teardown retained owner");
		TheTerrainVisual = saved_visual;
		edge.release_source_buffers();
		require(device.resource_counts().total() == 0,
			"original terrain shroud teardown retained Recording resource");
		display.reset();
	}
	require(device.resource_counts().total() == 0,
		"original terrain shroud projection re-entry retained Recording resource");
    if(std::getenv("ZH_M22_TERRAIN_SHROUD_PHYSICAL")) physicalProof(map_path);
	TheWritableGlobalData->m_partitionCellSize = saved_partition;
    TheWritableGlobalData->m_shroudColor=saved_color;
    TheWritableGlobalData->m_shroudAlpha=saved_alpha;
#if defined(_DEBUG) || defined(_INTERNAL)
    TheWritableGlobalData->m_fogOfWarOn=saved_fog;
#endif
	std::puts("original terrain shroud projection: material=1 rollback=2 generations=2 resources=0 draws=0");
}
