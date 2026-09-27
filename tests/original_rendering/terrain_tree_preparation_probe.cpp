#include "PreRTS.h"
#include "Common/GlobalData.h"
#include "Common/FileSystem.h"
#include "Common/MapReaderWriterInfo.h"
#include "GameClient/ClientRandomValue.h"
#include "GameClient/FXList.h"
#include "GameLogic/GameLogic.h"
#include "GameLogic/Object.h"
#include "GameLogic/PartitionManager.h"
#include "Common/PlayerList.h"
#include "Common/Player.h"
#include "W3DDevice/GameClient/HeightMap.h"
#include "W3DDevice/GameClient/W3DDisplay.h"
#include "W3DDevice/GameClient/W3DTerrainVisual.h"
#include "W3DDevice/GameClient/W3DView.h"
#include "W3DDevice/GameClient/W3DShroud.h"
#include "W3DDevice/GameClient/WorldHeightMap.h"
#include "W3DDevice/GameClient/W3DAssetManager.h"
#include "W3DDevice/GameClient/W3DScene.h"
#include "W3DDevice/GameClient/W3DTerrainTracks.h"
#include "W3DDevice/GameClient/W3DParticleSys.h"
#include "W3DDevice/GameClient/W3DSmudge.h"
#include "WW3D2/dx8vertexbuffer.h"
#include "WW3D2/light.h"
#include "WW3D2/dx8wrapper.h"
#include "WW3D2/ww3d.h"
#include "W3DDevice/GameClient/Module/W3DTreeDraw.h"
#include "WWLib/RAMFILE.H"
#include "assetmgr.h"
#include "camera.h"
#include "original_gpu_edge.h"
#include "tree_projected_decal_cpu.h"
#include "zh/renderer/recording_device.h"
#include "zh/platform/bgfx_device.h"
#include <SDL3/SDL.h>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <iterator>
#include <stdexcept>
#include <vector>

// Generated-only access to private owner checkpoints, with no shipping selector.
struct W3DFrameGeneratedProbeAccess {
    static void failPresent(zh::original_runtime::OriginalGpuEdge &edge)
    { edge.source_frame_present_fault_=true; }
    static void failCommit(zh::original_runtime::OriginalGpuEdge &edge)
    { edge.source_frame_commit_fault_=true; }
    static bool faultsConsumed(const zh::original_runtime::OriginalGpuEdge &edge)
    { return !edge.source_frame_present_fault_ && !edge.source_frame_commit_fault_; }
    static bool queuesConsumed(const W3DParticleSystemManager &particles)
    { return !static_cast<W3DSmudgeManager *>(TheSmudgeManager)->m_usedSmudgeSetList.Head() && !particles.m_readyToRender; }
    static std::array<unsigned,4> textureMetadata(const TextureBaseClass &texture)
    { return {unsigned(texture.Initialized),texture.LastAccessed,texture.LastInactivationSyncTime,texture.ExtendedInactivationTime}; }
    static unsigned extraTextureCapacity(const zh::original_runtime::OriginalGpuEdge &edge)
    {
        std::vector<const TextureBaseClass *> identities;
        const auto add=[&](const TextureBaseClass *texture) {
            for (auto *identity:identities) if (identity==texture) return;
            identities.push_back(texture);
        };
        for (const auto &entry:edge.textures_) add(entry.first);
        unsigned entries=0;
        auto *assets=WW3DAssetManager::Get_Instance();
        HashTemplateIterator<StringClass,TextureClass *> textures(assets->Texture_Hash());
        for (textures.First();!textures.Is_Done();textures.Next()) { ++entries;add(textures.Peek_Value()); }
        return 4096-std::max(entries,unsigned(identities.size()));
    }
    static bool sameSource(const DX8Wrapper::SourceStateSnapshot &left,const DX8Wrapper::SourceStateSnapshot &right)
    {
        if (left.render!=right.render || left.stages!=right.stages || left.transforms.size()!=right.transforms.size()
            || left.fog_enabled!=right.fog_enabled || left.fog_color!=right.fog_color
            || left.material_applied!=right.material_applied || left.light_enabled!=right.light_enabled
            || left.light_environment_selected!=right.light_environment_selected) return false;
        for (const auto &entry:left.transforms) {
            const auto found=right.transforms.find(entry.first);if (found==right.transforms.end()) return false;
            for (unsigned row=0;row<4;++row) for (unsigned column=0;column<4;++column)
                if (entry.second[row][column]!=found->second[row][column]) return false;
        }
        return true;
    }
    static std::vector<std::uint64_t> image(CameraClass *camera)
    {
        std::vector<std::uint64_t> result;
        const auto pointer=[&](const void *value) { result.push_back(reinterpret_cast<std::uintptr_t>(value)); };
        const auto real=[&](float value) { std::uint32_t bits;std::memcpy(&bits,&value,sizeof(bits));result.push_back(bits); };
        const auto vector=[&](const Vector3 &value) { real(value.X);real(value.Y);real(value.Z); };
        const auto bytes=[&](DX8VertexBufferClass *source) {
            pointer(source);
            const auto size=source?std::size_t(source->Get_Vertex_Count())*source->FVF_Info().Get_FVF_Size():0;
            result.push_back(size);
            for (std::size_t i=0;i<size;++i) result.push_back(source->Get_CPU_Vertex_Buffer()[i]);
        };
        auto *tracks=TheTerrainTracksRenderObjClassSystem;
        result.push_back(tracks->m_edgesToFlush);bytes(tracks->m_vertexBuffer);
        for (auto *head:{tracks->m_usedModules,tracks->m_freeModules}) {
            for (;head;head=head->m_nextSystem) pointer(head);
            pointer(nullptr);
        }
        auto *particles=dynamic_cast<W3DParticleSystemManager *>(TheParticleSystemManager);
        pointer(particles);
        if (particles) { result.push_back(particles->m_readyToRender);result.push_back(particles->m_onScreenParticleCount); }
        auto *smudges=dynamic_cast<W3DSmudgeManager *>(TheSmudgeManager);
        result.push_back(smudges->m_smudgeCountLastFrame);bytes(smudges->m_vertexBuffer);
        for (auto *head:{smudges->m_usedSmudgeSetList.Head(),smudges->m_freeSmudgeSetList.Head()}) {
            for (;head;head=head->Succ()) {
                pointer(head);result.push_back(head->getUsedSmudgeCount());
                for (auto *smudge=head->getUsedSmudgeList().Head();smudge;smudge=smudge->Succ()) pointer(smudge);
                pointer(nullptr);
            }
            pointer(nullptr);
        }
        auto *scene=W3DDisplay::m_3DScene;
        result.push_back(scene->Visibility_Checked);pointer(scene->m_camera);pointer(camera->Get_User_Data());
        vector(scene->m_infantryAmbient);
        for (const auto *environment:{&scene->m_defaultLightEnv,&scene->m_foggedLightEnv}) {
            vector(environment->Get_Equivalent_Ambient());result.push_back(environment->Get_Light_Count());
            for (Int i=0;i<environment->Get_Light_Count();++i) {
                vector(environment->Get_Light_Direction(i));vector(environment->Get_Light_Diffuse(i));
            }
        }
        for (unsigned stage=0;stage<8;++stage) pointer(DX8Wrapper::Peek_Texture(stage));
        pointer(DX8Wrapper::Peek_Material());result.push_back(DX8Wrapper::Pending_Changes());
        RefRenderObjListIterator render(&scene->RenderList),update(&scene->UpdateList);
        for (render.First();!render.Is_Done();render.Next()) {
            auto *object=render.Peek_Obj();pointer(object);result.push_back(object->Is_Visible());
            const auto &transform=object->Get_Transform_No_Validity_Check();
            for (unsigned row=0;row<3;++row) for (unsigned column=0;column<4;++column) real(transform[row][column]);
        }
        pointer(nullptr);
        for (update.First();!update.Is_Done();update.Next()) pointer(update.Peek_Obj());
        pointer(nullptr);
        for (Int i=-1;i<scene->m_numGlobalLights;++i) {
            auto *light=i<0?scene->m_scratchLight:scene->m_infantryLight[i];pointer(light);
            if (!light) continue;
            Vector3 diffuse,ambient;light->Get_Diffuse(&diffuse);light->Get_Ambient(&ambient);vector(diffuse);vector(ambient);
            const auto &transform=light->Get_Transform_No_Validity_Check();
            for (unsigned row=0;row<3;++row) for (unsigned column=0;column<4;++column) real(transform[row][column]);
        }
        result.push_back(WW3D::Get_Frame_Count());
        result.push_back(WW3D::Get_Last_Frame_Memory_Allocation_Count());result.push_back(WW3D::Get_Last_Frame_Memory_Free_Count());
        return result;
    }
    static void roundTrip(CameraClass *camera)
    {
        auto check=[](bool value) { if (!value) throw std::runtime_error("tree source checkpoint round trip rejected"); };
        auto buffer=[](DX8VertexBufferClass *source) {
            if (!source) return std::vector<unsigned char>{};
            const auto size=std::size_t(source->Get_Vertex_Count())*source->FVF_Info().Get_FVF_Size();
            const auto *bytes=source->Get_CPU_Vertex_Buffer();
            return std::vector<unsigned char>(bytes,bytes+size);
        };
        auto *tracks=TheTerrainTracksRenderObjClassSystem;
        const auto trackBytes=buffer(tracks->m_vertexBuffer);
        const auto trackEdges=tracks->m_edgesToFlush;
        auto *used=tracks->m_usedModules;auto *free=tracks->m_freeModules;
        auto modules=[](TerrainTracksRenderObjClass *head) {
            std::vector<TerrainTracksRenderObjClass *> identities;
            for (;head;head=head->m_nextSystem) identities.push_back(head);
            return identities;
        };
        const auto usedOrder=modules(used),freeOrder=modules(free);
        auto track=tracks->captureSourceFrame();
        tracks->m_edgesToFlush=trackEdges+3;
        if (!trackBytes.empty()) tracks->m_vertexBuffer->Get_CPU_Vertex_Buffer()[0]^=0x5a;
        tracks->restoreSourceFrame(*track);
        check(tracks->m_edgesToFlush==trackEdges && tracks->m_usedModules==used && tracks->m_freeModules==free &&
            buffer(tracks->m_vertexBuffer)==trackBytes && modules(tracks->m_usedModules)==usedOrder &&
            modules(tracks->m_freeModules)==freeOrder);
        W3DParticleSystemManager unselectedParticleOwner;
        auto *particles=dynamic_cast<W3DParticleSystemManager *>(TheParticleSystemManager);
        if (!particles) particles=&unselectedParticleOwner;
        const auto ready=particles->m_readyToRender;const auto screen=particles->m_onScreenParticleCount;
        auto particle=particles->captureSourceFrame();
        particles->m_readyToRender=!ready;particles->m_onScreenParticleCount=screen+7;
        particles->restoreSourceFrame(*particle);
        check(particles->m_readyToRender==ready && particles->m_onScreenParticleCount==screen);
        auto *smudges=dynamic_cast<W3DSmudgeManager *>(TheSmudgeManager);check(smudges!=nullptr);
        const auto smudgeBytes=buffer(smudges->m_vertexBuffer);
        const auto last=smudges->m_smudgeCountLastFrame;
        auto *usedSet=smudges->m_usedSmudgeSetList.Head();auto *freeSet=smudges->m_freeSmudgeSetList.Head();
        auto setOrder=[](SmudgeSet *head) {
            std::vector<SmudgeSet *> result;for (;head;head=head->Succ()) result.push_back(head);return result;
        };
        auto smudgeOrder=[](SmudgeSet *head) {
            std::vector<Smudge *> result;
            for (;head;head=head->Succ())
                for (auto *smudge=head->getUsedSmudgeList().Head();smudge;smudge=smudge->Succ()) result.push_back(smudge);
            return result;
        };
        const auto usedSets=setOrder(usedSet),freeSets=setOrder(freeSet);
        const auto usedSmudges=smudgeOrder(usedSet);
        auto smudge=smudges->captureSourceFrame();
        smudges->m_smudgeCountLastFrame=last+2;
        if (!smudgeBytes.empty()) smudges->m_vertexBuffer->Get_CPU_Vertex_Buffer()[0]^=0xa5;
        smudges->restoreSourceFrame(*smudge);
        check(smudges->m_smudgeCountLastFrame==last && smudges->m_usedSmudgeSetList.Head()==usedSet &&
            smudges->m_freeSmudgeSetList.Head()==freeSet && buffer(smudges->m_vertexBuffer)==smudgeBytes &&
            setOrder(smudges->m_usedSmudgeSetList.Head())==usedSets &&
            setOrder(smudges->m_freeSmudgeSetList.Head())==freeSets &&
            smudgeOrder(smudges->m_usedSmudgeSetList.Head())==usedSmudges);
        auto *scene=W3DDisplay::m_3DScene;
        const auto visibility=scene->Visibility_Checked;const auto user=camera->Get_User_Data();
        const auto defaultEnv=scene->m_defaultLightEnv;const auto fogEnv=scene->m_foggedLightEnv;
        const auto ambient=scene->m_infantryAmbient;auto *sceneCamera=scene->m_camera;
        struct LightState { LightClass *identity;Vector3 diffuse,ambient;Matrix3D transform; };
        std::vector<LightState> lights;
        for (Int i=0;i<scene->m_numGlobalLights;++i) {
            LightState state{scene->m_infantryLight[i],Vector3(0,0,0),Vector3(0,0,0),Matrix3D(true)};
            state.identity->Get_Diffuse(&state.diffuse);state.identity->Get_Ambient(&state.ambient);
            state.transform=state.identity->Get_Transform_No_Validity_Check();lights.push_back(state);
        }
        if (scene->m_scratchLight) {
            LightState state{scene->m_scratchLight,Vector3(0,0,0),Vector3(0,0,0),Matrix3D(true)};
            state.identity->Get_Diffuse(&state.diffuse);state.identity->Get_Ambient(&state.ambient);
            state.transform=state.identity->Get_Transform_No_Validity_Check();lights.push_back(state);
        }
        std::vector<RenderObjClass *> renderOrder,updateOrder;
        RefRenderObjListIterator render(&scene->RenderList),update(&scene->UpdateList);
        for (render.First();!render.Is_Done();render.Next()) renderOrder.push_back(render.Peek_Obj());
        for (update.First();!update.Is_Done();update.Next()) updateOrder.push_back(update.Peek_Obj());
        auto saved=scene->captureSourceFrame(camera);
        // Native visibility returns its encoded mask, not canonical bool 1.
        std::vector<int> visible;
        for (auto *object:renderOrder) { visible.push_back(object->Is_Visible());object->Set_Visible(!object->Is_Visible()); }
        for (const auto &light:lights) {
            light.identity->Set_Diffuse(Vector3(0.1f,0.2f,0.3f));light.identity->Set_Ambient(Vector3(0.3f,0.2f,0.1f));
            light.identity->Set_Transform(Matrix3D(true));
        }
        scene->Visibility_Checked=!visibility;scene->m_camera=camera;camera->Set_User_Data(scene);
        scene->m_defaultLightEnv.Reset(Vector3(1,2,3),Vector3(0.2f,0.3f,0.4f));
        scene->m_foggedLightEnv.Reset(Vector3(3,2,1),Vector3(0.4f,0.3f,0.2f));
        scene->m_infantryAmbient=Vector3(0.5f,0.5f,0.5f);
        scene->restoreSourceFrame(*saved);
        check(scene->Visibility_Checked==visibility);check(scene->m_camera==sceneCamera);check(camera->Get_User_Data()==user);
        check(scene->m_defaultLightEnv==defaultEnv);check(scene->m_foggedLightEnv==fogEnv);
        check(scene->m_infantryAmbient==ambient);
        for (unsigned i=0;i<renderOrder.size();++i) check(renderOrder[i]->Is_Visible()==visible[i]);
        for (const auto &light:lights) {
            Vector3 diffuse,lightAmbient;light.identity->Get_Diffuse(&diffuse);light.identity->Get_Ambient(&lightAmbient);
            check(diffuse==light.diffuse && lightAmbient==light.ambient);
            for (unsigned row=0;row<3;++row) for (unsigned column=0;column<4;++column)
                check(light.identity->Get_Transform_No_Validity_Check()[row][column]==light.transform[row][column]);
        }
        unsigned ordinal=0;
        for (render.First();!render.Is_Done();render.Next()) check(ordinal<renderOrder.size() && render.Peek_Obj()==renderOrder[ordinal++]);
        check(ordinal==renderOrder.size());ordinal=0;
        for (update.First();!update.Is_Done();update.Next()) check(ordinal<updateOrder.size() && update.Peek_Obj()==updateOrder[ordinal++]);
        check(ordinal==updateOrder.size());
    }
};

