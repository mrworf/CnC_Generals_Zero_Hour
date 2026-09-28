#include "PreRTS.h"
#include "Common/GlobalData.h"
#include "Common/ThingFactory.h"
#include "Common/ThingTemplate.h"
#include "Common/PlayerList.h"
#include "Common/Player.h"
#include "GameClient/ClientRandomValue.h"
#include "GameLogic/GameLogic.h"
#include "GameLogic/PartitionManager.h"
#include "W3DDevice/GameClient/HeightMap.h"
#include "W3DDevice/GameClient/W3DDisplay.h"
#include "W3DDevice/GameClient/W3DScene.h"
#include "W3DDevice/GameClient/W3DShroud.h"
#include "W3DDevice/GameClient/W3DPropBuffer.h"
#include "W3DDevice/GameClient/W3DTerrainVisual.h"
#include "W3DDevice/GameClient/W3DView.h"
#include "W3DDevice/GameClient/W3DAssetManager.h"
#include "W3DDevice/GameClient/Module/W3DTreeDraw.h"
#include "WW3D2/camera.h"
#include "WW3D2/ww3d.h"
#include "WW3D2/statistics.h"
#include "WW3D2/hlod.h"
#include "WW3D2/hanim.h"
#include "WW3D2/dx8wrapper.h"
#include "WW3D2/dx8vertexbuffer.h"
#include "WW3D2/dx8indexbuffer.h"
#include "WW3D2/distlod.h"
#include "WWLib/RAMFILE.H"
#include "original_gpu_edge.h"
#include "zh/renderer/recording_device.h"
#include "zh/platform/bgfx_device.h"
#include <SDL3/SDL.h>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <iterator>
#include <stdexcept>
#include <type_traits>
#include <vector>

