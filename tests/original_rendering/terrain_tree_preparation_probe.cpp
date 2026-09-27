#include "PreRTS.h"
#include "Common/GlobalData.h"
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
#include "W3DDevice/GameClient/W3DAssetManager.h"
#include "W3DDevice/GameClient/Module/W3DTreeDraw.h"
#include "WWLib/RAMFILE.H"
#include "assetmgr.h"
#include "camera.h"
#include "original_gpu_edge.h"
#include "zh/renderer/recording_device.h"
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <iterator>
#include <stdexcept>
#include <vector>

namespace {
void require(bool condition,const char* message)
{ if (!condition) throw std::runtime_error(message); }
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
}

extern "C" void zh_probe_terrain_tree_preparation()
{
    const char* path=std::getenv("ZH_M22_TREE_PREPARATION_MAP");
    const char* asset=std::getenv("ZH_M22_TREE_PREPARATION_ASSET");
    require(path && asset,"tree preparation generated providers missing");
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
                const auto drawFrame=[&] {
                    display.draw();
                    require(device.present(color).valid,"tree preparation ordinary presentation rejected");
                };
                auto* terrain=TheTerrainRenderObject;
                terrain->getShroud()->render(camera);
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
                    const bool failed=rejected(drawFrame);
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
                require(identity && terrain->treeVisibleCount()==1,"public draw did not prepare exact visible phase");
                CopyGameClientRandomState(rng);
                const auto acceptedResources=device.resource_counts();
                const auto* acceptedVertex=terrain->peekTreeVertexSource();
                const auto sway=terrain->treeSwayVector(0);
                drawFrame();CopyGameClientRandomState(after);
                require(terrain->preparedTreePhaseIdentity()==identity
                    && terrain->peekTreeVertexSource()==acceptedVertex
                    && terrain->treeSwayVector(0)==sway && device.resource_counts()==acceptedResources
                    && std::memcmp(rng,after,sizeof(rng))==0,
                    "same-phase public retry advanced C1/RNG/resources");
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
                drawFrame();
                require(terrain->preparedTreePhaseIdentity()>identity,"next successful phase reused identity");
                const auto nextIdentity=terrain->preparedTreePhaseIdentity();
                CopyGameClientRandomState(rng);
                edge.bind_frame_targets(color,depth,32,24);
                require(rejected(drawFrame) && terrain->preparedTreePhaseIdentity()==nextIdentity,
                    "stale target generation retried/crossed prepared phase");
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
    TheWritableGlobalData->m_partitionCellSize=savedPartition;
    TheWritableGlobalData->m_useTrees=savedTrees;
    std::puts("original tree preparation: immutable=1 retry=1 generations=2 resources=0");
}