extern "C" void zh_probe_source_frame_checkpoint_roundtrip(CameraClass *camera)
{ W3DFrameGeneratedProbeAccess::roundTrip(camera); }

namespace {
void require(bool condition,const char* message)
{ if (!condition) throw std::runtime_error(message); }
// Generated-only authored map-state adapter: the CPU owner has no linked editor
// setters. Production queue construction still reads the real native fields.
class GeneratedDecalMap final:public WorldHeightMap {
public:
    explicit GeneratedDecalMap(ChunkInputStream* input):WorldHeightMap(input,FALSE) {}
    void origin(Int x,Int y) { m_drawOriginX=x;m_drawOriginY=y; }
    void flip(Int x,Int y,bool value) {
        auto& bits=m_cellFlipState[y*m_flipStateWidth+(x>>3)];
        if (value) bits|=1<<(x&7);else bits&=~(1<<(x&7));
    }
};
template<class Operation> bool rejected(Operation operation)
{ try { operation(); } catch (const std::runtime_error&) { return true; } return false; }
class PreparedPhaseFX final : public FXNugget {
    MEMORY_POOL_GLUE_WITH_EXPLICIT_CREATE(PreparedPhaseFX,"PreparedPhaseFX",4,4)
public:
    PreparedPhaseFX(Int* count,BaseHeightMapRenderObjClass* terrain,W3DTerrainVisual* visual)
        : count_(count),terrain_(terrain),visual_(visual) {}
    void doFXPos(const Coord3D*,const Matrix3D*,Real,const Coord3D*,Real) const override
    {
        ++*count_;
        if (visual_) visual_->reset();
        else if (terrain_) terrain_->removeAllTrees();
    }
    FXPositionNuggetReadiness cpuPositionReady(const Coord3D*,const Coord3D*) const override
    { return FXPositionNuggetReadiness::Ready; }
private:
    Int* count_;
    BaseHeightMapRenderObjClass* terrain_;
    W3DTerrainVisual* visual_;
};
EMPTY_DTOR(PreparedPhaseFX)
// Generated logical partition extents exceed the tiny visual map. Capture
// exact native reveal/undo notices without replacing the real shroud query.
class PreparationShroudNotices final : public Display {
public:
    struct Notice { Int x,y; CellShroudStatus status; };
    std::vector<Notice> notices;
    void setShroudLevel(Int x,Int y,CellShroudStatus status) override
    { notices.push_back({x,y,status}); }
    void doSmartAssetPurgeAndPreload(const char*) override {}
#if defined(_DEBUG) || defined(_INTERNAL)
    void dumpAssetUsage(const char*) override {}
    void dumpModelAssets(const char*) override {}
#endif
    VideoBuffer* createVideoBuffer() override { return NULL; }
    void setClipRegion(IRegion2D*) override {}
    Bool isClippingEnabled() override { return FALSE; }
    void enableClipping(Bool) override {}
    void setTimeOfDay(TimeOfDay) override {}
    void createLightPulse(const Coord3D*,const RGBColor*,Real,Real,UnsignedInt,UnsignedInt) override {}
    void drawLine(Int,Int,Int,Int,Real,UnsignedInt) override {}
    void drawLine(Int,Int,Int,Int,Real,UnsignedInt,UnsignedInt) override {}
    void drawOpenRect(Int,Int,Int,Int,Real,UnsignedInt) override {}
    void drawFillRect(Int,Int,Int,Int,UnsignedInt) override {}
    void drawRectClock(Int,Int,Int,Int,Int,UnsignedInt) override {}
    void drawRemainingRectClock(Int,Int,Int,Int,Int,UnsignedInt) override {}
    void drawImage(const Image*,Int,Int,Int,Int,Color,DrawImageMode) override {}
    void drawVideoBuffer(VideoBuffer*,Int,Int,Int,Int) override {}
    void clearShroud() override {}
    void setBorderShroudLevel(UnsignedByte) override {}
    void preloadModelAssets(AsciiString) override {}
    void preloadTextureAssets(AsciiString) override {}
    void takeScreenShot() override {}
    void toggleMovieCapture() override {}
    void toggleLetterBox() override {}
    void enableLetterBox(Bool) override {}
    Real getAverageFPS() override { return 0; }
    Int getLastFrameDrawCalls() override { return 0; }
};

void recordingBoundaries(const char *path,const char *asset,bool projected=false)
{
    const auto shadowSetting=TheWritableGlobalData->m_useShadowDecals;
    // Aborted candidate IDs retain generation tombstones by design. Prove
    // their bounded admission separately; never inflate the frame budget to
    // accommodate an ever-growing diagnostic sweep in one device.
    {
        zh::renderer::RecordingGpuDevice device;
        const auto accepted=device.create_buffer({4,zh::renderer::BufferUsage::vertex,true},"accepted tombstone control");
        const unsigned value=0x12345678;
        require(accepted && device.upload({accepted,4,0,4},&value),"tombstone accepted baseline rejected");
        const auto baseline=device.snapshot();const auto bytes=device.buffer_bytes(accepted);
        const zh::renderer::DeviceTransactionDesc budget{zh::renderer::DeviceTransactionMode::idle_preparation,1,4,4,4096,0};
        for (unsigned i=0;i<3;++i) {
            zh::renderer::DeviceTransactionToken token;
            require(device.begin_device_transaction(budget,token),"tombstone bounded candidate admission rejected");
            const auto candidate=device.create_buffer({4,zh::renderer::BufferUsage::vertex,true},"aborted candidate");
            require(candidate && candidate!=accepted && device.abort_device_transaction(token)
                && device.snapshot()==baseline && device.buffer_bytes(accepted)==bytes,
                "tombstone abort altered accepted identity/bytes");
        }
        zh::renderer::DeviceTransactionToken token;
        require(device.begin_device_transaction(budget,token)
            && !device.create_buffer({4,zh::renderer::BufferUsage::vertex,true},"capacity bound plus one")
            && !device.commit_device_transaction(token) && device.abort_device_transaction(token)
            && device.snapshot()==baseline && device.buffer_bytes(accepted)==bytes
            && device.resource_counts().buffers==1,"tombstone capacity rejection touched accepted baseline");
        device.destroy(accepted);require(!device.resource_counts().total(),"tombstone control retained live resource");
    }
    unsigned completedOrdinal=0;
    for (;completedOrdinal<4096;++completedOrdinal) {
        if (projected) TheWritableGlobalData->m_useShadowDecals=FALSE;
        bool failed=false;
        // One fresh equivalent device/source owner per ordinal; the failed
        // attempt and its clean retry still share one immutable accepted phase.
        zh::renderer::RecordingGpuDevice device;
        {
            zh::original_runtime::OriginalGpuEdge edge(device);
            W3DDisplay display;display.init();
            auto *savedDisplay=TheDisplay;TheDisplay=&display;
            auto *savedVisual=TheTerrainVisual;auto *savedView=TheTacticalView;
            W3DTerrainVisual visual;TheTerrainVisual=&visual;
            try {
                display.setWidth(32);display.setHeight(24);visual.init();
                require(visual.load(AsciiString(path)),"journal boundary map load rejected");
                std::ifstream input(asset,std::ios::binary);
                std::vector<char> bytes(std::istreambuf_iterator<char>{input},std::istreambuf_iterator<char>{});
                RAMFileClass packet(bytes.data(),static_cast<int>(bytes.size()));
                require(!bytes.empty() && static_cast<WW3DAssetManager *>(W3DDisplay::m_assetManager)->Load_3D_Assets(packet),
                    "journal boundary model load rejected");
                auto *view=new W3DView;TheTacticalView=view;view->init();display.attachView(view);
                view->setWidth(32);view->setHeight(24);view->setDefaultView(0,0,1);
                auto *camera=view->get3DCamera();camera->Set_Clip_Planes(0.1f,1000.0f);
                camera->Set_Position(Vector3(12,18,30));
                zh::renderer::TextureDesc target;target.width=32;target.height=24;
                target.render_target=true;target.sampled=false;target.format=zh::renderer::TextureFormat::rgba8;
                const auto color=device.create_texture(target,"journal boundary color");
                target.format=zh::renderer::TextureFormat::depth24_stencil8;
                const auto depth=device.create_texture(target,"journal boundary depth");
                require(color && depth,"journal boundary targets rejected");edge.bind_frame_targets(color,depth,32,24);
                display.draw();require(device.present(color),"journal boundary ordinary baseline rejected");
                W3DTreeDrawModuleData data;data.m_modelName="TEST.LITONE01";data.m_textureName="Tree0.tga";
                if (projected) { data.m_doShadow=TRUE;TheWritableGlobalData->m_useShadowDecals=TRUE; }
                const Coord3D position{12,18,2};
                auto *terrain=TheTerrainRenderObject;
                require(terrain->tryAddTree(static_cast<DrawableID>(801),position,1,0,0,&data),
                    "journal boundary tree admission rejected");camera->Set_Position(Vector3(12,18,30));
                require(terrain->prepareTreeRenderPhase(camera)==BaseHeightMapRenderObjClass::TREE_PHASE_READY,
                    "journal boundary immutable phase rejected");
                const auto identity=terrain->preparedTreePhaseIdentity();
                const auto* queue=projected?zh::original_runtime::detail::TreeDecalGeneratedProbeAccess::peek(terrain):nullptr;
                const auto* texture=projected?zh::original_runtime::detail::TreeDecalGeneratedProbeAccess::texture(terrain):nullptr;
                const auto baseline=device.snapshot();const auto resources=device.resource_counts();
                const auto owners=W3DFrameGeneratedProbeAccess::image(camera);
                const auto source=DX8Wrapper::Inspect_Source_State();
                const auto *vertex=terrain->peekTreeVertexSource();const auto sway=terrain->treeSwayVector(0);
                UnsignedInt before[6],after[6];CopyGameClientRandomState(before);
                if (!completedOrdinal) {
                    device.fail_next_transaction_checkpoint();
                    require(rejected([&]{display.draw();}) && terrain->preparedTreePhaseIdentity()==identity
                        && device.snapshot()==baseline && device.resource_counts()==resources
                        && W3DFrameGeneratedProbeAccess::image(camera)==owners,
                        "journal begin rejection changed admitted source baseline");
                }
                device.fail_transaction_operation_after(completedOrdinal);
                failed=rejected([&]{display.draw();});CopyGameClientRandomState(after);
                if (failed) {
                    require(terrain->preparedTreePhaseIdentity()==identity && !edge.tree_source_frame_pending()
                        && (!projected || (queue && queue->intents.size()==1 && !queue->batches.empty()
                            && zh::original_runtime::detail::TreeDecalGeneratedProbeAccess::peek(terrain)==queue
                            && zh::original_runtime::detail::TreeDecalGeneratedProbeAccess::texture(terrain)==texture))
                        && !device.pass_active() && device.snapshot()==baseline && device.resource_counts()==resources
                        && W3DFrameGeneratedProbeAccess::image(camera)==owners
                        && W3DFrameGeneratedProbeAccess::sameSource(source,DX8Wrapper::Inspect_Source_State())
                        && terrain->peekTreeVertexSource()==vertex && terrain->treeSwayVector(0)==sway
                        && std::memcmp(before,after,sizeof(before))==0,
                        "journal operation fault changed baseline/phase/C1/RNG");
                    display.draw();CopyGameClientRandomState(after);
                }
                device.fail_transaction_operation_after(~0U);
                require(!terrain->preparedTreePhaseIdentity() && device.operation_counts().presents==2
                    && terrain->peekTreeVertexSource()==vertex && terrain->treeSwayVector(0)==sway
                    && std::memcmp(before,after,sizeof(before))==0,
                    "journal operation once-only retry did not complete exact phase/frame");
                visual.reset();edge.release_source_buffers();device.destroy(depth);device.destroy(color);
            } catch (...) {
                TheTacticalView=savedView;TheTerrainVisual=savedVisual;TheDisplay=savedDisplay;throw;
            }
            TheTacticalView=savedView;TheTerrainVisual=savedVisual;TheDisplay=savedDisplay;
        }
        require(!device.resource_counts().total(),"journal boundary source generation retained resources");
        if (!failed) break;
    }
    require(completedOrdinal>15 && completedOrdinal<4096,"journal boundary sweep incomplete/unbounded");
    std::printf("original tree draw boundaries: rejected=%u tombstones=1 retry=1\n",completedOrdinal);
    TheWritableGlobalData->m_useShadowDecals=shadowSetting;
}

void physicalDraw(const char *path,const char *asset)
{
    const bool projected=std::getenv("ZH_M22_TREE_DECAL_PHYSICAL")!=nullptr;
    const auto priorShadows=TheWritableGlobalData->m_useShadowDecals;
    require(SDL_Init(SDL_INIT_VIDEO),"physical tree video service rejected");
    auto *window=SDL_CreateWindow("generated source tree frame",32,24,SDL_WINDOW_HIDDEN);
    require(window!=nullptr,"physical tree window provider rejected");
    const auto paused=TheGameLogic->isGamePaused();TheGameLogic->setGamePaused(TRUE,FALSE);
    for (unsigned generation=0;generation<2;++generation) {
        if (projected) TheWritableGlobalData->m_useShadowDecals=FALSE;
        // linux_main owns shipping allocation/filesystem services for the
        // complete worker lifetime, just as in the independent program route.
        zh::renderer::BgfxOptions options;options.shader_root=ZH_BGFX_SHADER_DIR;
        zh::renderer::BgfxGpuDevice device(options);
        require(device.claim_window(window),"physical tree presentation provider rejected");
        {
            zh::original_runtime::OriginalGpuEdge edge(device);
            W3DDisplay display;display.init();
            auto *savedDisplay=TheDisplay;TheDisplay=&display;
            auto *savedVisual=TheTerrainVisual;auto *savedView=TheTacticalView;
            W3DTerrainVisual visual;TheTerrainVisual=&visual;
            try {
                display.setWidth(32);display.setHeight(24);visual.init();
                require(visual.load(AsciiString(path)),"physical tree draw map load rejected");
                std::ifstream input(asset,std::ios::binary);
                std::vector<char> bytes(std::istreambuf_iterator<char>{input},std::istreambuf_iterator<char>{});
                RAMFileClass packet(bytes.data(),static_cast<int>(bytes.size()));
                require(!bytes.empty() && static_cast<WW3DAssetManager *>(W3DDisplay::m_assetManager)->Load_3D_Assets(packet),
                    "physical tree draw model provider rejected");
                auto *view=new W3DView;TheTacticalView=view;view->init();display.attachView(view);
                view->setWidth(32);view->setHeight(24);view->setDefaultView(0,0,1);
                auto *camera=view->get3DCamera();camera->Set_Clip_Planes(0.1f,1000.0f);
                camera->Set_Position(Vector3(12,18,30));
                zh::renderer::TextureDesc target;target.width=32;target.height=24;
                target.render_target=true;target.sampled=true;target.format=zh::renderer::TextureFormat::rgba8;
                const auto color=device.create_texture(target,"source tree draw color");
                target.format=zh::renderer::TextureFormat::depth24_stencil8;target.sampled=false;
                const auto depth=device.create_texture(target,"source tree draw depth");
                require(color && depth,"physical tree draw targets rejected");
                edge.bind_frame_targets(color,depth,32,24);
                auto *terrain=TheTerrainRenderObject;
                terrain->getShroud()->fillShroudData(255);terrain->getShroud()->render(camera);
                display.draw();require(device.present(color),"physical ordinary frame presentation rejected");
                auto empty=device.readback_rgba(color);
                W3DTreeDrawModuleData data;data.m_modelName="TEST.LITONE01";data.m_textureName="Tree0.tga";
                const auto ground=terrain->getMap()->getDisplayHeight(1,1)*MAP_HEIGHT_SCALE;
                require(ground>2,"physical generated depth-occlusion baseline changed");
                const Coord3D position{12,18,terrain->getMap()->getDisplayHeight(3,3)*MAP_HEIGHT_SCALE+4};
                require(terrain->tryAddTree(static_cast<DrawableID>(701),position,8,0,0,&data),
                    "physical tree source admission rejected");
                camera->Set_Position(Vector3(12,18,position.z+15));
                if (projected) {
                    data.m_doShadow=FALSE;
                    require(terrain->prepareTreeRenderPhase(camera)==BaseHeightMapRenderObjClass::TREE_PHASE_READY,
                        "physical unshadowed tree phase rejected");
                    display.draw();empty=device.readback_rgba(color);
                    data.m_doShadow=TRUE;TheWritableGlobalData->m_useShadowDecals=TRUE;
                }
                const auto* treeVertex=terrain->peekTreeVertexSource();
                const auto* treeBytes=treeVertex->Get_CPU_Vertex_Buffer();
                const std::vector<unsigned char> treeGeometry(treeBytes,
                    treeBytes+std::size_t(treeVertex->Get_Vertex_Count())*treeVertex->FVF_Info().Get_FVF_Size());
                require(terrain->prepareTreeRenderPhase(camera)==BaseHeightMapRenderObjClass::TREE_PHASE_READY,
                    "physical immutable source phase rejected");
                std::vector<bool> projectedCoverage(32*24,false);
                if (projected) {
                    const auto* queue=zh::original_runtime::detail::TreeDecalGeneratedProbeAccess::peek(terrain);
                    Matrix4x4 projection;camera->Get_D3D_Projection_Matrix(&projection);
                    const auto composite=projection*Matrix4x4(camera->Get_View_Matrix());
                    for (const auto& batch:queue->batches) {
                        std::vector<std::array<float,2>> screen;
                        for (const auto& vertex:batch.vertices) {
                            const float input[4]={vertex.x,vertex.y,vertex.z,1};float clip[4]{};
                            for (unsigned row=0;row<4;++row) for (unsigned col=0;col<4;++col)
                                clip[row]+=composite[row][col]*input[col];
                            require(clip[3]>0 && clip[2]>=0 && clip[2]<=clip[3],"physical decal escapes admitted clip/depth");
                            // Pinned Vulkan backend uses a negative-height viewport.
                            screen.push_back({(clip[0]/clip[3]+1)*16,(1-clip[1]/clip[3])*12});
                        }
                        const auto edge=[](const auto& a,const auto& b,float x,float y) {
                            return (b[0]-a[0])*(y-a[1])-(b[1]-a[1])*(x-a[0]);
                        };
                        for (std::size_t index=0;index<batch.indices.size();index+=3) {
                            const auto& a=screen[batch.indices[index]];const auto& b=screen[batch.indices[index+1]];
                            const auto& c=screen[batch.indices[index+2]];
                            for (unsigned y=0;y<24;++y) for (unsigned x=0;x<32;++x) {
                                const float ab=edge(a,b,x+0.5f,y+0.5f),bc=edge(b,c,x+0.5f,y+0.5f),ca=edge(c,a,x+0.5f,y+0.5f);
                                if ((ab>=-0.01f && bc>=-0.01f && ca>=-0.01f) || (ab<=0.01f && bc<=0.01f && ca<=0.01f))
                                    projectedCoverage[y*32+x]=true;
                            }
                        }
                    }
                    unsigned coverage=0,nonblack=0;
                    for (std::size_t pixel=0;pixel<projectedCoverage.size();++pixel) if (projectedCoverage[pixel]) {
                        ++coverage;if (empty[pixel*4]|empty[pixel*4+1]|empty[pixel*4+2]) ++nonblack;
                    }
                    require(coverage>0 && coverage<projectedCoverage.size() && nonblack>0,
                        "physical unshadowed fixture has no bounded nonblack projected terrain region");
                }
                const auto identity=terrain->preparedTreePhaseIdentity();
                UnsignedInt before[6],after[6];CopyGameClientRandomState(before);
                for (unsigned fault=0;fault<2;++fault) {
                    const auto owners=W3DFrameGeneratedProbeAccess::image(camera);
                    const auto live=device.live_resource_count();const auto frames=device.native_frame_advance_count();
                    if (!fault) W3DFrameGeneratedProbeAccess::failPresent(edge);
                    else W3DFrameGeneratedProbeAccess::failCommit(edge);
                    require(rejected([&]{display.draw();}),"physical late source frame fault admitted");
                    CopyGameClientRandomState(after);
                    require(terrain->preparedTreePhaseIdentity()==identity && !edge.tree_source_frame_pending()
                        && !device.pass_active() && device.live_resource_count()==live
                        && device.native_frame_advance_count()==frames
                        && W3DFrameGeneratedProbeAccess::image(camera)==owners
                        && W3DFrameGeneratedProbeAccess::faultsConsumed(edge)
                        && std::memcmp(before,after,sizeof(before))==0,
                        "physical source abort changed phase/owners/native frame/RNG");
                    require(device.readback_rgba(color)==empty,"physical rejected frame touched accepted pixels");
                }
                const auto frames=device.native_frame_advance_count();
                display.draw();CopyGameClientRandomState(after);
                require(!terrain->preparedTreePhaseIdentity() && device.native_frame_advance_count()==frames+1
                    && std::memcmp(before,after,sizeof(before))==0,
                    "physical retry did not complete exactly one source frame");
                const auto accepted=device.readback_rgba(color);
                require(accepted.size()==32*24*4 && accepted!=empty,"physical source tree emitted no distinct pixels");
                if (projected) {
                    unsigned darkened=0;
                    for (std::size_t pixel=0;pixel<accepted.size();pixel+=4) {
                        if (!projectedCoverage[pixel/4]) {
                            require(std::memcmp(accepted.data()+pixel,empty.data()+pixel,4)==0,
                                "physical projected decal changed pixels outside authored coverage");
                            continue;
                        }
                        if (accepted[pixel]>=empty[pixel] || !empty[pixel]) continue;
                        // Generated grayscale default resource samples128 or192,
                        // with linear boundary interpolation. Native ZERO/SRCCOLOR
                        // multiplies the accepted terrain rather than alpha-over.
                        for (unsigned channel=0;channel<3;++channel)
                            require(accepted[pixel+channel]+2>=empty[pixel+channel]*128U/255U
                                && accepted[pixel+channel]<=empty[pixel+channel]*192U/255U+2,
                                "physical projected decal is not source multiplicative");
                        ++darkened;
                    }
                    require(darkened>0,"physical projected tree decal did not sample terrain pixels");
                    const auto* current=terrain->peekTreeVertexSource();
                    require(current==treeVertex && std::memcmp(current->Get_CPU_Vertex_Buffer(),treeGeometry.data(),treeGeometry.size())==0,
                        "physical optional projected decal altered accepted tree geometry");
                }
                require(terrain->prepareTreeRenderPhase(camera)==BaseHeightMapRenderObjClass::TREE_PHASE_READY,
                    "physical equivalent paused phase rejected");
                display.draw();
                require(device.readback_rgba(color)==accepted,"physical identical paused source draw changed pixels");
                visual.reset();edge.release_source_buffers();device.destroy(depth);device.destroy(color);
            } catch (...) {
                TheTacticalView=savedView;TheTerrainVisual=savedVisual;TheDisplay=savedDisplay;throw;
            }
            TheTacticalView=savedView;TheTerrainVisual=savedVisual;TheDisplay=savedDisplay;
        }
        device.release_window();
        require(device.wait_idle() && device.live_resource_count()==0,"physical tree draw teardown retained resources");
    }
    TheGameLogic->setGamePaused(paused,FALSE);
    TheWritableGlobalData->m_useShadowDecals=priorShadows;
    SDL_DestroyWindow(window);SDL_Quit();
    std::puts("original tree draw physical: source=1 rollback=1 retry=1 generations=2 resources=0");
    if (projected) std::puts("original tree decal physical: source=1 multiplicative=1 rollback=1 retry=1 generations=2 resources=0");
}
}

