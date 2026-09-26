#include "original_gpu_edge.h"
#include "dx8wrapper.h"
#include "texture.h"
#include "ww3d.h"
#include "zh/renderer/recording_device.h"
#include "zh/platform/bgfx_device.h"
#include "zh/original_process.h"

#include <array>
#include <cstring>
#include <iostream>
#include <stdexcept>
#include <string>

// This minimal fixture has no volume owner. Direct-entry rejection must precede
// that optional provider; the sentinel proves it is never queried.
static unsigned volume_provider_queries=0;
bool zh_w3d_volume_provider_owns(const VertexBufferClass*,const IndexBufferClass*)
{ ++volume_provider_queries;return false; }

namespace {
using namespace zh::renderer;
using Edge=zh::original_runtime::OriginalGpuEdge;
struct ShaderProbe:ShaderClass {
    static bool dirty() { return ShaderDirty; }
    static unsigned long current() { return CurrentShader; }
};
void check(bool value,const char* category) { if (!value) throw std::runtime_error(category); }
template<class F> void rejects(F action,const char* category)
{ bool failed=false;try { action(); } catch (const std::exception&) { failed=true; } check(failed,category); }
struct Texture final:TextureClass {
    unsigned& destroyed;
    unsigned initializations=0;
    explicit Texture(unsigned& counter):TextureClass("generated-stage",nullptr,MIP_LEVELS_1),destroyed(counter) {}
    ~Texture() override { ++destroyed; }
    void Init() override { ++initializations;TextureClass::Init(); }
    unsigned accessed() const { return LastAccessed; }
    void access(unsigned value) { LastAccessed=value; }
};
Texture* source(Edge& edge,unsigned& destroyed,bool ready=true)
{
    auto* texture=NEW_REF(Texture,(destroyed));unsigned mips=1;
    auto handle=edge.create_texture(WW3D_FORMAT_A8R8G8B8,4,4,mips);
    std::array<unsigned,16> pixels{};pixels.fill(0xff00ff00);
    edge.upload_texture(handle,0,4,4,16,pixels.data(),sizeof(pixels));edge.publish_texture(texture,handle);
    if (ready) texture->Apply_Gpu_Texture(WW3D_FORMAT_A8R8G8B8,4,4);
    texture->access(12345);return texture;
}
bool matrices(const std::map<int,Matrix4x4>& left,const std::map<int,Matrix4x4>& right)
{
    if (left.size()!=right.size()) return false;
    auto a=left.begin(),b=right.begin();
    for (;a!=left.end();++a,++b) if (a->first!=b->first || std::memcmp(&a->second,&b->second,sizeof(Matrix4x4))) return false;
    return true;
}
bool same(const DX8Wrapper::SourceStateSnapshot& a,const DX8Wrapper::SourceStateSnapshot& b)
{
    return !std::memcmp(&a.material,&b.material,sizeof(a.material)) && a.render==b.render && a.stages==b.stages
        && matrices(a.transforms,b.transforms) && a.fog_enabled==b.fog_enabled && a.fog_color==b.fog_color
        && a.material_applied==b.material_applied && a.light_enabled==b.light_enabled
        && !std::memcmp(a.lights.data(),b.lights.data(),sizeof(a.lights))
        && a.light_environment_selected==b.light_environment_selected;
}
struct Baseline {
    DX8Wrapper::SourceStateSnapshot state=DX8Wrapper::Inspect_Source_State();
    unsigned dirty=DX8Wrapper::Pending_Changes();
    bool shader_dirty=ShaderProbe::dirty();
    unsigned long shader=ShaderProbe::current();
    const VertexMaterialClass* material=DX8Wrapper::Peek_Material();
    std::array<TextureBaseClass*,8> textures{};
    std::uint64_t revision;
    explicit Baseline(Edge& edge):revision(edge.source_revision())
    { for (unsigned stage=0;stage<8;++stage) textures[stage]=DX8Wrapper::Peek_Texture(stage); }
    bool equals(Edge& edge) const {
        if (!same(state,DX8Wrapper::Inspect_Source_State()) || dirty!=DX8Wrapper::Pending_Changes()
            || shader_dirty!=ShaderProbe::dirty() || shader!=ShaderProbe::current()
            || material!=DX8Wrapper::Peek_Material() || revision!=edge.source_revision()) return false;
        for (unsigned stage=0;stage<8;++stage) if (textures[stage]!=DX8Wrapper::Peek_Texture(stage)) return false;
        return true;
    }
};
Edge::SourceStageDesc desc(Edge& edge,const Edge::SourceStageSelection* selections,unsigned count=1)
{ Edge::SourceStageDesc result;result.generation=edge.generation();result.selections=selections;result.count=count;return result; }
void stage(Edge& edge,const Edge::SourceStageToken& token,TextureBaseClass* texture)
{
    DX8Wrapper::Set_Texture(0,texture);
    DX8Wrapper::Set_DX8_Texture_Stage_State(0,D3DTSS_TEXCOORDINDEX,0);
    DX8Wrapper::Set_DX8_Texture_Stage_State(0,D3DTSS_TEXTURETRANSFORMFLAGS,D3DTTFF_COUNT2);
    auto matrix=Matrix4x4(true);matrix[2][0]=0.25F;
    DX8Wrapper::Set_Transform(D3DTS_TEXTURE0,matrix);
    edge.apply_source_stages(token);
}
void recording()
{
    RecordingGpuDevice device;unsigned destroyed=0;
    {
        Edge edge(device);auto* prior=source(edge,destroyed);auto* candidate=source(edge,destroyed);
        auto* unrelated=source(edge,destroyed);auto* unready=source(edge,destroyed,false);
        TextureDesc target;target.width=target.height=4;target.render_target=true;target.format=TextureFormat::bgra8;
        const auto color=device.create_texture(target,"generated stage phase color");target.format=TextureFormat::depth24_stencil8;
        const auto depth=device.create_texture(target,"generated stage phase depth");
        check(color && depth,"stage phase attachments");edge.bind_frame_targets(color,depth,4,4);
        DX8Wrapper::Set_Texture(0,prior);prior->Apply(0); // Deliberately retains its dirty bit.
        prior->access(12345);
        DX8Wrapper::Set_DX8_Texture_Stage_State(0,D3DTSS_COLOROP,D3DTOP_SELECTARG1);
        auto initial=Matrix4x4(true);initial[2][0]=0.125F;DX8Wrapper::Set_Transform(D3DTS_TEXTURE0,initial);
        DX8Wrapper::Set_Texture(3,unrelated);
        DX8Wrapper::Set_DX8_Texture_Stage_State(3,D3DTSS_TEXCOORDINDEX,3);
        auto untouched=Matrix4x4(true);untouched[2][1]=0.75F;
        DX8Wrapper::Set_Transform(D3DTS_TEXTURE0+3,untouched);
        auto* material=NEW_REF(VertexMaterialClass,());
        DX8Wrapper::Set_Material(material);material->Release_Ref();DX8Wrapper::Set_Shader(ShaderClass::_PresetOpaqueShader);
        const Edge::SourceStageSelection selection{0,candidate};
        const auto good=desc(edge,&selection);Edge::SourceStageToken token{11,12,13,14};
        edge.begin_source_frame(true,true,0,0,0,1);
        const auto active_frame=device.snapshot();
        check(!edge.begin_source_stages(good,token) && token.owner==11 && token.sequence==12
            && prior->Num_Refs()==2 && candidate->Num_Refs()==1 && device.snapshot()==active_frame
            && !edge.reserved_source_reference_count(),"active-frame admission changed original owner");
        edge.end_source_frame(false);
        const Baseline baseline(edge);const auto native=device.snapshot();const auto sampler=edge.pending_stage(0).sampler;
        const auto refs=prior->Num_Refs();const auto candidate_refs=candidate->Num_Refs();
        for (unsigned bad=0;bad<15;++bad) {
            auto rejected=good;std::array<Edge::SourceStageSelection,9> selections{};
            selections.fill(selection);
            if (bad==0) ++rejected.generation;
            if (bad==1) rejected.selections=nullptr;
            if (bad==2) rejected.count=0;
            if (bad==3) {rejected.selections=selections.data();rejected.count=9;}
            if (bad==4) {selections[0].stage=8;rejected.selections=selections.data();}
            if (bad==5) {rejected.selections=selections.data();rejected.count=2;}
            if (bad==6) rejected.stage_keys=13;
            if (bad==7) rejected.transform_nodes=9;
            if (bad==8) rejected.transform_nodes=0;
            if (bad==9) rejected.commands=4097;
            if (bad==10) rejected.resources=4097;
            if (bad==11) rejected.bytes=64U*1024U*1024U+1;
            if (bad==12) {selections[0].texture=reinterpret_cast<TextureBaseClass*>(1);rejected.selections=selections.data();}
            if (bad==13) {selections[0].texture=unready;rejected.selections=selections.data();}
            if (bad==14) rejected.stage_keys=0;
            check(!edge.begin_source_stages(rejected,token) && token.owner==11 && token.sequence==12
                && token.generation==13 && token.mask==14 && baseline.equals(edge) && device.snapshot()==native
                && prior->Num_Refs()==refs && candidate->Num_Refs()==candidate_refs && !unready->initializations
                && !edge.reserved_source_reference_count() && !edge.queued_source_reference_count(),
                "preflight rejection changed source, device, refs or token");
        }
        candidate->Get_Filter().Set_U_Addr_Mode(static_cast<TextureFilterClass::TxtAddrMode>(9));
        check(!edge.begin_source_stages(good,token) && baseline.equals(edge) && device.snapshot()==native
            && candidate->accessed()==12345,"invalid complete filter tuple had effects");
        candidate->Get_Filter().Set_U_Addr_Mode(TextureFilterClass::TEXTURE_ADDRESS_REPEAT);
        for (unsigned allocation=0;allocation<4;++allocation) {
            edge.fail_source_stage_allocation_after(allocation);
            check(!edge.begin_source_stages(good,token) && baseline.equals(edge) && device.snapshot()==native
                && prior->Num_Refs()==refs && candidate->Num_Refs()==candidate_refs
                && !edge.queued_source_reference_count() && !edge.reserved_source_reference_count(),
                "checkpoint allocation failure changed admission baseline");
            check(edge.begin_source_stages(good,token),"checkpoint allocation retry");
            check(edge.abort_source_stages(token) && edge.drain_source_references(edge.generation())
                && baseline.equals(edge) && device.snapshot()==native,"checkpoint retry changed identity");
        }
        check(edge.begin_source_stages(good,token),"candidate map allocation admission");
        edge.fail_source_stage_allocation_after(0);
        rejects([&]{stage(edge,token,candidate);},"candidate map allocation fault missed");
        check(!edge.commit_source_stages(token) && edge.abort_source_stages(token)
            && edge.drain_source_references(edge.generation()) && baseline.equals(edge) && device.snapshot()==native,
            "candidate map allocation rollback changed baseline");
        const Edge::SourceStageSelection empty_stage{1,nullptr};
        check(edge.begin_source_stages(desc(edge,&empty_stage),token),"candidate transform allocation admission");
        edge.fail_source_stage_allocation_after(0);
        rejects([&]{DX8Wrapper::Set_Transform(D3DTS_TEXTURE0+1,Matrix4x4(true));},"candidate transform allocation fault missed");
        check(!edge.commit_source_stages(token) && edge.abort_source_stages(token)
            && baseline.equals(edge) && device.snapshot()==native,"candidate transform allocation rollback changed baseline");
        for (unsigned fault=0;fault<3;++fault) {
            auto admission=good;
            if (fault==0) device.fail_next_transaction_checkpoint();
            if (fault==1) admission.resources=1;
            if (fault==2) admission.bytes=1;
            check(!edge.begin_source_stages(admission,token) && baseline.equals(edge) && device.snapshot()==native
                && prior->Num_Refs()==refs && candidate->Num_Refs()==candidate_refs
                && !edge.queued_source_reference_count() && !edge.reserved_source_reference_count(),
                "device admission cancellation left source pins or changed baseline");
            check(edge.begin_source_stages(good,token),"admission retry failed");
            check(edge.abort_source_stages(token) && edge.drain_source_references(edge.generation())
                && baseline.equals(edge) && device.snapshot()==native,"admission retry identity changed");
        }
        TextureFilterClass invalid_filter(MIP_LEVELS_1);
        invalid_filter.Set_Min_Filter(static_cast<TextureFilterClass::FilterType>(TextureFilterClass::FILTER_TYPE_COUNT));
        for (unsigned fault=0;fault<48;++fault) {
            check(edge.begin_source_stages(good,token),"fault stage admission");
            auto foreign=token;++foreign.sequence;
            check(!edge.abort_source_stages(foreign) && !edge.commit_source_stages(foreign),"foreign token consumed stage owner");
            auto untouched_token=token;check(!edge.begin_source_stages(good,untouched_token)
                && untouched_token.sequence==token.sequence,"nested admission poisoned stage owner");
            if (fault<15) device.fail_transaction_operation_after(fault);
            if (fault==15) device.fail_next_sampler_create();
            if (fault==36) {device.fail_transaction_operation_after(0);device.fail_next_transaction_diagnostic_allocation();}
            // Out-of-scope rejection must poison an otherwise commit-ready
            // owner, not merely benefit from its initial unapplied state.
            if (fault>=16 && fault!=36) stage(edge,token,candidate);
            bool failed=false;
            try {
                if (fault<16) stage(edge,token,candidate);
                if (fault<15) for (unsigned publication=0;publication<3;++publication)
                    edge.record_source_state("generated late stage publication");
                if (fault==16) DX8Wrapper::Set_Texture(3,candidate);
                if (fault==17) DX8Wrapper::Set_Material(nullptr);
                if (fault==18) DX8Wrapper::Set_Shader(ShaderClass::_PresetOpaqueShader);
                if (fault==19) candidate->Get_Filter().Set_V_Addr_Mode(TextureFilterClass::TEXTURE_ADDRESS_CLAMP);
                if (fault==20) DX8Wrapper::Set_Transform(D3DTS_VIEW,Matrix4x4(true));
                if (fault==21) ShaderClass::Invalidate();
                if (fault==22) ShaderClass::Invert_Backface_Culling(true);
                if (fault==23) WW3D::Sync(999);
                if (fault==24) candidate->Get_Filter()=prior->Get_Filter();
                if (fault==25) candidate->Apply_Gpu_Texture(WW3D_FORMAT_A8R8G8B8,8,8);
                if (fault==26) candidate->TextureClass::Init();
                if (fault==27) DX8Wrapper::Set_World_Identity();
                if (fault==28) WW3D::Enable_Texturing(false);
                if (fault==29) WW3D::Set_Texture_Filter(TextureFilterClass::TEXTURE_FILTER_BILINEAR);
                if (fault==30) DX8Wrapper::Apply_Render_State_Changes();
                if (fault==31) {unsigned mips=1;edge.create_texture(WW3D_FORMAT_A8R8G8B8,4,4,mips);}
                if (fault==32) DX8Wrapper::Set_DX8_Texture_Stage_State(0,999,0);
                if (fault==33) DX8Wrapper::Set_DX8_Texture_Stage_State(0,D3DTSS_COLOROP,999);
                if (fault==34) DX8Wrapper::Set_Texture(0,prior);
                if (fault==35) candidate->Get_Filter().Apply(3);
                if (fault==36) stage(edge,token,candidate);
                if (fault==37) {
                    bool exact=false;try {invalid_filter.Apply(0);} catch (const std::runtime_error& error) {
                        exact=std::string(error.what())=="original texture filter or stage is invalid";
                    }
                    check(exact && !edge.commit_source_stages(token),"transaction invalid tuple precedence/poison changed");
                    throw std::runtime_error("generated admitted invalid-tuple rejection");
                }
                if (fault==38) edge.begin_source_frame(true,true,1,0,0,1);
                if (fault==39) edge.end_source_frame(false);
                if (fault==40) edge.set_source_viewport(0,0,4,4,0,1);
                if (fault==41) edge.clear_source_viewport(true,true,false,{0,0,0,1},1,0);
                if (fault==42) edge.draw_source_indexed(reinterpret_cast<const VertexBufferClass*>(1),
                    reinterpret_cast<const IndexBufferClass*>(1),0,3,0,0,3,zh::renderer::PrimitiveTopology::triangle_list);
                if (fault==43) edge.draw_volume_stencil(reinterpret_cast<const VertexBufferClass*>(1),
                    reinterpret_cast<const IndexBufferClass*>(1),0,3,0,3,1);
                if (fault==44) edge.prepare_tree_state(reinterpret_cast<const VertexBufferClass*>(1),zh::original_runtime::TreeVertexUniform{});
                if (fault==45) {edge.abort_source_frame();throw std::runtime_error("generated frame-abort rejection");}
                if (fault==46) {edge.release_volume_stencil();throw std::runtime_error("generated resource-retirement rejection");}
                if (fault==47) edge.texture_creation_unavailable(WW3D_FORMAT_A8R8G8B8,4,4,1,0);
            } catch (const std::exception&) { failed=true; }
            check(failed && !edge.commit_source_stages(token) && WW3D::Get_Sync_Time()==0 && !volume_provider_queries,
                "stage fault did not poison commit or changed time");
            check(edge.abort_source_stages(token) && !edge.abort_source_stages(token)
                && edge.drain_source_references(edge.generation()) && baseline.equals(edge)
                && device.snapshot()==native && edge.pending_stage(0).sampler==sampler
                && prior->Num_Refs()==refs && candidate->Num_Refs()==candidate_refs
                && prior->accessed()==12345 && candidate->accessed()==12345
                && !edge.queued_source_reference_count(),"stage failure rollback changed identity or access");
            check(edge.begin_source_stages(good,token),"stage retry admission");stage(edge,token,candidate);
            check(edge.abort_source_stages(token) && edge.drain_source_references(edge.generation())
                && baseline.equals(edge) && device.snapshot()==native,"stage retry rollback changed identity");
        }
        check(edge.begin_source_stages(good,token),"successful stage admission");stage(edge,token,candidate);
        edge.fail_next_source_stage_commit();
        check(!edge.commit_source_stages(token) && edge.abort_source_stages(token)
            && edge.drain_source_references(edge.generation()) && baseline.equals(edge) && device.snapshot()==native,
            "late source commit failure changed baseline");
        check(edge.begin_source_stages(good,token),"source commit retry admission");stage(edge,token,candidate);
        check(edge.commit_source_stages(token) && !edge.commit_source_stages(token)
            && !edge.abort_source_stages(token) && edge.drain_source_references(edge.generation()),"successful commit/retirement");
        const auto accepted=DX8Wrapper::Inspect_Source_State();
        check(DX8Wrapper::Peek_Texture(0)==candidate && prior->Num_Refs()==1 && candidate->Num_Refs()==2
            && candidate->accessed()==WW3D::Get_Sync_Time() && !candidate->initializations
            && !(DX8Wrapper::Pending_Changes()&1) && (DX8Wrapper::Pending_Changes()&~1U)==(baseline.dirty&~1U)
            && ShaderProbe::dirty()==baseline.shader_dirty && ShaderProbe::current()==baseline.shader
            && DX8Wrapper::Peek_Material()==baseline.material && accepted.render==baseline.state.render
            && accepted.stages[3]==baseline.state.stages[3]
            && !std::memcmp(&accepted.transforms.at(D3DTS_TEXTURE0+3),&untouched,sizeof(untouched))
            && DX8Wrapper::Peek_Texture(3)==unrelated && edge.pending_stage(0).sampler!=sampler
            && !edge.queued_source_reference_count() && !edge.reserved_source_reference_count(),
            "selected commit changed unrelated pending state or left pins");
        // The former stage can be sole-owned by its source stage, then by A's pin.
        candidate->Release_Ref();const Edge::SourceStageSelection null_selection{0,nullptr};
        check(edge.begin_source_stages(desc(edge,&null_selection),token),"sole-owner null admission");
        stage(edge,token,nullptr);
        check(edge.commit_source_stages(token) && edge.queued_source_reference_count()==1 && destroyed==0,
            "sole stage commit ran terminal callback");
        device.fail_next_transaction_checkpoint();
        check(!edge.drain_source_references(edge.generation()) && destroyed==0 && edge.queued_source_reference_count()==1,
            "terminal retirement failure consumed source");
        check(edge.drain_source_references(edge.generation()) && destroyed==1 && !edge.queued_source_reference_count(),
            "terminal retirement retry did not retire once");
        check(edge.begin_source_stages(good,token)==false,"removed provider was admitted");
        std::array<Edge::SourceStageSelection,8> maximum{};
        const std::array<std::pair<unsigned,unsigned>,12> keys{{
            {D3DTSS_TEXCOORDINDEX,0},{D3DTSS_TEXTURETRANSFORMFLAGS,D3DTTFF_COUNT2},
            {D3DTSS_BUMPENVMAT00,0},{D3DTSS_BUMPENVMAT01,0},{D3DTSS_BUMPENVMAT10,0},{D3DTSS_BUMPENVMAT11,0},
            {D3DTSS_COLOROP,D3DTOP_MODULATE},{D3DTSS_ALPHAOP,D3DTOP_MODULATE},
            {D3DTSS_COLORARG1,D3DTA_TEXTURE},{D3DTSS_COLORARG2,D3DTA_CURRENT},
            {D3DTSS_ALPHAARG1,D3DTA_TEXTURE},{D3DTSS_ALPHAARG2,D3DTA_CURRENT}}};
        for (unsigned selected=0;selected<8;++selected) {
            maximum[selected]={selected,nullptr};
            for (const auto key:keys) DX8Wrapper::Set_DX8_Texture_Stage_State(selected,key.first,key.second);
            DX8Wrapper::Set_Transform(D3DTS_TEXTURE0+selected,Matrix4x4(true));
        }
        const Baseline maximum_baseline(edge);const auto maximum_native=device.snapshot();
        auto maximum_desc=desc(edge,maximum.data(),8);auto insufficient=maximum_desc;insufficient.stage_keys=11;
        check(!edge.begin_source_stages(insufficient,token) && maximum_baseline.equals(edge)
            && device.snapshot()==maximum_native && !edge.reserved_source_reference_count(),"key capacity preflight mutated baseline");
        insufficient=maximum_desc;insufficient.transform_nodes=7;
        check(!edge.begin_source_stages(insufficient,token) && maximum_baseline.equals(edge)
            && device.snapshot()==maximum_native && !edge.reserved_source_reference_count(),"transform capacity preflight mutated baseline");
        check(edge.begin_source_stages(maximum_desc,token),"exact stage/key/transform maximum admission");
        for (unsigned selected=0;selected<8;++selected) {
            DX8Wrapper::Set_Texture(selected,nullptr);
            DX8Wrapper::Set_DX8_Texture_Stage_State(selected,D3DTSS_TEXCOORDINDEX,selected);
            DX8Wrapper::Set_Transform(D3DTS_TEXTURE0+selected,Matrix4x4(true));
        }
        edge.apply_source_stages(token);
        check(edge.abort_source_stages(token) && edge.drain_source_references(edge.generation())
            && maximum_baseline.equals(edge) && device.snapshot()==maximum_native,
            "exact maximum abort lost map keys or source identities");
        prior->Release_Ref();unready->Release_Ref();unrelated->Release_Ref();
        DX8Wrapper::Reset_Source_State();
        device.destroy(color);device.destroy(depth);
    }
    check(destroyed==4 && device.resource_counts().total()==0,"stage recording residual");
    {
        Edge edge(device);auto* prior=source(edge,destroyed);auto* candidate=source(edge,destroyed);
        DX8Wrapper::Set_Texture(0,prior);prior->Release_Ref();
        const Edge::SourceStageSelection selection{0,candidate};Edge::SourceStageToken token;
        check(edge.begin_source_stages(desc(edge,&selection),token),"reset stage admission");stage(edge,token,candidate);
        DX8Wrapper::Reset_Source_State();
        check(!edge.source_stages_active() && !edge.queued_source_reference_count()
            && !edge.reserved_source_reference_count() && !edge.abort_source_stages(token)
            && !DX8Wrapper::Peek_Texture(0) && destroyed==5 && candidate->Num_Refs()==1
            && candidate->accessed()==12345,"reset did not cancel exact stage owner before mutation");
        candidate->Release_Ref();
    }
    check(destroyed==6 && device.resource_counts().total()==0,"reset stage residual");
    {
        Edge edge(device);auto* prior=source(edge,destroyed);auto* candidate=source(edge,destroyed);
        DX8Wrapper::Set_Texture(0,prior);prior->Release_Ref();
        const Edge::SourceStageSelection selection{0,candidate};Edge::SourceStageToken token;
        check(edge.begin_source_stages(desc(edge,&selection),token),"destructor stage admission");stage(edge,token,candidate);
        candidate->Release_Ref(); // No caller owner survives; edge cancellation must retire both once.
    }
    check(destroyed==8 && device.resource_counts().total()==0,"stage destructor residual");
    {
        Edge edge(device);auto* prior=source(edge,destroyed);auto* candidate=source(edge,destroyed);
        DX8Wrapper::Set_Texture(0,prior);const Edge::SourceStageSelection selection{0,candidate};
        Edge::SourceStageToken token;check(edge.begin_source_stages(desc(edge,&selection),token),"removal stage admission");
        stage(edge,token,candidate);candidate->Invalidate();
        check(!edge.source_stages_active() && !edge.commit_source_stages(token) && !edge.abort_source_stages(token)
            && !edge.queued_source_reference_count() && !edge.reserved_source_reference_count()
            && DX8Wrapper::Peek_Texture(0)==prior && prior->Num_Refs()==2 && candidate->Num_Refs()==1
            && !candidate->Is_Initialized() && candidate->accessed()==12345,
            "source removal failed to cancel live stage owner");
        check(!edge.begin_source_stages(desc(edge,&selection),token),"removed source retry was admitted");
        candidate->Release_Ref();prior->Release_Ref();DX8Wrapper::Reset_Source_State();
    }
    check(destroyed==10 && device.resource_counts().total()==0,"removed stage residual");
}
struct Scene {
    BgfxGpuDevice& device;ShaderHandle vs,fs;PipelineHandle pipeline;
    BufferHandle uniform,vertices,indices;TextureHandle color,depth;SamplerHandle sampler;
    explicit Scene(BgfxGpuDevice& owner):device(owner)
    {
        vs=device.create_shader({ShaderStage::vertex,"renderer/video.vert",1,0},"stage viewport");
        fs=device.create_shader({ShaderStage::fragment,"renderer/video.frag",0,1},"stage sample");
        PipelineDesc pipeline_desc;pipeline_desc.vertex_shader=vs;pipeline_desc.fragment_shader=fs;
        pipeline_desc.vertex_layout=VertexLayout::position_color_uv;pipeline_desc.color_format=TextureFormat::bgra8;
        pipeline_desc.raster.cull=CullMode::none;
        pipeline=device.create_pipeline(PipelineKey(pipeline_desc),"stage sample");
        std::array<float,4> viewport{64,64,0,0};
        struct Vertex { float x,y;UInt32 diffuse;float u,v; };
        std::array<Vertex,3> geometry{{{0,0,0xffffffff,0,0},{128,0,0xffffffff,2,0},{0,128,0xffffffff,0,2}}};
        std::array<UInt16,3> order{0,1,2};
        uniform=device.create_buffer({sizeof(viewport),BufferUsage::uniform,true},"stage viewport");
        vertices=device.create_buffer({sizeof(geometry),BufferUsage::vertex,true},"stage geometry");
        indices=device.create_buffer({sizeof(order),BufferUsage::index,true},"stage indices");
        check(vs && fs && pipeline && uniform && vertices && indices
            && device.upload({uniform,sizeof(viewport),0,sizeof(viewport)},viewport.data())
            && device.upload({vertices,sizeof(geometry),0,sizeof(geometry)},geometry.data())
            && device.upload({indices,sizeof(order),0,sizeof(order)},order.data()),"stage sample resources");
        TextureDesc target;target.width=target.height=64;target.render_target=true;target.format=TextureFormat::bgra8;
        color=device.create_texture(target,"stage target");target.format=TextureFormat::depth24_stencil8;
        depth=device.create_texture(target,"stage depth");
        check(color && depth,"stage sample targets");
    }
    std::vector<UInt8> render(TextureHandle texture,SamplerHandle sampler)
    {
        RenderPassDesc pass;pass.color_targets[0]=color;pass.color_target_count=1;
        pass.depth_target=depth;pass.width=pass.height=64;
        DrawDesc draw;draw.pipeline=pipeline;draw.vertex_buffer=vertices;draw.index_buffer=indices;
        draw.index_element_size=IndexElementSize::uint16;draw.vertex_or_index_count=3;
        draw.vertex_bindings.uniform_count=1;draw.vertex_bindings.uniforms[0]={uniform,0,16};
        draw.fragment_bindings.texture_count=1;draw.fragment_bindings.textures[0]=texture;
        draw.fragment_bindings.samplers[0]=sampler;
        check(device.begin_pass(pass,"generated stage identity sample") && device.draw(draw) && device.end_pass(),
            "stage sample draw");return device.readback_rgba(color);
    }
    ~Scene() {device.destroy(pipeline);device.destroy(vs);device.destroy(fs);device.destroy(uniform);
        device.destroy(vertices);device.destroy(indices);device.destroy(color);device.destroy(depth);}
};
void physical()
{
    BgfxOptions options;options.shader_root=ZH_BGFX_SHADER_DIR;BgfxGpuDevice device(options);unsigned destroyed=0;
    {
        Scene scene(device);Edge edge(device);auto* prior=source(edge,destroyed);auto* candidate=source(edge,destroyed);
        DX8Wrapper::Set_Texture(0,prior);prior->Apply(0);prior->access(12345);
        DX8Wrapper::Set_Texture(3,prior);DX8Wrapper::Set_Shader(ShaderClass::_PresetOpaqueShader);
        const Baseline baseline(edge);const auto prior_pending=edge.pending_stage(0);
        const auto pixels=scene.render(prior_pending.texture,prior_pending.sampler);
        const unsigned center=(32*64+32)*4;
        check(pixels.size()==64*64*4 && pixels[center]==0 && pixels[center+1]==255 && pixels[center+2]==0,
            "physical stage baseline sampling");
        Edge::SourceStageSelection selection{0,candidate};const auto good=desc(edge,&selection);Edge::SourceStageToken token;
        auto malformed=good;malformed.transform_nodes=9;
        const auto frames=device.native_frame_advance_count(),retires=device.native_retirement_destroy_count();
        check(!edge.begin_source_stages(malformed,token) && baseline.equals(edge) && prior->Num_Refs()==3
            && candidate->Num_Refs()==1 && device.native_frame_advance_count()==frames
            && device.native_retirement_destroy_count()==retires,"physical bound rejection touched accepted owner");
        for (unsigned fault=0;fault<2;++fault) {
            check(edge.begin_source_stages(good,token),"physical stage admission");stage(edge,token,candidate);
            if (fault==0) edge.fail_next_source_stage_commit();
            else rejects([&]{DX8Wrapper::Set_Transform(D3DTS_VIEW,Matrix4x4(true));},"physical outside-stage mutation accepted");
            check(!edge.commit_source_stages(token) && edge.abort_source_stages(token)
                && edge.drain_source_references(edge.generation()) && baseline.equals(edge)
                && edge.pending_stage(0).sampler==prior_pending.sampler && candidate->accessed()==12345
                && prior->Num_Refs()==3 && candidate->Num_Refs()==1
                && device.native_frame_advance_count()==frames && device.native_retirement_destroy_count()==retires,
                "physical stage rollback touched accepted identity or native boundary");
        }
        check(scene.render(prior_pending.texture,prior_pending.sampler)==pixels,"physical rejection changed sampled pixels");
        const auto retry_frames=device.native_frame_advance_count(),retry_retires=device.native_retirement_destroy_count();
        check(edge.begin_source_stages(good,token),"physical stage retry admission");stage(edge,token,candidate);
        check(edge.commit_source_stages(token) && edge.drain_source_references(edge.generation())
            && !edge.queued_source_reference_count() && !edge.reserved_source_reference_count()
            && device.native_frame_advance_count()==retry_frames && device.native_retirement_destroy_count()==retry_retires,
            "physical successful stage consumed native boundary or left pins");
        const auto accepted=edge.pending_stage(0);
        check(accepted.source==candidate && accepted.sampler!=prior_pending.sampler
            && scene.render(accepted.texture,accepted.sampler)==pixels,"physical retry publication or sampler pixels");
        prior->Release_Ref();candidate->Release_Ref();DX8Wrapper::Reset_Source_State();
    }
    check(destroyed==2 && device.wait_idle() && device.live_resource_count()==0,"physical stage residual");
}
}
int main(int argc,char** argv)
{
    try {
        struct Services {
            Services(){char reason[64]{};check(zh::original_process::initialize_services(0,reason,sizeof(reason)),"stage process services");}
            ~Services(){zh::original_process::shutdown_services();}
        } services;
        for (unsigned generation=0;generation<2;++generation)
            if (argc==2 && std::string(argv[1])=="--gpu") physical();else recording();
        std::cout<<"original stage transaction: ok\n";return 0;
    } catch (const std::exception& error) { std::cerr<<error.what()<<'\n';return 1; }
}