struct W3DFrameGeneratedProbeAccess {
    static void failPresent(zh::original_runtime::OriginalGpuEdge& edge) {edge.source_frame_present_fault_=true;}
    static void failCommit(zh::original_runtime::OriginalGpuEdge& edge) {edge.source_frame_commit_fault_=true;}
    static bool faultsConsumed(const zh::original_runtime::OriginalGpuEdge& edge)
    {return !edge.source_frame_present_fault_ && !edge.source_frame_commit_fault_;}
};
struct W3DPropFrameGeneratedProbeAccess {
    static void animate(BaseHeightMapRenderObjClass& terrain,WW3DAssetManager& assets) {
        auto* motion=assets.Get_HAnim("TESTTREE.IDLE");
        if(!motion) throw std::runtime_error("prop visual motion provider");
        for(int i=0;i<terrain.m_propBuffer->m_numProps;++i)
            if(auto* hierarchy=dynamic_cast<HLodClass*>(terrain.m_propBuffer->m_props[i].m_robj))
                hierarchy->Set_Animation(motion,0,RenderObjClass::ANIM_MODE_LOOP);
        motion->Release_Ref();
    }
    static std::vector<unsigned char> image(BaseHeightMapRenderObjClass& terrain) {
        auto* buffer=terrain.m_propBuffer;
        std::vector<unsigned char> result;
        const auto add=[&](const void* source,std::size_t size) {
            const auto* bytes=static_cast<const unsigned char*>(source);result.insert(result.end(),bytes,bytes+size);
        };
        add(&buffer,sizeof(buffer));add(&buffer->m_numProps,sizeof(buffer->m_numProps));
        add(&buffer->m_numPropTypes,sizeof(buffer->m_numPropTypes));
        add(buffer->m_props,buffer->m_numProps*sizeof(TProp));
        add(&buffer->m_anythingChanged,sizeof(buffer->m_anythingChanged));add(&buffer->m_doCull,sizeof(buffer->m_doCull));
        for (int i=0;i<buffer->m_numProps;++i) if(auto* object=buffer->m_props[i].m_robj) {
            const auto& transform=object->Get_Transform_No_Validity_Check();add(&transform,sizeof(transform));
        }
        return result;
    }
    static void sound(BaseHeightMapRenderObjClass& terrain,WW3DAssetManager& assets,bool doubled,bool enabled) {
        auto* motion=assets.Get_HAnim("TESTTREE.IDLE");
        if(!motion) throw std::runtime_error("prop sound preflight provider");
        motion->Set_Embedded_Sound_Bone_Index(enabled?0:EMBEDDED_SOUND_BONE_INDEX_NOT_SET);
        for(int i=0;i<terrain.m_propBuffer->m_numProps;++i)
            if(auto* hierarchy=dynamic_cast<HLodClass*>(terrain.m_propBuffer->m_props[i].m_robj)) {
                if(doubled) hierarchy->Set_Animation(motion,0,motion,0,0.5f);
                else hierarchy->Set_Animation(motion,0,RenderObjClass::ANIM_MODE_LOOP);
            }
        motion->Release_Ref();
    }
};
namespace {
void require(bool value,const char* category) {if(!value) throw std::runtime_error(category);}
template<class Operation> bool rejected(Operation operation) {try {operation();} catch(...) {return true;} return false;}
// The generated logical partition is larger than the tiny visual map. Record
// real reveal/undo notifications, then restore the real display before drawing.
// This adapter never supplies or bypasses the production shroud query.
class PropShroudNotices final:public Display {
public:
    struct Notice {Int x,y;CellShroudStatus status;};
    std::vector<Notice> notices;
    void setShroudLevel(Int x,Int y,CellShroudStatus status) override {notices.push_back({x,y,status});}
    void doSmartAssetPurgeAndPreload(const char*) override {}
#if defined(_DEBUG) || defined(_INTERNAL)
    void dumpAssetUsage(const char*) override {}
    void dumpModelAssets(const char*) override {}
#endif
    VideoBuffer* createVideoBuffer() override {return NULL;}
    void setClipRegion(IRegion2D*) override {}
    Bool isClippingEnabled() override {return FALSE;}
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
    Real getAverageFPS() override {return 0;}
    Int getLastFrameDrawCalls() override {return 0;}
};
unsigned occurrences(const std::string& trace,const char* token) {
    unsigned count=0;for(auto at=trace.find(token);at!=std::string::npos;at=trace.find(token,at+1)) ++count;return count;
}
std::vector<unsigned char> bindings() {
    RenderStateStruct state;DX8Wrapper::Get_Render_State(state);
    std::vector<unsigned char> result;
    const auto add=[&](const auto& field) {
        const auto* first=reinterpret_cast<const unsigned char*>(&field);
        result.insert(result.end(),first,first+sizeof(field));
    };
    add(state.vertex_buffers[0]);add(state.index_buffer);
    add(state.vertex_buffer_types[0]);add(state.index_buffer_type);
    add(state.vba_offset);add(state.vba_count);add(state.iba_offset);add(state.index_base_offset);
    return result;
}
template<class Device> bool generation(Device& device,const char* map,const char* asset,bool combined,int ordinal=-1) {
    using zh::original_runtime::OriginalGpuEdge;
    constexpr bool recording=std::is_same_v<Device,zh::renderer::RecordingGpuDevice>;
    OriginalGpuEdge edge(device);
    bool operationFailed=false;
    TheWritableGlobalData->m_useShadowDecals=FALSE;
    {
        W3DDisplay display;display.init();
        auto* savedDisplay=TheDisplay;auto* savedVisual=TheTerrainVisual;auto* savedView=TheTacticalView;
        struct Providers {
            Display* display;TerrainVisual* visual;View* view;
            ~Providers() {TheTacticalView=view;TheTerrainVisual=visual;TheDisplay=display;}
        } providers{savedDisplay,savedVisual,savedView};
        TheDisplay=&display;W3DTerrainVisual visual;TheTerrainVisual=&visual;
        try {
            display.setWidth(32);display.setHeight(24);visual.init();
            require(visual.load(AsciiString(map)),"prop frame map admission");
            std::ifstream input(asset,std::ios::binary);
            std::vector<char> bytes(std::istreambuf_iterator<char>{input},std::istreambuf_iterator<char>{});
            RAMFileClass packet(bytes.data(),static_cast<int>(bytes.size()));
            require(!bytes.empty() && static_cast<WW3DAssetManager*>(W3DDisplay::m_assetManager)->Load_3D_Assets(packet),"prop frame asset admission");
            auto* view=new W3DView;TheTacticalView=view;view->init();display.attachView(view);
            view->setWidth(32);view->setHeight(24);view->setDefaultView(0,0,1);
            auto* camera=view->get3DCamera();camera->Set_Clip_Planes(0.1f,1000);camera->Set_Position(Vector3(12,18,30));
            zh::renderer::TextureDesc target;target.width=32;target.height=24;target.render_target=true;
            target.sampled=true;target.format=zh::renderer::TextureFormat::rgba8;
            const auto color=device.create_texture(target,"source prop color");
            target.sampled=false;target.format=zh::renderer::TextureFormat::depth24_stencil8;
            const auto depth=device.create_texture(target,"source prop depth");require(color&&depth,"prop frame targets");
            edge.bind_frame_targets(color,depth,32,24);
            auto* terrain=TheTerrainRenderObject;
            const Coord3D position{12,18,terrain->getMap()->getDisplayHeight(3,3)*MAP_HEIGHT_SCALE+4};
            camera->Set_Position(Vector3(12,18,position.z+15));
            terrain->getShroud()->fillShroudData(255);terrain->getShroud()->render(camera);
            display.draw();require(device.present(color),"ordinary prop baseline present");
            std::vector<std::uint8_t> pixels;
            if constexpr (!recording) pixels=device.readback_rgba(color);
            const auto* model=TheThingFactory->findTemplate(AsciiString("FramePropFixture"),FALSE);
            require(model!=nullptr,"prop frame template");visual.addProp(model,&position,0);
            for(const char* name:{"FrameStaticFixture","FrameSkinFixture","FrameHlodFixture"}) {
                const auto* sibling=TheThingFactory->findTemplate(AsciiString(name),FALSE);
                require(sibling,"prop sibling template");visual.addProp(sibling,&position,0);
            }
            if(ordinal<0) {
                DistLODNodeDefStruct nodes[2];
                nodes[0].Name=const_cast<char*>("TEST.LITONE01");nodes[1].Name=const_cast<char*>("TEST.STATIC01");
                nodes[0].ResDownDist=20;nodes[0].ResUpDist=10;nodes[1].ResDownDist=50;nodes[1].ResUpDist=40;
                W3DDisplay::m_assetManager->Add_Prototype(new DistLODPrototypeClass(new DistLODDefClass("PROP_LEGACY",2,nodes)));
                for(const char* name:{"FrameHmodelFixture","FrameCollectionFixture","FrameLegacyFixture"}) {
                    const auto* source=TheThingFactory->findTemplate(AsciiString(name),FALSE);
                    require(source,"prop composite template");visual.addProp(source,&position,0);
                }
            }
            W3DPropFrameGeneratedProbeAccess::animate(*terrain,*W3DDisplay::m_assetManager);
            require(terrain->hasLiveProps(),"prop frame instance publication");
            if constexpr(recording) if(ordinal<0) {
                const auto source=W3DPropFrameGeneratedProbeAccess::image(*terrain);
                const auto trace=device.snapshot();const auto resources=device.resource_counts();
                const auto frame=WW3D::Get_Frame_Count();UnsignedInt before[6],after[6];CopyGameClientRandomState(before);
                for(unsigned fault=0;fault<4;++fault) {
                    if(fault==0) device.fail_next_texture_create();
                    if(fault==1) device.fail_next_texture_upload();
                    if(fault==2) device.fail_next_transaction_checkpoint();
                    if(fault==3) device.fail_transaction_operation_after(0);
                    require(rejected([&]{terrain->preparePropSourceFrame();}),"prop preparation fault not reached");
                    device.fail_transaction_operation_after(~0U);CopyGameClientRandomState(after);
                    require(device.snapshot()==trace && device.resource_counts()==resources
                        && W3DPropFrameGeneratedProbeAccess::image(*terrain)==source
                        && !edge.tree_source_frame_pending() && WW3D::Get_Frame_Count()==frame
                        && !std::memcmp(before,after,sizeof(before)),"prop preparation failure mutated source/live resources");
                }
            }
            terrain->preparePropSourceFrame();
            // The empty terrain baseline uses the dedicated terrain edge,
            // so it does not select mesh world/view state. Declare the same
            // valid identity baseline before inspecting mesh binding offsets.
            DX8Wrapper::Set_Transform(D3DTS_WORLD,Matrix4x4(true));
            DX8Wrapper::Set_Transform(D3DTS_VIEW,Matrix4x4(true));
            if constexpr(recording) if(ordinal<0) {
                // Registered SINGLE and DOUBLE providers bearing sound must
                // reject before the frame journal, registration or RNG work.
                for(bool doubled:{false,true}) {
                    W3DPropFrameGeneratedProbeAccess::sound(*terrain,*W3DDisplay::m_assetManager,doubled,true);
                    const auto source=W3DPropFrameGeneratedProbeAccess::image(*terrain);
                    const auto trace=device.snapshot();const auto counts=device.resource_counts();
                    const auto frame=WW3D::Get_Frame_Count();UnsignedInt before[6],after[6];CopyGameClientRandomState(before);
                    for(int retry=0;retry<2;++retry) {
                        require(rejected([&]{terrain->preparePropSourceFrame();}),"prop sound-bearing graph admitted");
                        CopyGameClientRandomState(after);
                        require(device.snapshot()==trace && device.resource_counts()==counts
                            && W3DPropFrameGeneratedProbeAccess::image(*terrain)==source
                            && WW3D::Get_Frame_Count()==frame && !edge.tree_source_frame_pending()
                            && !std::memcmp(before,after,sizeof(before)),"prop sound preflight mutation");
                    }
                }
                W3DPropFrameGeneratedProbeAccess::sound(*terrain,*W3DDisplay::m_assetManager,false,false);
                terrain->preparePropSourceFrame();
                const auto baseline=bindings();const auto trace=device.snapshot();
                {
                    DynamicVBAccessClass access(BUFFER_TYPE_DYNAMIC_SORTING,dynamic_fvf_type,3);
                    require(rejected([&]{DX8Wrapper::Set_Vertex_Buffer(access);}),"prop outside-frame sorting VB admitted");
                }
                {
                    DynamicIBAccessClass access(BUFFER_TYPE_DYNAMIC_SORTING,3);
                    require(rejected([&]{DX8Wrapper::Set_Index_Buffer(access,0);}),"prop outside-frame sorting IB admitted");
                }
                require(bindings()==baseline && device.snapshot()==trace,"prop rejected sorting binding mutated state");
                DynamicVBAccessClass::_Reset(false);DynamicIBAccessClass::_Reset(false);
            }
            require(ThePartitionManager && ThePlayerList && ThePlayerList->getLocalPlayer(),"prop shroud providers");
            const auto player=ThePlayerList->getLocalPlayer()->getPlayerIndex();
            const auto mask=ThePlayerList->getLocalPlayer()->getPlayerMask();
            const Real radius=2*ThePartitionManager->getCellSize();
            PropShroudNotices notices;
            const auto reveal=[&](bool undo) {
                TheDisplay=&notices;
                try {
                    if(undo) ThePartitionManager->undoShroudReveal(position.x,position.y,radius,mask);
                    else ThePartitionManager->doShroudReveal(position.x,position.y,radius,mask);
                } catch(...) {TheDisplay=&display;throw;}
                TheDisplay=&display;terrain->notifyShroudChanged();
            };
            if constexpr(recording) if(ordinal<0) {
                const auto status=ThePartitionManager->getPropShroudStatusForPlayer(player,&position);
                require(status==OBJECTSHROUD_SHROUDED || status==OBJECTSHROUD_FOGGED,"prop logical generation entry");
                if(status==OBJECTSHROUD_SHROUDED) {
                    const auto before=device.snapshot();display.draw();const auto trace=device.snapshot().substr(before.size());
                    require(occurrences(trace,"draw pipeline=")==14 && occurrences(trace,"present ")==1,
                        "prop SHROUDED suppression/source terrain frame");
                }
            }
            reveal(false);const auto first=notices.notices.size();reveal(true);
            require(first && notices.notices.size()==2*first &&
                ThePartitionManager->getPropShroudStatusForPlayer(player,&position)==OBJECTSHROUD_FOGGED,
                "prop real fog baseline");
            for(std::size_t i=0;i<first;++i) require(notices.notices[i].status==CELLSHROUD_CLEAR
                && notices.notices[first+i].status==CELLSHROUD_FOGGED && notices.notices[i].x==notices.notices[first+i].x
                && notices.notices[i].y==notices.notices[first+i].y,"prop reveal/undo notification identity");
            if constexpr(recording) if(ordinal<0) {
                const auto before=device.snapshot();display.draw();const auto trace=device.snapshot().substr(before.size());
                require(occurrences(trace,"draw pipeline=")>14 && occurrences(trace,"present ")==1,
                    "prop native FOGGED eligibility/shroud pass");
            }
            reveal(false);const auto finalFirst=notices.notices.size();
            require(ThePartitionManager->getPropShroudStatusForPlayer(player,&position)==OBJECTSHROUD_CLEAR
                && TheDisplay==&display,"prop real clear/display restoration");
            W3DTreeDrawModuleData tree;
            if(combined) {
                tree.m_modelName="TEST.LITONE01";tree.m_textureName="Tree0.tga";tree.m_doShadow=TRUE;
                require(terrain->tryAddTree(static_cast<DrawableID>(701),position,8,0,0,&tree),"combined prop tree admission");
                TheWritableGlobalData->m_useShadowDecals=TRUE;
                require(terrain->prepareTreeRenderPhase(camera)==BaseHeightMapRenderObjClass::TREE_PHASE_READY,"combined prop phase");
            }
            // Completed shroud/terrain frames may leave no selected mesh
            // transform. Re-establish the declared mesh inspection baseline.
            DX8Wrapper::Set_Transform(D3DTS_WORLD,Matrix4x4(true));
            DX8Wrapper::Set_Transform(D3DTS_VIEW,Matrix4x4(true));
            const auto phase=terrain->preparedTreePhaseIdentity();
            if constexpr(recording) if(ordinal<0) {
                const auto image=W3DPropFrameGeneratedProbeAccess::image(*terrain);
                const auto trace=device.snapshot();const auto resources=device.resource_counts();
                require(edge.begin_tree_source_frame(true),"prop active removal journal admission");
                require(rejected([&]{visual.reset();}) && rejected([&]{terrain->freeMapResources();})
                    && rejected([&]{visual.addProp(model,&position,0);}),"prop pending checkpoint mutation accepted");
                require(WW3D::Begin_Render()==WW3D_ERROR_OK,"prop active removal source frame begin");
                camera->Apply();DX8Wrapper::Set_World_Identity();
                require(rejected([&]{visual.reset();}) && rejected([&]{terrain->freeMapResources();}),
                    "prop active frame removal accepted");
                require(WW3D::End_Render(false)==WW3D_ERROR_OK,"prop active removal source frame end");
                require(edge.abort_tree_source_frame() && device.snapshot()==trace && device.resource_counts()==resources
                    && terrain->preparedTreePhaseIdentity()==phase
                    && W3DPropFrameGeneratedProbeAccess::image(*terrain)==image,
                    "prop active removal changed owner/phase/buffers");
                terrain->preparePropSourceFrame();
            }
            for(unsigned fault=0;fault<unsigned(ordinal<0?2:1);++fault) {
                const auto source=W3DPropFrameGeneratedProbeAccess::image(*terrain);
                const auto frame=WW3D::Get_Frame_Count();UnsignedInt rng[6],after[6];CopyGameClientRandomState(rng);
                const auto bound=bindings();
                if constexpr(recording) {
                    const auto trace=device.snapshot();const auto resources=device.resource_counts();
                    if(ordinal>=0) device.fail_transaction_operation_after(unsigned(ordinal));
                    else if(!fault) W3DFrameGeneratedProbeAccess::failPresent(edge);else W3DFrameGeneratedProbeAccess::failCommit(edge);
                    operationFailed=rejected([&]{display.draw();});
                    if(ordinal>=0) device.fail_transaction_operation_after(~0U);
                    else require(operationFailed,"prop late rejection not reached");
                    if(!operationFailed) break;
                    const std::string preparation=combined ? "" : "marker \"original W3DView::updateView terrain center\"\n";
                    require(device.snapshot().substr(trace.size())==preparation && device.resource_counts()==resources,
                        "prop transactional frame rollback/preparation marker sequence");
                } else {
                    if(!fault) W3DFrameGeneratedProbeAccess::failPresent(edge);else W3DFrameGeneratedProbeAccess::failCommit(edge);
                    require(rejected([&]{display.draw();}) && device.readback_rgba(color)==pixels,"prop physical partial pixels");
                }
                CopyGameClientRandomState(after);
                require(!edge.tree_source_frame_pending() && !device.pass_active() && !WW3D::Is_Rendering()
                    && W3DFrameGeneratedProbeAccess::faultsConsumed(edge)
                    && bindings()==bound
                    && terrain->preparedTreePhaseIdentity()==phase && W3DPropFrameGeneratedProbeAccess::image(*terrain)==source
                    && WW3D::Get_Frame_Count()==frame && !std::memcmp(rng,after,sizeof(rng)),"prop exact frame rollback");
            }
            if constexpr(recording) {
              if(ordinal<0 || operationFailed) {
                const auto before=device.snapshot();display.draw();const auto trace=device.snapshot().substr(before.size());
                const auto update=trace.find("W3DView::updateView terrain center");
                require(occurrences(trace,"W3DView::updateView terrain center")==unsigned(!combined)
                    && (combined || update<trace.find("begin_pass ")),
                    "prop retry preparation marker order");
                const auto prop=trace.find("HeightMap prop mesh task generation");
                const auto flush=trace.find("opaque/empty-occluded/shader flush complete");
                require(prop!=std::string::npos && flush>prop && occurrences(trace,"draw pipeline=")>14
                    && occurrences(trace,"present ")==1 && trace.find("present ")>flush,"prop source task/draw/present order");
                if(combined) require(trace.find("multiplicative tree triangles")>flush
                    && trace.find("immutable indexed triangles")>trace.find("multiplicative tree triangles"),"combined prop decal/tree order");
              }
            } else {
                display.draw();const auto accepted=device.readback_rgba(color);
                require(accepted.size()==32*24*4 && accepted!=pixels,"prop physical distinct pixels");
                display.draw();require(device.readback_rgba(color)==accepted,"prop physical identical retry pixels");
            }
            require(!terrain->preparedTreePhaseIdentity() && !edge.tree_source_frame_pending(),"prop frame completion");
            reveal(true);const auto count=finalFirst-2*first;
            require(count && notices.notices.size()==finalFirst+count &&
                ThePartitionManager->getPropShroudStatusForPlayer(player,&position)==OBJECTSHROUD_FOGGED,
                "prop final logical generation baseline");
            for(std::size_t i=0;i<count;++i) require(notices.notices[2*first+i].status==CELLSHROUD_CLEAR
                && notices.notices[finalFirst+i].status==CELLSHROUD_FOGGED
                && notices.notices[2*first+i].x==notices.notices[finalFirst+i].x
                && notices.notices[2*first+i].y==notices.notices[finalFirst+i].y,"prop final reveal/undo order");
            visual.reset();edge.release_source_buffers();device.destroy(depth);device.destroy(color);
        } catch(...) {
            // Preserve the primary generated failure: prop teardown requires
            // the still-published exact display/terrain provider identity.
            if(edge.tree_source_frame_pending() && !edge.abort_tree_source_frame()) std::terminate();
            visual.reset();
            throw;
        }
    }
    return operationFailed;
}
}
extern "C" void zh_probe_prop_frame() {
    const auto* map=std::getenv("ZH_M22_PROP_FRAME_MAP");const auto* asset=std::getenv("ZH_M22_PROP_FRAME_ASSET");
    require(map&&asset,"prop frame generated inputs");
    const auto trees=TheWritableGlobalData->m_useTrees,shadows=TheWritableGlobalData->m_useShadowDecals;
    const auto partition=TheWritableGlobalData->m_partitionCellSize;
    TheWritableGlobalData->m_partitionCellSize=MAP_XY_FACTOR;
    const auto paused=TheGameLogic->isGamePaused();TheGameLogic->setGamePaused(TRUE,FALSE);
    for(unsigned pass=0;pass<2;++pass) for(unsigned repeat=0;repeat<2;++repeat) {
        TheWritableGlobalData->m_useTrees=pass!=0;TheWritableGlobalData->m_useShadowDecals=pass!=0;
        zh::renderer::RecordingGpuDevice device;generation(device,map,asset,pass!=0);
        require(device.resource_counts().total()==0,"prop Recording teardown resources");
    }
    for(unsigned pass=0;pass<2;++pass) {
        TheWritableGlobalData->m_useTrees=pass!=0;unsigned ordinal=0;
        for(;ordinal<4096;++ordinal) {
            zh::renderer::RecordingGpuDevice device;
            const bool failed=generation(device,map,asset,pass!=0,int(ordinal));
            require(!device.resource_counts().total(),"prop operation generation teardown");
            if(!failed) break;
        }
        require(ordinal>15 && ordinal<4096,"prop operation sweep incomplete/bound");
        std::printf("original prop frame boundaries: combined=%u rejected=%u retry=1\n",pass,ordinal);
    }
    std::puts("original prop frame: source=1 rollback=1 retry=1 generations=2 resources=0");
    if(std::getenv("ZH_M22_PROP_FRAME_PHYSICAL")) {
        require(SDL_Init(SDL_INIT_VIDEO),"prop physical video services");
        auto* window=SDL_CreateWindow("generated source prop frame",32,24,SDL_WINDOW_HIDDEN);require(window,"prop physical window");
        for(unsigned pass=0;pass<2;++pass) for(unsigned repeat=0;repeat<2;++repeat) {
            TheWritableGlobalData->m_useTrees=pass!=0;TheWritableGlobalData->m_useShadowDecals=pass!=0;
            zh::renderer::BgfxOptions options;options.shader_root=ZH_BGFX_SHADER_DIR;
            zh::renderer::BgfxGpuDevice device(options);require(device.claim_window(window),"prop physical claim");
            generation(device,map,asset,pass!=0);device.release_window();
            require(device.wait_idle()&&device.live_resource_count()==0,"prop physical teardown resources");
        }
        SDL_DestroyWindow(window);SDL_Quit();
        std::puts("original prop frame physical: source=1 rollback=1 retry=1 generations=2 resources=0");
    }
    Debug_Statistics::Shutdown_Statistics();
    TheWritableGlobalData->m_useTrees=trees;TheWritableGlobalData->m_useShadowDecals=shadows;
    TheWritableGlobalData->m_partitionCellSize=partition;
    TheGameLogic->setGamePaused(paused,FALSE);
}