static void recordingDecals(const char* path,const char* asset)
{
    using namespace zh::original_runtime::detail;
    const auto shadows=TheWritableGlobalData->m_useShadowDecals,trees=TheWritableGlobalData->m_useTrees;
    const auto partition=TheWritableGlobalData->m_partitionCellSize;
    TheWritableGlobalData->m_partitionCellSize=MAP_XY_FACTOR;
    const auto paused=TheGameLogic->isGamePaused();
    TheWritableGlobalData->m_useShadowDecals=FALSE;TheWritableGlobalData->m_useTrees=TRUE;
    TheGameLogic->setGamePaused(TRUE,FALSE);
    for (unsigned generation=0;generation<2;++generation) {
        TheWritableGlobalData->m_useShadowDecals=FALSE;
        zh::renderer::RecordingGpuDevice device;
        {
            zh::original_runtime::OriginalGpuEdge edge(device);
            W3DDisplay display;display.init();
            auto* priorDisplay=TheDisplay;auto* priorVisual=TheTerrainVisual;auto* priorView=TheTacticalView;
            TheDisplay=&display;W3DTerrainVisual visual;TheTerrainVisual=&visual;
            try {
                display.setWidth(32);display.setHeight(24);visual.init();
                require(visual.load(AsciiString(path)),"tree decal generated map rejected");
                TheWritableGlobalData->m_useShadowDecals=TRUE;
                std::ifstream input(asset,std::ios::binary);
                std::vector<char> bytes(std::istreambuf_iterator<char>{input},std::istreambuf_iterator<char>{});
                RAMFileClass packet(bytes.data(),static_cast<int>(bytes.size()));
                require(!bytes.empty() && static_cast<WW3DAssetManager*>(W3DDisplay::m_assetManager)->Load_3D_Assets(packet),
                    "tree decal generated model rejected");
                auto* view=new W3DView;TheTacticalView=view;view->init();display.attachView(view);
                view->setWidth(32);view->setHeight(24);view->setDefaultView(0,0,1);
                auto* camera=view->get3DCamera();camera->Set_Clip_Planes(0.1f,1000);camera->Set_Position(Vector3(12,18,30));
                zh::renderer::TextureDesc target;target.width=32;target.height=24;target.render_target=true;
                target.format=zh::renderer::TextureFormat::rgba8;const auto color=device.create_texture(target,"tree decal color");
                target.format=zh::renderer::TextureFormat::depth24_stencil8;const auto depth=device.create_texture(target,"tree decal depth");
                require(color && depth,"tree decal target rejected");edge.bind_frame_targets(color,depth,32,24);
                auto* terrain=TheTerrainRenderObject;terrain->getShroud()->fillShroudData(255);terrain->getShroud()->render(camera);
                W3DTreeDrawModuleData data;data.m_modelName="TEST.LITONE01";data.m_textureName="Tree0.tga";data.m_doShadow=TRUE;
                const Coord3D position{12,18,2};
                require(terrain->tryAddTree(static_cast<DrawableID>(901),position,8,0,0,&data),"tree decal source admission rejected");
                const auto* vertex=terrain->peekTreeVertexSource();const auto resources=device.resource_counts();
                const auto commands=device.snapshot();UnsignedInt rng[6],after[6];CopyGameClientRandomState(rng);
                auto* files=TheFileSystem;TheFileSystem=nullptr;
                const auto missingFiles=terrain->prepareTreeRenderPhase(camera);TheFileSystem=files;
                CopyGameClientRandomState(after);
                require(missingFiles==BaseHeightMapRenderObjClass::TREE_PHASE_REJECTED
                    && !terrain->preparedTreePhaseIdentity() && terrain->peekTreeVertexSource()==vertex
                    && device.resource_counts()==resources && std::memcmp(rng,after,sizeof(rng))==0,
                    "tree decal missing file provider published candidate state");
                for (const char* fault:{"queue","texture-create","texture-upload","texture-publish","vertex","index","program","publish"}) {
                    setenv("ZH_M22_TREE_DECAL_FAIL_AT",fault,1);
                    const auto result=terrain->prepareTreeRenderPhase(camera);unsetenv("ZH_M22_TREE_DECAL_FAIL_AT");
                    CopyGameClientRandomState(after);
                    require(result==BaseHeightMapRenderObjClass::TREE_PHASE_REJECTED && !terrain->preparedTreePhaseIdentity()
                        && terrain->peekTreeVertexSource()==vertex && device.resource_counts()==resources
                        && std::memcmp(rng,after,sizeof(rng))==0 && terrain->treeToppleStartEvents(static_cast<DrawableID>(901))==0
                        && !edge.tree_source_frame_pending() && !TreeDecalGeneratedProbeAccess::peek(terrain),
                        "tree decal candidate failure published C1/RNG/FX/resources");
                }
                if (!std::getenv("ZH_M22_TREE_DECAL_PROVIDER_NEGATIVE")) {
                require(terrain->prepareTreeRenderPhase(camera)==BaseHeightMapRenderObjClass::TREE_PHASE_READY,
                    "tree decal source preparation rejected");
                const auto identity=terrain->preparedTreePhaseIdentity();const auto* queue=TreeDecalGeneratedProbeAccess::peek(terrain);
                require(identity && queue && queue->intents.size()==1 && queue->batches.size()==1
                    && queue->intents[0].id==static_cast<DrawableID>(901) && queue->intents[0].size==1
                    && queue->intents[0].position.x==position.x && queue->intents[0].position.y==position.y
                    && TreeDecalGeneratedProbeAccess::objectless(terrain),"tree decal pre-topple/objectless source intent differs");
                const auto* texture=TreeDecalGeneratedProbeAccess::texture(terrain);
                require(texture && edge.resident_texture(texture) && texture->Num_Refs()==2,
                    "tree decal source/phase texture ownership multiplicity differs");
                auto* map=terrain->getMap();const auto& rectangle=queue->rectangle;
                require(rectangle.startX==map->getDrawOrgX() && rectangle.startY==map->getDrawOrgY()
                    && rectangle.endX==std::min(map->getDrawOrgX()+map->getDrawWidth()-1,map->getXExtent()-1)
                    && rectangle.endY==std::min(map->getDrawOrgY()+map->getDrawHeight()-1,map->getYExtent()-1),
                    "tree decal unlisted-only rectangle did not use current map");
                const auto& batch=queue->batches[0];
                require(batch.vertices.size()==4 && batch.indices.size()==6,"tree decal source rectangle geometry differs");
                for (const auto& v:batch.vertices) {
                    const Int x=static_cast<Int>(v.x/MAP_XY_FACTOR)+map->getBorderSizeInline();
                    const Int y=static_cast<Int>(v.y/MAP_XY_FACTOR)+map->getBorderSizeInline();
                    require(v.z==map->getHeight(x,y)*MAP_HEIGHT_SCALE+0.01f*MAP_XY_FACTOR
                        && v.diffuse==0xffffffffU && v.u1==(v.x-position.x)+0.5f && v.v1==(v.y-position.y)+0.5f,
                        "tree decal native height/negative-Y/UV source formula differs");
                }
                const auto bytesImage=[&] {
                    std::vector<unsigned char> result;
                    for (const auto& b:queue->batches) {
                        const auto* v=reinterpret_cast<const unsigned char*>(b.vertices.data());
                        result.insert(result.end(),v,v+b.vertices.size()*sizeof(b.vertices[0]));
                        const auto* i=reinterpret_cast<const unsigned char*>(b.indices.data());
                        result.insert(result.end(),i,i+b.indices.size()*sizeof(b.indices[0]));
                    } return result;
                };
                const auto immutable=bytesImage();const auto pins=device.resource_counts();CopyGameClientRandomState(rng);
                for (unsigned fault=0;fault<2;++fault) {
                    const auto baseline=device.snapshot();const auto owners=W3DFrameGeneratedProbeAccess::image(camera);
                    if (!fault) W3DFrameGeneratedProbeAccess::failPresent(edge);else W3DFrameGeneratedProbeAccess::failCommit(edge);
                    require(rejected([&]{display.draw();}),"tree decal late transaction fault admitted");CopyGameClientRandomState(after);
                    require(terrain->preparedTreePhaseIdentity()==identity && TreeDecalGeneratedProbeAccess::peek(terrain)==queue
                        && TreeDecalGeneratedProbeAccess::texture(terrain)==texture && bytesImage()==immutable
                        && device.resource_counts()==pins && device.snapshot()==baseline
                        && W3DFrameGeneratedProbeAccess::image(camera)==owners && std::memcmp(rng,after,sizeof(rng))==0,
                        "tree decal late failure changed accepted queue/resource/frame/C1/RNG");
                }
                const auto beforeTrace=device.snapshot();display.draw();
                const auto trace=device.snapshot().substr(beforeTrace.size());
                const auto pre=trace.find("original tree source opaque/empty-occluded/shader flush complete");
                const auto decal=trace.find("W3DProjectedShadowManager::flushDecals multiplicative tree triangles");
                const auto tree=trace.find("BaseHeightMap::renderTrees immutable indexed triangles");
                require(pre!=std::string::npos && decal>pre && tree>decal && tree!=std::string::npos
                    && !terrain->preparedTreePhaseIdentity() && texture->Num_Refs()==1,
                    "tree decal multiplicative source order/completion ownership differs");
                TheWritableGlobalData->m_useShadowDecals=FALSE;
                require(terrain->prepareTreeRenderPhase(camera)==BaseHeightMapRenderObjClass::TREE_PHASE_READY
                    && TreeDecalGeneratedProbeAccess::peek(terrain)->intents.empty()
                    && !TreeDecalGeneratedProbeAccess::texture(terrain),"shadow-disabled phase created optional owner");
                terrain->cancelTreeRenderPhase();TheWritableGlobalData->m_useShadowDecals=TRUE;
                data.m_doShadow=FALSE;
                require(terrain->prepareTreeRenderPhase(camera)==BaseHeightMapRenderObjClass::TREE_PHASE_READY
                    && TreeDecalGeneratedProbeAccess::peek(terrain)->intents.empty(),"DoShadow=false queued a projected decal");
                terrain->cancelTreeRenderPhase();data.m_doShadow=TRUE;
                const auto cameraPosition=camera->Get_Position();camera->Set_Position(Vector3(10000,10000,30));
                require(terrain->prepareTreeRenderPhase(camera)==BaseHeightMapRenderObjClass::TREE_PHASE_READY
                    && !terrain->treeIsVisible(static_cast<DrawableID>(901))
                    && TreeDecalGeneratedProbeAccess::peek(terrain)->intents.empty(),"hidden tree queued a projected decal");
                terrain->cancelTreeRenderPhase();camera->Set_Position(cameraPosition);
                TreeDecalQueue direct;const std::vector<TreeDecalIntent> one{{static_cast<DrawableID>(901),position,20}};
                require(build_tree_decal_queue(*map,one,direct),"tree decal direct source algebra control rejected");
                auto tooMany=one;tooMany.resize(4001,one[0]);const auto prior=direct.batches[0].vertices.size();
                require(!build_tree_decal_queue(*map,tooMany,direct) && direct.batches[0].vertices.size()==prior,
                    "tree decal bound+1 mutated accepted queue");
                tooMany.resize(4000);
                require(build_tree_decal_queue(*map,tooMany,direct) && direct.intents.size()==4000
                    && direct.batches.size()>1,"tree decal exact intent capacity/stream split rejected");
                for (const auto& b:direct.batches) {
                    require(b.vertices.size()<=32768 && b.indices.size()<=65535,
                        "tree decal streaming batch narrowed native count");
                    for (auto i:b.indices) require(i<b.vertices.size(),"tree decal split index crossed batch");
                }
                CachedFileInputStream largeInput;
                require(largeInput.open(AsciiString(std::getenv("ZH_M22_TREE_DECAL_LARGE_MAP"))),
                    "tree decal large grid fixture missing");
                std::shared_ptr<GeneratedDecalMap> large(NEW_REF(GeneratedDecalMap,(&largeInput)),
                    [](GeneratedDecalMap* value){value->Release_Ref();});largeInput.close();
                large->setDrawWidth(112);large->setDrawHeight(112);
                const std::vector<TreeDecalIntent> broad{{static_cast<DrawableID>(901),{530,530,2},2000}};
                TreeDecalQueue clipped;
                require(build_tree_decal_queue(*large,broad,clipped) && clipped.batches.size()==1
                    && clipped.batches[0].vertices.size()==104*104 && clipped.batches[0].indices.size()==103*103*6
                    && clipped.batches[0].vertices.front().x==20 && clipped.batches[0].vertices.front().y==20,
                    "tree decal 104-axis clip/border source rule differs");
                const std::size_t patchBytes=clipped.batches[0].vertices.size()*sizeof(VertexFormatXYZDUV1)
                    +clipped.batches[0].indices.size()*sizeof(UnsignedShort);
                const Int sx=4,sy=4,row=104;
                for (const bool flip:{false,true}) {
                large->flip(sx,sy,flip);
                require(build_tree_decal_queue(*large,broad,clipped),"tree decal winding control rejected");
                const std::array<UnsignedShort,6> expected=flip
                    ?std::array<UnsignedShort,6>{1,row,0,1,row+1,row}
                    :std::array<UnsignedShort,6>{0,row+1,row,0,1,row+1};
                require(std::equal(expected.begin(),expected.end(),clipped.batches[0].indices.begin()),
                    "tree decal authored flip winding changed");
                }
                auto byteLimit=broad;byteLimit.resize((64U*1024U*1024U)/patchBytes,broad[0]);
                require(build_tree_decal_queue(*large,byteLimit,clipped),"tree decal exact byte-bound admission rejected");
                const auto acceptedBatches=clipped.batches.size();byteLimit.push_back(broad[0]);
                require(!build_tree_decal_queue(*large,byteLimit,clipped) && clipped.batches.size()==acceptedBatches,
                    "tree decal byte bound+1 changed accepted queue");
                // Alternating patches force distinct batches below the byte bound:
                // 63654 + 2400 indices cannot share the native ushort-count batch.
                std::vector<TreeDecalIntent> alternating;
                for (unsigned i=0;i<256;++i)
                    alternating.push_back({static_cast<DrawableID>(901+i),{530,530,2},i%2?200.0f:2000.0f});
                const auto capacityCommands=device.snapshot();const auto capacityResources=device.resource_counts();
                const auto capacityOwners=W3DFrameGeneratedProbeAccess::image(camera);
                UnsignedInt capacityRng[6],capacityAfter[6];CopyGameClientRandomState(capacityRng);
                require(build_tree_decal_queue(*large,alternating,clipped) && clipped.batches.size()==256
                    && clipped.intents.size()==256,"tree decal exact batch capacity rejected");
                const TreeDecalBatch largePatch=clipped.batches[0],smallPatch=clipped.batches[1];
                require(largePatch.vertices.size()==104*104 && largePatch.indices.size()==103*103*6
                    && smallPatch.vertices.size()==21*21 && smallPatch.indices.size()==20*20*6,
                    "tree decal alternating source grid differs");
                struct BatchIdentity {
                    const VertexFormatXYZDUV1* vertices;const UnsignedShort* indices;
                    std::size_t vertexCapacity,indexCapacity;
                };
                std::array<BatchIdentity,256> batchIdentities{};
                std::size_t alternatingBytes=0;
                for (std::size_t i=0;i<clipped.batches.size();++i) {
                    const auto& batch=clipped.batches[i];
                    batchIdentities[i]={batch.vertices.data(),batch.indices.data(),
                        batch.vertices.capacity(),batch.indices.capacity()};
                    alternatingBytes+=batch.vertices.size()*sizeof(VertexFormatXYZDUV1)
                        +batch.indices.size()*sizeof(UnsignedShort);
                }
                require(alternatingBytes==51491328 && alternatingBytes<64U*1024U*1024U,
                    "tree decal batch capacity control reached byte bound instead");
                const auto* batchStorage=clipped.batches.data();const auto* intentStorage=clipped.intents.data();
                const auto batchCapacity=clipped.batches.capacity(),intentCapacity=clipped.intents.capacity();
                const auto capacityRectangle=clipped.rectangle;
                alternating.push_back({static_cast<DrawableID>(1157),{530,530,2},2000});
                require(!build_tree_decal_queue(*large,alternating,clipped)
                    && clipped.batches.size()==256 && clipped.intents.size()==256
                    && clipped.batches.data()==batchStorage && clipped.intents.data()==intentStorage
                    && clipped.batches.capacity()==batchCapacity && clipped.intents.capacity()==intentCapacity
                    && clipped.rectangle.startX==capacityRectangle.startX && clipped.rectangle.startY==capacityRectangle.startY
                    && clipped.rectangle.endX==capacityRectangle.endX && clipped.rectangle.endY==capacityRectangle.endY,
                    "tree decal batch bound+1 mutated accepted queue storage/rectangle");
                for (std::size_t i=0;i<256;++i) {
                    const auto& batch=clipped.batches[i];const auto& expected=i%2?smallPatch:largePatch;
                    const auto& identity=batchIdentities[i];const auto& intent=clipped.intents[i];
                    require(batch.vertices.data()==identity.vertices && batch.indices.data()==identity.indices
                        && batch.vertices.capacity()==identity.vertexCapacity && batch.indices.capacity()==identity.indexCapacity
                        && batch.vertices.size()==expected.vertices.size() && batch.indices.size()==expected.indices.size()
                        && std::memcmp(batch.vertices.data(),expected.vertices.data(),batch.vertices.size()*sizeof(VertexFormatXYZDUV1))==0
                        && std::memcmp(batch.indices.data(),expected.indices.data(),batch.indices.size()*sizeof(UnsignedShort))==0
                        && intent.id==alternating[i].id && intent.size==alternating[i].size
                        && intent.position.x==530 && intent.position.y==530 && intent.position.z==2,
                        "tree decal batch bound+1 changed accepted identity/order/bytes");
                }
                CopyGameClientRandomState(capacityAfter);
                require(device.snapshot()==capacityCommands && device.resource_counts()==capacityResources
                    && W3DFrameGeneratedProbeAccess::image(camera)==capacityOwners
                    && std::memcmp(capacityRng,capacityAfter,sizeof(capacityRng))==0,
                    "tree decal batch admission changed source/RNG/GPU state");
                large->setDrawWidth(20);large->setDrawHeight(18);large->origin(8,9);
                require(build_tree_decal_queue(*large,broad,clipped) && clipped.rectangle.startX==8
                    && clipped.rectangle.startY==9 && clipped.rectangle.endX==27 && clipped.rectangle.endY==26
                    && clipped.batches[0].vertices.front().x==60 && clipped.batches[0].vertices.front().y==70,
                    "tree decal shifted current-map rectangle/border changed");
                auto invalid=one;invalid[0].size=0;
                require(!build_tree_decal_queue(*map,invalid,direct),"tree decal zero size admitted");
                invalid[0].size=std::numeric_limits<Real>::infinity();
                require(!build_tree_decal_queue(*map,invalid,direct),"tree decal nonfinite size admitted");
                require(terrain->prepareTreeRenderPhase(camera)==BaseHeightMapRenderObjClass::TREE_PHASE_READY,
                    "tree decal cancel/retry failed");
                terrain->removeTree(static_cast<DrawableID>(901));
                require(!terrain->preparedTreePhaseIdentity() && !TreeDecalGeneratedProbeAccess::peek(terrain),
                    "tree decal owner removal retained phase");
                Object* crusher=TheGameLogic->getFirstObject();while (crusher && crusher->getCrusherLevel()<=1) crusher=crusher->getNextObject();
                require(crusher && ThePlayerList && ThePartitionManager,"tree decal real topple providers missing");
                const auto crusherPosition=*crusher->getPosition();const auto crusherGeometry=crusher->getGeometryInfo();
                const Coord3D treePosition{13,18,2},unitPosition{12,18,2};
                const auto playerMask=ThePlayerList->getLocalPlayer()->getPlayerMask();
                PreparationShroudNotices notices;
                TheDisplay=&notices;
                try {
                    ThePartitionManager->doShroudReveal(treePosition.x,treePosition.y,1,playerMask);
                    const auto reveal=notices.notices.size();
                    ThePartitionManager->undoShroudReveal(treePosition.x,treePosition.y,1,playerMask);
                    require(reveal>0 && notices.notices.size()==2*reveal,"tree decal fog baseline notices incomplete");
                    for (std::size_t i=0;i<reveal;++i)
                        require(notices.notices[i].status==CELLSHROUD_CLEAR && notices.notices[i+reveal].status==CELLSHROUD_FOGGED
                            && notices.notices[i].x==notices.notices[i+reveal].x && notices.notices[i].y==notices.notices[i+reveal].y,
                            "tree decal fog baseline reveal/undo sequence differs");
                } catch (...) { TheDisplay=&display;throw; }
                TheDisplay=&display;
                require(ThePartitionManager->getPropShroudStatusForPlayer(ThePlayerList->getLocalPlayer()->getPlayerIndex(),&treePosition)
                    ==OBJECTSHROUD_FOGGED,"tree decal real fog baseline missing");
                data.m_doTopple=TRUE;data.m_killWhenToppled=FALSE;data.m_minimumToppleSpeed=2;
                data.m_initialVelocityPercent=1;data.m_initialAccelPercent=0;data.m_bounceVelocityPercent=0;
                data.m_toppleFX=NULL;data.m_bounceFX=NULL;
                require(terrain->tryAddTree(static_cast<DrawableID>(902),treePosition,8,0,0,&data),"tree decal topple source rejected");
                TheGameLogic->setGamePaused(FALSE,FALSE);
                crusher->setGeometryInfo(GeometryInfo(GEOMETRY_CYLINDER,FALSE,2,4,4));crusher->setPosition(&unitPosition);
                require(terrain->treeToppleState(static_cast<DrawableID>(902))==1,"tree decal public collision did not enter FALLING");
                require(terrain->prepareTreeRenderPhase(camera)==BaseHeightMapRenderObjClass::TREE_PHASE_READY
                    && terrain->treeToppleState(static_cast<DrawableID>(902))==2
                    && TreeDecalGeneratedProbeAccess::peek(terrain)->intents.empty(),
                    "FALLING tree was queued using post-advance FOGGED state");
                terrain->cancelTreeRenderPhase();
                TheDisplay=&notices;const auto firstNotice=notices.notices.size();
                try { ThePartitionManager->doShroudReveal(treePosition.x,treePosition.y,1,playerMask); }
                catch (...) { TheDisplay=&display;throw; }
                TheDisplay=&display;const auto revealCount=notices.notices.size()-firstNotice;
                CopyGameClientRandomState(rng);const auto foggedResources=device.resource_counts();
                const auto foggedVertex=terrain->peekTreeVertexSource();
                setenv("ZH_M22_TREE_DECAL_FAIL_AT","publish",1);
                const auto failed=terrain->prepareTreeRenderPhase(camera);unsetenv("ZH_M22_TREE_DECAL_FAIL_AT");CopyGameClientRandomState(after);
                require(failed==BaseHeightMapRenderObjClass::TREE_PHASE_REJECTED
                    && terrain->treeToppleState(static_cast<DrawableID>(902))==2 && !terrain->preparedTreePhaseIdentity()
                    && terrain->peekTreeVertexSource()==foggedVertex && device.resource_counts()==foggedResources
                    && std::memcmp(rng,after,sizeof(rng))==0,"FOGGED decal publication failure advanced C1/RNG");
                require(terrain->prepareTreeRenderPhase(camera)==BaseHeightMapRenderObjClass::TREE_PHASE_READY
                    && terrain->treeToppleState(static_cast<DrawableID>(902))==4
                    && TreeDecalGeneratedProbeAccess::peek(terrain)->intents.size()==1,
                    "FOGGED source decal was derived from post-advance DOWN state");
                const auto foggedPhase=terrain->preparedTreePhaseIdentity();const auto* foggedQueue=TreeDecalGeneratedProbeAccess::peek(terrain);
                const auto foggedBytes=foggedQueue->batches[0].vertices;
                require(terrain->prepareTreeRenderPhase(camera)==BaseHeightMapRenderObjClass::TREE_PHASE_READY
                    && terrain->preparedTreePhaseIdentity()==foggedPhase && TreeDecalGeneratedProbeAccess::peek(terrain)==foggedQueue
                    && std::memcmp(foggedQueue->batches[0].vertices.data(),foggedBytes.data(),foggedBytes.size()*sizeof(foggedBytes[0]))==0,
                    "later topple state altered immutable pre-advance decal batch");
                display.draw();
                require(terrain->prepareTreeRenderPhase(camera)==BaseHeightMapRenderObjClass::TREE_PHASE_READY
                    && TreeDecalGeneratedProbeAccess::peek(terrain)->intents.empty(),"DOWN tree queued a projected decal");
                terrain->cancelTreeRenderPhase();terrain->removeTree(static_cast<DrawableID>(902));
                crusher->setPosition(&crusherPosition);crusher->setGeometryInfo(crusherGeometry);
                TheDisplay=&notices;
                try { ThePartitionManager->undoShroudReveal(treePosition.x,treePosition.y,1,playerMask); }
                catch (...) { TheDisplay=&display;throw; }
                TheDisplay=&display;
                require(revealCount>0 && notices.notices.size()==firstNotice+2*revealCount,
                    "tree decal real reveal/undo did not restore fog ownership");
                for (std::size_t i=0;i<revealCount;++i)
                    require(notices.notices[firstNotice+i].status==CELLSHROUD_CLEAR
                        && notices.notices[firstNotice+revealCount+i].status==CELLSHROUD_FOGGED
                        && notices.notices[firstNotice+i].x==notices.notices[firstNotice+revealCount+i].x
                        && notices.notices[firstNotice+i].y==notices.notices[firstNotice+revealCount+i].y,
                        "tree decal final reveal/undo sequence differs");
                TheGameLogic->setGamePaused(TRUE,FALSE);
                // A different tree's immediate FX removes an upright decal
                // owner after phase publication. Both nuggets must execute,
                // and cancellation may use only the already-owned phase.
                auto* fx=const_cast<FXList*>(TheFXListStore->findFXList("FixtureGraph60"));
                require(fx,"tree decal FX-removal provider missing");Int dispatches=0;
                fx->clear();fx->addFXNugget(newInstance(PreparedPhaseFX)(&dispatches,terrain,NULL));
                fx->addFXNugget(newInstance(PreparedPhaseFX)(&dispatches,NULL,NULL));
                data.m_doTopple=FALSE;data.m_toppleFX=NULL;
                require(terrain->tryAddTree(static_cast<DrawableID>(903),treePosition,1,0,0,&data),
                    "tree decal FX survivor admission rejected");
                data.m_doTopple=TRUE;data.m_toppleFX=fx;
                require(terrain->tryAddTree(static_cast<DrawableID>(904),treePosition,1,0,0,&data),
                    "tree decal FX trigger admission rejected");
                crusher->setGeometryInfo(GeometryInfo(GEOMETRY_CYLINDER,FALSE,2,4,4));crusher->setPosition(&unitPosition);
                require(terrain->treeToppleStartEvents(static_cast<DrawableID>(904))==1,
                    "tree decal public FX collision did not stage intent");
                TheDisplay=&notices;const auto fxFirst=notices.notices.size();
                try { ThePartitionManager->doShroudReveal(treePosition.x,treePosition.y,1,playerMask); }
                catch (...) { TheDisplay=&display;throw; }
                TheDisplay=&display;const auto fxReveals=notices.notices.size()-fxFirst;
                device.fail_next_transaction_checkpoint();CopyGameClientRandomState(rng);
                require(rejected([&]{display.draw();}) && dispatches==0
                    && !terrain->preparedTreePhaseIdentity() && terrain->treeInstanceCount()==2,
                    "tree decal rejected probe published phase or dispatched FX");CopyGameClientRandomState(after);
                require(std::memcmp(rng,after,sizeof(rng))==0,"tree decal rejected FX probe advanced RNG");
                require(rejected([&]{display.draw();}) && dispatches==2
                    && !terrain->preparedTreePhaseIdentity() && terrain->treeInstanceCount()==0,
                    "tree decal FX owner removal failed exact phase cancellation/later nugget");
                require(terrain->prepareTreeRenderPhase(camera)==BaseHeightMapRenderObjClass::TREE_PHASE_EMPTY
                    && dispatches==2,"tree decal FX retry replayed consumed dispatch");
                fx->clear();crusher->setPosition(&crusherPosition);crusher->setGeometryInfo(crusherGeometry);
                TheDisplay=&notices;
                try { ThePartitionManager->undoShroudReveal(treePosition.x,treePosition.y,1,playerMask); }
                catch (...) { TheDisplay=&display;throw; }
                TheDisplay=&display;
                require(fxReveals>0 && notices.notices.size()==fxFirst+2*fxReveals,
                    "tree decal FX reveal/undo count changed");
                for (std::size_t i=0;i<fxReveals;++i)
                    require(notices.notices[fxFirst+i].status==CELLSHROUD_CLEAR
                        && notices.notices[fxFirst+fxReveals+i].status==CELLSHROUD_FOGGED
                        && notices.notices[fxFirst+i].x==notices.notices[fxFirst+fxReveals+i].x
                        && notices.notices[fxFirst+i].y==notices.notices[fxFirst+fxReveals+i].y,
                        "tree decal FX reveal/undo identity changed");
                } else {
                    CopyGameClientRandomState(rng);
                    require(terrain->prepareTreeRenderPhase(camera)==BaseHeightMapRenderObjClass::TREE_PHASE_REJECTED
                        && !terrain->preparedTreePhaseIdentity() && device.resource_counts()==resources
                        && terrain->peekTreeVertexSource()==vertex,"tree decal wrong file provider accepted");
                    CopyGameClientRandomState(after);require(std::memcmp(rng,after,sizeof(rng))==0,
                        "tree decal wrong provider consumed RNG");
                    terrain->removeTree(static_cast<DrawableID>(901));
                }
                visual.reset();edge.release_source_buffers();device.destroy(depth);device.destroy(color);
            } catch (...) { TheDisplay=priorDisplay;TheTerrainVisual=priorVisual;TheTacticalView=priorView;throw; }
            TheDisplay=priorDisplay;TheTerrainVisual=priorVisual;TheTacticalView=priorView;
        }
        require(device.resource_counts().total()==0,"tree decal generation retained native resources");
    }
    TheGameLogic->setGamePaused(paused,FALSE);TheWritableGlobalData->m_useShadowDecals=shadows;TheWritableGlobalData->m_useTrees=trees;
    TheWritableGlobalData->m_partitionCellSize=partition;
    std::puts("original tree decal: source=1 rollback=1 retry=1 generations=2 resources=0");
}

extern "C" void zh_probe_terrain_tree_preparation()
{
    const char* path=std::getenv("ZH_M22_TREE_PREPARATION_MAP");
    const char* asset=std::getenv("ZH_M22_TREE_PREPARATION_ASSET");
    require(path && asset,"tree preparation generated providers missing");
    if (std::getenv("ZH_M22_TREE_DECAL_PROFILE")) {
        recordingDecals(path,asset);
        if (std::getenv("ZH_M22_TREE_DECAL_PROVIDER_NEGATIVE")) return;
        {
            const auto partition=TheWritableGlobalData->m_partitionCellSize;
            const auto trees=TheWritableGlobalData->m_useTrees;
            const auto pause=TheGameLogic->isGamePaused();
            TheWritableGlobalData->m_partitionCellSize=MAP_XY_FACTOR;TheWritableGlobalData->m_useTrees=TRUE;
            TheGameLogic->setGamePaused(TRUE,FALSE);
            recordingBoundaries(path,asset,true);
            TheWritableGlobalData->m_partitionCellSize=partition;TheWritableGlobalData->m_useTrees=trees;
            TheGameLogic->setGamePaused(pause,FALSE);
        }
        if (std::getenv("ZH_M22_TREE_DECAL_PHYSICAL")) {
            const auto partition=TheWritableGlobalData->m_partitionCellSize;
            const auto trees=TheWritableGlobalData->m_useTrees;
            TheWritableGlobalData->m_partitionCellSize=MAP_XY_FACTOR;TheWritableGlobalData->m_useTrees=TRUE;
            physicalDraw(path,asset);
            TheWritableGlobalData->m_partitionCellSize=partition;TheWritableGlobalData->m_useTrees=trees;
        }
        return;
    }
    const auto savedPartition=TheWritableGlobalData->m_partitionCellSize;
    const auto savedTrees=TheWritableGlobalData->m_useTrees;
    TheWritableGlobalData->m_partitionCellSize=MAP_XY_FACTOR;
    TheWritableGlobalData->m_useTrees=TRUE;
    zh::renderer::RecordingGpuDevice device;
    for (unsigned generation=0;generation<2;++generation) {
        {
        zh::original_runtime::OriginalGpuEdge edge(device);
        {
            W3DDisplay display;display.init();
            auto* savedDisplay=TheDisplay;TheDisplay=&display;
            auto* savedVisual=TheTerrainVisual;
            auto* savedView=TheTacticalView;
            W3DTerrainVisual visual;TheTerrainVisual=&visual;
            try {
                display.setWidth(32);display.setHeight(24);
                visual.init();require(visual.load(AsciiString(path)),"tree preparation map load rejected");
                std::ifstream input(asset,std::ios::binary);
                std::vector<char> bytes(std::istreambuf_iterator<char>{input},std::istreambuf_iterator<char>{});
                require(!bytes.empty(),"tree preparation model packet missing");
                RAMFileClass packet(bytes.data(),static_cast<int>(bytes.size()));
                require(static_cast<WW3DAssetManager*>(W3DDisplay::m_assetManager)->Load_3D_Assets(packet),
                    "tree preparation source model load rejected");
                auto* view=new W3DView;TheTacticalView=view;view->init();display.attachView(view);
                view->setWidth(32);view->setHeight(24);view->setDefaultView(0,0,1);
                auto* camera=view->get3DCamera();
                camera->Set_Clip_Planes(0.1f,1000.0f);
                camera->Set_Position(Vector3(12,18,30));
                zh::renderer::TextureDesc target;
                target.width=32;target.height=24;target.render_target=true;target.sampled=false;
                target.format=zh::renderer::TextureFormat::rgba8;
                auto color=device.create_texture(target,"tree preparation color");
                target.format=zh::renderer::TextureFormat::depth24_stencil8;
                auto depth=device.create_texture(target,"tree preparation depth");
                require(color && depth,"tree preparation targets missing");
                edge.bind_frame_targets(color,depth,32,24);
                auto* terrain=TheTerrainRenderObject;
                const auto displayFrame=[&] {
                    const auto presents=device.operation_counts().presents;
                    display.draw();
                    if (device.operation_counts().presents==presents)
                        require(device.present(color).valid,"tree preparation ordinary presentation rejected");
                };
                const auto drawFrame=[&] {
                    // C2D completes successful display frames. Keep C2C's
                    // retained-phase preparation/retry/cancel controls explicit.
                    if (!TheGlobalData->m_useTrees) { terrain->cancelTreeRenderPhase();return; }
                    const auto result=terrain->prepareTreeRenderPhase(camera);
                    if (result==BaseHeightMapRenderObjClass::TREE_PHASE_REJECTED ||
                        result==BaseHeightMapRenderObjClass::TREE_PHASE_CANCELED)
                        throw std::runtime_error("tree preparation source owner rejected");
                };
                terrain->getShroud()->render(camera);
                W3DFrameGeneratedProbeAccess::roundTrip(camera);
                W3DTreeDrawModuleData data;
                data.m_modelName="TEST.LITONE01";data.m_textureName="Tree0.tga";
                const Coord3D position{12,18,2};
                require(terrain->tryAddTree(static_cast<DrawableID>(501),position,1,0,0,&data),
                    "tree preparation source admission rejected");
                const auto before=device.resource_counts();
                const auto* vertex=terrain->peekTreeVertexSource();
                UnsignedInt rng[6],after[6];CopyGameClientRandomState(rng);
                for (unsigned fault=0;fault<8;++fault) {
                    if (fault==0) device.fail_next_shader_create();
                    if (fault==1) device.fail_next_pipeline_create();
                    if (fault==2) device.fail_next_buffer_create();
                    if (fault==3) device.fail_buffer_create_after(3);
                    if (fault==4) device.fail_buffer_upload_after(2);
                    if (fault==5) device.fail_buffer_upload_after(3);
                    if (fault==6) device.fail_next_transaction_checkpoint();
                    if (fault==7) setenv("ZH_M22_TREE_FRAME_FAIL_AT","publish",1);
                    const bool failed=rejected(displayFrame);
                    if (fault==7) unsetenv("ZH_M22_TREE_FRAME_FAIL_AT");
                    require(failed,"tree preparation fault accepted source draw");
                    CopyGameClientRandomState(after);
                    require(!terrain->preparedTreePhaseIdentity() && terrain->peekTreeVertexSource()==vertex
                        && device.resource_counts()==before && !device.pass_active()
                        && std::memcmp(rng,after,sizeof(rng))==0,
                        "tree preparation fault advanced accepted geometry/RNG/resources");
                }
                drawFrame();
                const auto identity=terrain->preparedTreePhaseIdentity();
                require(identity && terrain->treeVisibleCount()==1,"source preparation did not prepare exact visible phase");
                CopyGameClientRandomState(rng);
                const auto acceptedResources=device.resource_counts();
                const auto* acceptedVertex=terrain->peekTreeVertexSource();
                const auto sway=terrain->treeSwayVector(0);
                drawFrame();CopyGameClientRandomState(after);
                require(terrain->preparedTreePhaseIdentity()==identity
                    && terrain->peekTreeVertexSource()==acceptedVertex
                    && terrain->treeSwayVector(0)==sway && device.resource_counts()==acceptedResources
                    && std::memcmp(rng,after,sizeof(rng))==0,
                    "same-phase preparation retry advanced C1/RNG/resources");
                require(!terrain->completeTreeRenderPhase(identity+1)
                    && terrain->preparedTreePhaseIdentity()==identity,"foreign completion consumed phase");
                require(terrain->completeTreeRenderPhase(identity)
                    && !terrain->completeTreeRenderPhase(identity),"phase completion not exactly once");
                const auto completed=device.resource_counts();
                require(completed.buffers+2==acceptedResources.buffers
                    && completed.samplers+2==acceptedResources.samplers
                    && completed.shaders+2==acceptedResources.shaders
                    && completed.pipelines+1==acceptedResources.pipelines
                    && completed.textures==acceptedResources.textures,
                    "completion did not retire exactly seven phase-owned units");
                std::string lastCompletedTrace;
                W3DParticleSystemManager frameParticles;
                auto *priorParticles=TheParticleSystemManager;TheParticleSystemManager=&frameParticles;
                try {
                for (unsigned lateFault=0;lateFault<2;++lateFault) {
                    auto *assets=static_cast<WW3DAssetManager *>(W3DDisplay::m_assetManager);
                    auto *resident=assets->Get_Texture(lateFault?"M22ExpiryResident1":"M22ExpiryResident0",MIP_LEVELS_1,
                        WW3D_FORMAT_A8R8G8B8,false);
                    auto *nonresident=assets->Get_Texture(lateFault?"M22ExpiryNonresident1":"M22ExpiryNonresident0",MIP_LEVELS_1,
                        WW3D_FORMAT_A8R8G8B8,false);
                    require(resident && nonresident,"expiry generated texture owners missing");
                    resident->Apply_Gpu_Texture(WW3D_FORMAT_A8R8G8B8,1,1);
                    nonresident->Apply_Gpu_Texture(WW3D_FORMAT_A8R8G8B8,1,1);
                    unsigned mips=1;const auto residentHandle=edge.create_texture(WW3D_FORMAT_A8R8G8B8,1,1,mips);
                    const unsigned pixel=0xff345678;edge.upload_texture(residentHandle,0,1,1,4,&pixel,4);
                    edge.publish_texture(resident,residentHandle);
                    resident->Set_Inactivation_Time(1);nonresident->Set_Inactivation_Time(1);
                    require(edge.resident_texture(resident) && !edge.resident_texture(nonresident),
                        "expiry control did not distinguish resident/nonresident identities");
                    const auto residentMetadata=W3DFrameGeneratedProbeAccess::textureMetadata(*resident);
                    const auto nonresidentMetadata=W3DFrameGeneratedProbeAccess::textureMetadata(*nonresident);
                    const auto residentPixels=device.texture_bytes(residentHandle);
                    const auto thumbnails=WW3D::Get_Thumbnail_Enabled();WW3D::Set_Thumbnail_Enabled(true);
                    WW3D::Sync(WW3D::Get_Sync_Time()+100);
                    auto *smudges=static_cast<W3DSmudgeManager *>(TheSmudgeManager);
                    auto *set=smudges->addSmudgeSet();auto *smudge=set->addSmudgeToSet();
                    smudge->m_pos=Vector3(16,16,0);smudge->m_offset=Vector2(0,0);smudge->m_size=4;smudge->m_opacity=0.5f;
                    const Vector3 points[5]={Vector3(14,18,0),Vector3(14,14,0),Vector3(18,14,0),Vector3(18,18,0),Vector3(16,16,0)};
                    for (Int i=0;i<5;++i) {
                        smudge->m_verts[i].pos=points[i];
                        smudge->m_verts[i].uv.Set((i==2||i==3)?1:0,(i==0||i==3)?0:1);
                    }
                    frameParticles.queueParticleRender();
                    drawFrame();
                    const auto retained=terrain->preparedTreePhaseIdentity();
                    if (!lateFault) {
                        const auto capacityCommands=device.snapshot();const auto capacityResources=device.resource_counts();
                        const auto capacityOwners=W3DFrameGeneratedProbeAccess::image(camera);CopyGameClientRandomState(rng);
                        const auto capacity=W3DFrameGeneratedProbeAccess::extraTextureCapacity(edge);
                        std::vector<TextureClass *> extra;extra.reserve(capacity+1);
                        for (unsigned entry=0;entry<=capacity;++entry) {
                            char name[64];std::snprintf(name,sizeof(name),"M22FrameCapacity%u",entry);
                            extra.push_back(assets->Get_Texture(name,MIP_LEVELS_1,WW3D_FORMAT_A8R8G8B8,false));
                        }
                        require(rejected(displayFrame),"asset-hash bound plus one admitted a tree frame");
                        CopyGameClientRandomState(after);
                        require(terrain->preparedTreePhaseIdentity()==retained && !edge.tree_source_frame_pending()
                            && device.snapshot()==capacityCommands && device.resource_counts()==capacityResources
                            && W3DFrameGeneratedProbeAccess::image(camera)==capacityOwners
                            && std::memcmp(rng,after,sizeof(rng))==0,
                            "asset-hash over-capacity mutated source/frame/C1/RNG");
                        assets->Release_Texture(extra.back());extra.back()->Release_Ref();extra.pop_back();
                        require(edge.begin_tree_source_frame() && edge.abort_tree_source_frame()
                            && terrain->preparedTreePhaseIdentity()==retained
                            && device.snapshot()==capacityCommands && device.resource_counts()==capacityResources,
                            "exact asset-hash metadata maximum did not admit/abort without mutation");
                        for (auto *texture:extra) { assets->Release_Texture(texture);texture->Release_Ref(); }
                    }
                    const auto priorCommands=device.snapshot();
                    const auto priorResources=device.resource_counts();
                    const auto priorOwners=W3DFrameGeneratedProbeAccess::image(camera);
                    const auto source=DX8Wrapper::Inspect_Source_State();
                    const auto priorPresents=device.operation_counts().presents;
                    const auto *priorVertex=terrain->peekTreeVertexSource();
                    const auto priorSway=terrain->treeSwayVector(0);
                    CopyGameClientRandomState(rng);
                    if (!lateFault) W3DFrameGeneratedProbeAccess::failPresent(edge);
                    else W3DFrameGeneratedProbeAccess::failCommit(edge);
                    require(rejected(displayFrame),"integrated late source frame fault was admitted");
                    CopyGameClientRandomState(after);
                    const auto restored=DX8Wrapper::Inspect_Source_State();
                    require(W3DFrameGeneratedProbeAccess::faultsConsumed(edge)
                        && !edge.tree_source_frame_pending() && !device.pass_active()
                        && terrain->preparedTreePhaseIdentity()==retained
                        && terrain->peekTreeVertexSource()==priorVertex && terrain->treeSwayVector(0)==priorSway
                        && device.snapshot()==priorCommands && device.resource_counts()==priorResources
                        && W3DFrameGeneratedProbeAccess::image(camera)==priorOwners
                        && W3DFrameGeneratedProbeAccess::textureMetadata(*resident)==residentMetadata
                        && W3DFrameGeneratedProbeAccess::textureMetadata(*nonresident)==nonresidentMetadata
                        && edge.texture_handle(resident)==residentHandle && device.texture_bytes(residentHandle)==residentPixels
                        && W3DFrameGeneratedProbeAccess::sameSource(source,restored)
                        && std::memcmp(rng,after,sizeof(rng))==0,
                        "integrated late source frame rollback changed baseline or retained phase");
                    displayFrame();CopyGameClientRandomState(after);
                    require(!terrain->preparedTreePhaseIdentity() && !edge.tree_source_frame_pending()
                        && device.operation_counts().presents==priorPresents+1
                        && terrain->peekTreeVertexSource()==priorVertex && terrain->treeSwayVector(0)==priorSway
                        && std::memcmp(rng,after,sizeof(rng))==0,
                        "integrated once-only fault retry did not present/retire exactly once");
                    lastCompletedTrace=device.snapshot().substr(priorCommands.size());
                    require(W3DFrameGeneratedProbeAccess::queuesConsumed(frameParticles),
                        "successful tree frame did not consume particle/smudge queue exactly once");
                    require(!resident->Is_Initialized() && !nonresident->Is_Initialized()
                        && !edge.resident_texture(resident)
                        && W3DFrameGeneratedProbeAccess::textureMetadata(*resident)[2]==WW3D::Get_Sync_Time()
                        && W3DFrameGeneratedProbeAccess::textureMetadata(*nonresident)[2]==WW3D::Get_Sync_Time(),
                        "successful tree retry did not publish exact resident/nonresident expiry metadata");
                    WW3D::Set_Thumbnail_Enabled(thumbnails);
                    resident->Release_Ref();nonresident->Release_Ref();
                }
                } catch (...) { TheParticleSystemManager=priorParticles;throw; }
                TheParticleSystemManager=priorParticles;
                const auto &sourceTrace=lastCompletedTrace;
                const auto terrainAt=sourceTrace.find("original RTS3DScene::Render map terrain");
                const auto opaqueAt=sourceTrace.find("draw pipeline=");
                const auto treeAt=sourceTrace.find("original BaseHeightMap::renderTrees immutable indexed triangles");
                // The producer marker witnesses completed submission.
                const auto treeDrawAt=sourceTrace.rfind("draw pipeline=",treeAt);
                const auto preTreeAt=sourceTrace.find("original tree source opaque/empty-occluded/shader flush complete");
                const auto stencilAt=sourceTrace.find("original tree source post-tree stencil flush complete");
                const auto waterAt=sourceTrace.find("original tree source static/water flush complete");
                const auto particleAt=sourceTrace.find("original tree source particle/smudge flush complete");
                const auto endAt=sourceTrace.find("end_pass",treeDrawAt);
                const auto presentAt=sourceTrace.find("present ",endAt);
                require(terrainAt<opaqueAt && opaqueAt<preTreeAt && preTreeAt<treeDrawAt && treeDrawAt<treeAt
                    && treeAt<stencilAt && stencilAt<waterAt && waterAt<particleAt && particleAt<endAt
                    && endAt<presentAt && presentAt!=std::string::npos
                    && sourceTrace.find("original BaseHeightMap::renderTrees immutable indexed triangles",treeAt+1)==std::string::npos
                    && sourceTrace.find("present ",presentAt+1)==std::string::npos,
                    "source tree frame reordered/duplicated terrain-tree-end-present adjacency");
                drawFrame();
                require(terrain->preparedTreePhaseIdentity()>identity,"next successful phase reused identity");
                const auto nextIdentity=terrain->preparedTreePhaseIdentity();
                CopyGameClientRandomState(rng);
                edge.bind_frame_targets(color,depth,32,24);
                require(rejected(drawFrame) && terrain->preparedTreePhaseIdentity()==nextIdentity,
                    "stale target generation retried/crossed prepared phase");
                const auto staleCommands=device.snapshot();const auto staleResources=device.resource_counts();
                require(rejected(displayFrame) && terrain->preparedTreePhaseIdentity()==nextIdentity
                    && device.snapshot()==staleCommands && device.resource_counts()==staleResources,
                    "public stale phase consumed identity or emitted frame commands");
                require(!terrain->completeTreeRenderPhase(nextIdentity),"stale frame generation completed phase");
                CopyGameClientRandomState(after);
                require(std::memcmp(rng,after,sizeof(rng))==0,"stale target rejection advanced RNG");
                const auto beforeCancel=device.resource_counts();
                terrain->cancelTreeRenderPhase();
                require(!terrain->preparedTreePhaseIdentity(),"cancellation retained phase identity");
                const auto canceled=device.resource_counts();
                require(canceled.buffers+2==beforeCancel.buffers
                    && canceled.samplers+2==beforeCancel.samplers && canceled.shaders+2==beforeCancel.shaders
                    && canceled.pipelines+1==beforeCancel.pipelines && canceled.textures==beforeCancel.textures,
                    "cancellation did not retire exactly seven phase-owned units");
                const Bool savedPause=TheGameLogic->isGamePaused();
                TheGameLogic->setGamePaused(TRUE,FALSE);
                const auto pausedSway=terrain->treeSwayVector(0);
                CopyGameClientRandomState(rng);
                drawFrame();CopyGameClientRandomState(after);
                require(terrain->treeSwayVector(0)==pausedSway && std::memcmp(rng,after,sizeof(rng))==0,
                    "source pause advanced breeze/RNG");
                const auto pausedIdentity=terrain->preparedTreePhaseIdentity();
                require(terrain->completeTreeRenderPhase(pausedIdentity),"paused completion rejected");
                camera->Set_Position(Vector3(10000,10000,30));
                drawFrame();
                require(terrain->treeVisibleCount()==0 && terrain->preparedTreePhaseIdentity()>pausedIdentity,
                    "hidden source phase substituted visible geometry");
                const auto hiddenCommands=device.snapshot();const auto hiddenPresents=device.operation_counts().presents;
                const auto *hiddenVertex=terrain->peekTreeVertexSource();CopyGameClientRandomState(rng);
                displayFrame();CopyGameClientRandomState(after);
                require(!terrain->preparedTreePhaseIdentity() && terrain->peekTreeVertexSource()==hiddenVertex
                    && device.operation_counts().presents==hiddenPresents+1
                    && device.snapshot().substr(hiddenCommands.size()).find(
                        "original BaseHeightMap::renderTrees immutable indexed triangles")==std::string::npos
                    && std::memcmp(rng,after,sizeof(rng))==0,
                    "hidden public tree frame emitted triangles or retained/re-advanced phase");
                terrain->cancelTreeRenderPhase();
                camera->Set_Position(Vector3(12,18,30));
                TheGameLogic->setGamePaused(savedPause,FALSE);
                drawFrame();
                TheWritableGlobalData->m_useTrees=FALSE;
                drawFrame();
                require(!terrain->preparedTreePhaseIdentity(),"disabled trees retained immutable phase");
                TheWritableGlobalData->m_useTrees=TRUE;
                drawFrame();
                terrain->removeTree(static_cast<DrawableID>(501));
                require(!terrain->preparedTreePhaseIdentity(),"tree removal retained phase pins");
                const auto removedCommands=device.snapshot();CopyGameClientRandomState(rng);
                displayFrame();CopyGameClientRandomState(after);
                require(!terrain->preparedTreePhaseIdentity()
                    && device.snapshot().substr(removedCommands.size()).find(
                        "original BaseHeightMap::renderTrees immutable indexed triangles")==std::string::npos
                    && std::memcmp(rng,after,sizeof(rng))==0,
                    "removed public tree frame replayed triangles/C1/RNG");
                require(terrain->prepareTreeRenderPhase(camera)==BaseHeightMapRenderObjClass::TREE_PHASE_EMPTY,
                    "empty phase silently required tree providers");
                Object* crusher=TheGameLogic->getFirstObject();
                while (crusher && crusher->getCrusherLevel()<=1) crusher=crusher->getNextObject();
                require(crusher && TheFXListStore,"prepared FX generated crusher/provider missing");
                const Coord3D crusherPosition=*crusher->getPosition();
                const GeometryInfo crusherGeometry=crusher->getGeometryInfo();
                const Bool priorPause=TheGameLogic->isGamePaused();
                TheGameLogic->setGamePaused(TRUE,FALSE);
                auto* fx=const_cast<FXList*>(TheFXListStore->findFXList("FixtureGraph60"));
                require(fx,"prepared FX source-store list missing");
                const Coord3D unitPosition{12,18,2};
                const Coord3D treePosition{13,18,2};
                auto* localPlayer=ThePlayerList->getLocalPlayer();
                const auto playerMask=localPlayer->getPlayerMask();
                PreparationShroudNotices shroudNotices;
                // Establish visited-then-fogged source state; never-seen
                // shroud is a different native topple branch.
                TheDisplay=&shroudNotices;
                try {
                    ThePartitionManager->doShroudReveal(treePosition.x,treePosition.y,1,playerMask);
                    const auto baselineReveal=shroudNotices.notices.size();
                    ThePartitionManager->undoShroudReveal(treePosition.x,treePosition.y,1,playerMask);
                    require(baselineReveal>0 && shroudNotices.notices.size()==2*baselineReveal,
                        "preparation fog baseline notification count differs");
                    for (std::size_t index=0;index<baselineReveal;++index) {
                        const auto& reveal=shroudNotices.notices[index];
                        const auto& undo=shroudNotices.notices[baselineReveal+index];
                        require(reveal.status==CELLSHROUD_CLEAR && undo.status==CELLSHROUD_FOGGED
                            && reveal.x==undo.x && reveal.y==undo.y,
                            "preparation fog baseline notification sequence differs");
                    }
                } catch (...) { TheDisplay=&display;throw; }
                TheDisplay=&display;
                shroudNotices.notices.clear();
                require(ThePartitionManager->getPropShroudStatusForPlayer(localPlayer->getPlayerIndex(),&treePosition)
                    == OBJECTSHROUD_FOGGED,"preparation logical fog baseline missing");
                data.m_doTopple=TRUE;data.m_killWhenToppled=TRUE;
                data.m_toppleFX=NULL;data.m_bounceFX=NULL;
                data.m_sinkFrames=1;data.m_sinkDistance=2;
                data.m_minimumToppleSpeed=2;data.m_initialVelocityPercent=1;
                data.m_initialAccelPercent=0;data.m_bounceVelocityPercent=0;
                TheGameLogic->setGamePaused(FALSE,FALSE);
                TheDisplay=&shroudNotices;
                try { ThePartitionManager->doShroudReveal(treePosition.x,treePosition.y,1,playerMask); }
                catch (...) { TheDisplay=&display;throw; }
                TheDisplay=&display;
                const auto sinkRevealCount=shroudNotices.notices.size();
                require(terrain->tryAddTree(static_cast<DrawableID>(503),treePosition,1,0,0,&data),
                    "last-tree sink preparation admission rejected");
                crusher->setGeometryInfo(GeometryInfo(GEOMETRY_CYLINDER,FALSE,2,4,4));
                crusher->setPosition(&unitPosition);
                drawFrame();
                require(terrain->treeToppleState(static_cast<DrawableID>(503))==4
                    && terrain->completeTreeRenderPhase(terrain->preparedTreePhaseIdentity()),
                    "last-tree topple did not enter source DOWN phase");
                const auto downVertex=terrain->peekTreeVertexSource();
                drawFrame();
                require(terrain->treeToppleState(static_cast<DrawableID>(503))==4
                    && terrain->treeInstanceCount()==1 && terrain->peekTreeVertexSource()!=downVertex
                    && terrain->completeTreeRenderPhase(terrain->preparedTreePhaseIdentity()),
                    "last-tree sink did not publish its decrement before deletion");
                const auto sinkVertex=terrain->peekTreeVertexSource();
                const auto sinkResources=device.resource_counts();
                CopyGameClientRandomState(rng);
                device.fail_next_transaction_checkpoint();
                require(rejected(drawFrame) && terrain->treeInstanceCount()==1
                    && !terrain->preparedTreePhaseIdentity() && terrain->peekTreeVertexSource()==sinkVertex
                    && device.resource_counts()==sinkResources,"rejected last-delete probe mutated accepted tree");
                CopyGameClientRandomState(after);
                require(std::memcmp(rng,after,sizeof(rng))==0,"rejected last-delete probe consumed RNG");
                require(terrain->prepareTreeRenderPhase(camera)==BaseHeightMapRenderObjClass::TREE_PHASE_EMPTY
                    && terrain->treeInstanceCount()==0 && !terrain->preparedTreePhaseIdentity(),
                    "successful last-delete preparation did not publish explicit EMPTY");
                TheDisplay=&shroudNotices;
                try { ThePartitionManager->undoShroudReveal(treePosition.x,treePosition.y,1,playerMask); }
                catch (...) { TheDisplay=&display;throw; }
                TheDisplay=&display;
                require(sinkRevealCount>0 && shroudNotices.notices.size()==2*sinkRevealCount,
                    "last-tree sink notification pairing differs");
                for (std::size_t index=0;index<sinkRevealCount;++index) {
                    const auto& reveal=shroudNotices.notices[index];
                    const auto& undo=shroudNotices.notices[sinkRevealCount+index];
                    require(reveal.x==undo.x && reveal.y==undo.y && reveal.status==CELLSHROUD_CLEAR
                        && undo.status==CELLSHROUD_FOGGED,"last-tree sink notification sequence differs");
                }
                crusher->setPosition(&crusherPosition);
                TheGameLogic->setGamePaused(TRUE,FALSE);
                for (const Bool reset:{FALSE,TRUE}) {
                    Int dispatches=0;
                    fx->clear();
                    fx->addFXNugget(newInstance(PreparedPhaseFX)(&dispatches,reset?NULL:terrain,reset?&visual:NULL));
                    fx->addFXNugget(newInstance(PreparedPhaseFX)(&dispatches,NULL,NULL));
                    data.m_doTopple=TRUE;data.m_killWhenToppled=FALSE;data.m_toppleFX=fx;
                    camera->Set_Position(Vector3(treePosition.x,treePosition.y,50));
                    require(terrain->tryAddTree(static_cast<DrawableID>(502),treePosition,1,0,0,&data),
                        "prepared FX tree admission rejected");
                    crusher->setGeometryInfo(GeometryInfo(GEOMETRY_CYLINDER,FALSE,2,4,4));
                    crusher->setPosition(&unitPosition);
                    require(terrain->treeToppleStartEvents(static_cast<DrawableID>(502))==1,
                        "prepared FX public collision did not stage intent");
                    if (!reset) {
                        require(ThePartitionManager->getShroudStatusForPlayer(localPlayer->getPlayerIndex(),&treePosition)
                            != CELLSHROUD_CLEAR,"suppressed FX fixture unexpectedly clear");
                        drawFrame();
                        require(dispatches==0 && terrain->treeConsumedToppleStartEvents(static_cast<DrawableID>(502))==1
                            && terrain->preparedTreePhaseIdentity(),"source shroud suppression replayed or retained intent");
                        terrain->removeTree(static_cast<DrawableID>(502));
                        crusher->setPosition(&crusherPosition);
                        require(terrain->tryAddTree(static_cast<DrawableID>(502),treePosition,1,0,0,&data),
                            "clear FX retry tree admission rejected");
                        crusher->setPosition(&unitPosition);
                    }
                    const auto firstNotice=shroudNotices.notices.size();
                    TheDisplay=&shroudNotices;
                    try { ThePartitionManager->doShroudReveal(treePosition.x,treePosition.y,1,playerMask); }
                    catch (...) { TheDisplay=&display;throw; }
                    TheDisplay=&display;
                    const auto revealCount=shroudNotices.notices.size()-firstNotice;
                    require(ThePartitionManager->getShroudStatusForPlayer(localPlayer->getPlayerIndex(),&treePosition)
                        == CELLSHROUD_CLEAR && revealCount>0 && TheDisplay==&display,
                        "prepared FX reveal did not establish real clear status");
                    CopyGameClientRandomState(rng);
                    device.fail_next_transaction_checkpoint();
                    require(rejected(drawFrame) && dispatches==0
                        && terrain->treeConsumedToppleStartEvents(static_cast<DrawableID>(502))==0,
                        "prepared FX rejected probe consumed/dispatch intent");
                    CopyGameClientRandomState(after);
                    require(std::memcmp(rng,after,sizeof(rng))==0,"prepared FX rejected probe consumed RNG");
                    const bool canceledDraw=rejected(drawFrame);
                    require(canceledDraw && dispatches==2
                        && !terrain->preparedTreePhaseIdentity() && terrain->treeInstanceCount()==0,
                        "prepared FX removal/reset failed cancellation or lost later nugget");
                    require(!terrain->completeTreeRenderPhase(nextIdentity),"old epoch completed new/canceled phase");
                    crusher->setPosition(&crusherPosition);
                    TheDisplay=&shroudNotices;
                    try { ThePartitionManager->undoShroudReveal(treePosition.x,treePosition.y,1,playerMask); }
                    catch (...) { TheDisplay=&display;throw; }
                    TheDisplay=&display;
                    require(shroudNotices.notices.size()==firstNotice+2*revealCount,
                        "prepared FX reveal/undo notification count differs");
                    for (std::size_t index=0;index<revealCount;++index) {
                        const auto& reveal=shroudNotices.notices[firstNotice+index];
                        const auto& undo=shroudNotices.notices[firstNotice+revealCount+index];
                        require(reveal.status==CELLSHROUD_CLEAR && undo.status==CELLSHROUD_FOGGED
                            && reveal.x==undo.x && reveal.y==undo.y && TheDisplay==&display,
                            "prepared FX reveal/undo notifications or display restoration changed");
                    }
                    if (!reset) require(terrain->prepareTreeRenderPhase(camera)==BaseHeightMapRenderObjClass::TREE_PHASE_EMPTY
                        && dispatches==2,"removed phase replayed effects");
                    fx->clear();
                }
                crusher->setGeometryInfo(crusherGeometry);
                TheGameLogic->setGamePaused(priorPause,FALSE);
                visual.reset();
                edge.release_source_buffers();
                device.destroy(depth);device.destroy(color);
            } catch (...) {
                TheTacticalView=savedView;TheTerrainVisual=savedVisual;TheDisplay=savedDisplay;throw;
            }
            TheTacticalView=savedView;TheTerrainVisual=savedVisual;TheDisplay=savedDisplay;
        }
        edge.release_source_buffers();
        } // Generic terrain programs retire at the existing Edge destructor.
        require(device.resource_counts().total()==0,"tree preparation generation retained resources");
    }
    if (std::getenv("ZH_M22_TREE_DRAW_BOUNDARIES")) {
        const auto *boundaryAsset=std::getenv("ZH_M22_TREE_DRAW_BOUNDARY_ASSET");
        require(boundaryAsset!=nullptr,"generated exact boundary asset missing");
        recordingBoundaries(path,boundaryAsset);
    }
    if (std::getenv("ZH_M22_TREE_DRAW_PHYSICAL")) physicalDraw(path,asset);
    TheWritableGlobalData->m_partitionCellSize=savedPartition;
    TheWritableGlobalData->m_useTrees=savedTrees;
    std::puts("original tree preparation: immutable=1 retry=1 generations=2 resources=0");
    std::puts("original tree draw: source=1 rollback=1 retry=1 generations=2 resources=0");
}
