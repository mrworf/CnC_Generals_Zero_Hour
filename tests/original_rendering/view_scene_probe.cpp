#include "PreRTS.h"
#include "GameLogic/GameLogic.h"
#include "Common/GlobalData.h"
#include "GameClient/CommandXlat.h"
#include "W3DDevice/GameClient/W3DDisplay.h"
#include "W3DDevice/GameClient/W3DScene.h"
#include "W3DDevice/GameClient/W3DView.h"
#include "W3DDevice/GameClient/W3DAssetManager.h"
#include "W3DDevice/GameClient/HeightMap.h"
#include "W3DDevice/GameClient/W3DShadow.h"
#include "W3DDevice/GameClient/W3DTerrainTracks.h"
#include "W3DDevice/GameClient/W3DWater.h"
#include "W3DDevice/GameClient/W3DSmudge.h"
#include "W3DDevice/GameClient/W3DTerrainVisual.h"
#include "WW3D2/assetmgr.h"
#include "WW3D2/camera.h"
#include "WW3D2/dx8wrapper.h"
#include "WW3D2/rendobj.h"
#include "WW3D2/rinfo.h"
#include "WW3D2/ww3d.h"
#include "WWLib/RAMFILE.H"
#include "original_gpu_edge.h"
#include "zh/platform/bgfx_device.h"
#include "zh/renderer/recording_device.h"

#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <iterator>
#include <stdexcept>
#include <string>
#include <vector>

namespace {
void require(bool value, const char* message)
{
    if (!value) throw std::runtime_error(message);
}
}

extern "C" void zh_probe_view_scene()
{
    require(TheGameLogic && TheGameLogic->isInGame(),
        "original view frame requires initialized GameClient scenario");
    const char* path = std::getenv("ZH_M22_VIEW_SCENE_ASSET");
    require(path, "original view frame packet path missing");
    std::ifstream input(path, std::ios::binary);
    require(input.good(), "original view frame packet unreadable");
    std::vector<char> bytes(std::istreambuf_iterator<char>{input}, std::istreambuf_iterator<char>{});
    require(!bytes.empty(), "original view frame packet empty");
    const bool physical = std::getenv("ZH_M22_VIEW_SCENE_PHYSICAL") != nullptr;
    auto run = [&](zh::renderer::GpuDevice& device, unsigned width,
        zh::renderer::TextureFormat format, auto readback) {
        zh::renderer::TextureDesc target;
        target.width = width;
        target.height = width * 3 / 4;
        target.render_target = true;
        target.format = format;
        const auto color = device.create_texture(target, "original W3DView color");
        target.format = zh::renderer::TextureFormat::depth24_stencil8;
        auto depth = device.create_texture(target, "original W3DView depth");
        require(color && depth, "original W3DView frame targets missing");
        std::vector<unsigned char> empty_baseline;
        {
            W3DView no_edge;
            bool rejected = false;
            try { no_edge.init(); }
            catch (const std::runtime_error&) { rejected = true; }
            require(rejected && !no_edge.get3DCamera(),
                "original W3DView initialized without source display/edge");
            bool no_terrain_edge = false;
            try { auto* terrain = NEW_REF(HeightMapRenderObjClass, ()); terrain->Release_Ref(); }
            catch (const std::runtime_error&) { no_terrain_edge = true; }
            require(no_terrain_edge && !TheTerrainRenderObject && !TheHeightMap,
                "original empty terrain published without device edge");
            W3DShadowManager no_shadow_edge;
            bool shadow_edge_rejected = false;
            try { no_shadow_edge.init(); }
            catch (const std::runtime_error&) { shadow_edge_rejected = true; }
            require(shadow_edge_rejected && !TheW3DShadowManager,
                "original disabled-shadow owner initialized without device edge");
            const Int saved_no_edge_tracks = TheGlobalData->m_maxTerrainTracks;
            TheWritableGlobalData->m_maxTerrainTracks = 0;
            TerrainTracksRenderObjClassSystem no_track_edge;
            bool track_edge_rejected = false;
            try { no_track_edge.init(NULL); }
            catch (const std::runtime_error&) { track_edge_rejected = true; }
            require(track_edge_rejected && !TheTerrainTracksRenderObjClassSystem,
                "original zero-track owner initialized without device edge");
            TheWritableGlobalData->m_maxTerrainTracks = saved_no_edge_tracks;
            auto* no_water_edge = NEW_REF(WaterRenderObjClass, ());
            bool water_edge_rejected = false;
            try { no_water_edge->init(0, 0, 0, NULL,
                WaterRenderObjClass::WATER_TYPE_0_TRANSLUCENT); }
            catch (const std::runtime_error&) { water_edge_rejected = true; }
            require(water_edge_rejected && !TheWaterRenderObj,
                "original no-water owner initialized without device edge");
            no_water_edge->Release_Ref();
            W3DSmudgeManager no_smudge_edge;
            bool smudge_edge_rejected = false;
            try { no_smudge_edge.init(); }
            catch (const std::runtime_error&) { smudge_edge_rejected = true; }
            require(smudge_edge_rejected && !TheSmudgeManager,
                "original empty smudge owner initialized without device edge");
            TerrainVisual *saved_visual = TheTerrainVisual;
            auto* no_visual_edge = new W3DTerrainVisual;
            TheTerrainVisual = no_visual_edge;
            bool visual_edge_rejected = false;
            try { no_visual_edge->init(); }
            catch (const std::runtime_error&) { visual_edge_rejected = true; }
            require(visual_edge_rejected && !TheTerrainRenderObject && !TheHeightMap &&
                !TheWaterRenderObj && !TheSmudgeManager,
                "original empty terrain visual initialized without device edge");
            delete no_visual_edge;
            TheTerrainVisual = saved_visual;
        }
        {
            zh::original_runtime::OriginalGpuEdge edge(device);
            W3DView no_owner;
            bool rejected = false;
            try { no_owner.init(); }
            catch (const std::runtime_error&) { rejected = true; }
            require(rejected && !no_owner.get3DCamera(),
                "original W3DView initialized without display owners");
            {
                W3DDisplay display;
                display.init();
                const Bool saved_volumes = TheGlobalData->m_useShadowVolumes;
                const Bool saved_decals = TheGlobalData->m_useShadowDecals;
                TheWritableGlobalData->m_useShadowVolumes = FALSE;
                TheWritableGlobalData->m_useShadowDecals = FALSE;
                const Int saved_track_count = TheGlobalData->m_maxTerrainTracks;
                TheWritableGlobalData->m_maxTerrainTracks = 0;
                TerrainTracksRenderObjClassSystem tracks;
                tracks.init(W3DDisplay::m_3DScene);
                require(TheTerrainTracksRenderObjClassSystem == &tracks &&
                    !tracks.hasPendingGpuResources(),
                    "original zero-track system published GPU resources");
                tracks.init(W3DDisplay::m_3DScene);
                TerrainTracksRenderObjClassSystem duplicate_tracks;
                bool duplicate_track_rejected = false;
                try { duplicate_tracks.init(W3DDisplay::m_3DScene); }
                catch (const std::runtime_error&) { duplicate_track_rejected = true; }
                require(duplicate_track_rejected && TheTerrainTracksRenderObjClassSystem == &tracks,
                    "original zero-track duplicate displaced owner");
                TerrainTracksRenderObjClassSystem wrong_scene_tracks;
                bool wrong_scene_rejected = false;
                try { wrong_scene_tracks.init(NULL); }
                catch (const std::runtime_error&) { wrong_scene_rejected = true; }
                require(wrong_scene_rejected && TheTerrainTracksRenderObjClassSystem == &tracks,
                    "original zero-track owner accepted wrong scene");
                require(!tracks.bindTrack(NULL, 1.0f, ""),
                    "original zero-track owner allocated a module");
                tracks.update();
                tracks.flush();
                tracks.Reset();
                tracks.ReleaseResources();
                tracks.ReAcquireResources();
                require(!tracks.hasPendingGpuResources(),
                    "original zero-track reset acquired GPU resources");
                const Bool saved_water_plane = TheGlobalData->m_useWaterPlane;
                const Bool saved_cloud_plane = TheGlobalData->m_useCloudPlane;
                TheWritableGlobalData->m_useWaterPlane = TRUE;
                TheWritableGlobalData->m_useCloudPlane = FALSE;
                auto* enabled_water = NEW_REF(WaterRenderObjClass, ());
                bool water_mode_rejected = false;
                try { enabled_water->init(0, 0, 0, W3DDisplay::m_3DScene,
                    WaterRenderObjClass::WATER_TYPE_0_TRANSLUCENT); }
                catch (const std::runtime_error&) { water_mode_rejected = true; }
                require(water_mode_rejected && !TheWaterRenderObj,
                    "original enabled water plane initialized without rendering support");
                enabled_water->Release_Ref();
                TheWritableGlobalData->m_useWaterPlane = FALSE;
                auto* wrong_water = NEW_REF(WaterRenderObjClass, ());
                bool wrong_water_scene = false;
                try { wrong_water->init(0, 0, 0, NULL,
                    WaterRenderObjClass::WATER_TYPE_0_TRANSLUCENT); }
                catch (const std::runtime_error&) { wrong_water_scene = true; }
                require(wrong_water_scene && !TheWaterRenderObj,
                    "original no-water owner accepted wrong parent scene");
                wrong_water->Release_Ref();
                auto* water = NEW_REF(WaterRenderObjClass, ());
                require(water->init(0, 0, 0, W3DDisplay::m_3DScene,
                    WaterRenderObjClass::WATER_TYPE_0_TRANSLUCENT) == 0 &&
                    TheWaterRenderObj == water &&
                    water->Class_ID() == RenderObjClass::CLASSID_UNKNOWN,
                    "original no-water owner publication failed");
                require(water->init(0, 0, 0, W3DDisplay::m_3DScene,
                    WaterRenderObjClass::WATER_TYPE_0_TRANSLUCENT) == 0,
                    "original no-water owner re-entry failed");
                W3DDisplay::m_3DScene->Add_Render_Object(water);
                require(water->Peek_Scene() == W3DDisplay::m_3DScene &&
                    water->Num_Refs() == 2,
                    "original disabled-water scene membership lost owner ref");
                auto* duplicate_water = NEW_REF(WaterRenderObjClass, ());
                bool duplicate_water_rejected = false;
                try { duplicate_water->init(0, 0, 0, W3DDisplay::m_3DScene,
                    WaterRenderObjClass::WATER_TYPE_0_TRANSLUCENT); }
                catch (const std::runtime_error&) { duplicate_water_rejected = true; }
                require(duplicate_water_rejected && TheWaterRenderObj == water,
                    "original no-water duplicate displaced owner");
                duplicate_water->Release_Ref();
                bool water_extent_rejected = false;
                try { water->init(0, 1, 0, W3DDisplay::m_3DScene,
                    WaterRenderObjClass::WATER_TYPE_0_TRANSLUCENT); }
                catch (const std::runtime_error&) { water_extent_rejected = true; }
                require(water_extent_rejected && TheWaterRenderObj == water,
                    "original positive water extent silently succeeded");
                bool water_type_rejected = false;
                try { water->init(0, 0, 0, W3DDisplay::m_3DScene,
                    WaterRenderObjClass::WATER_TYPE_1_FB_REFLECTION); }
                catch (const std::runtime_error&) { water_type_rejected = true; }
                require(water_type_rejected && TheWaterRenderObj == water,
                    "original unsupported water type silently succeeded");
                water->setGridHeightClamps(0, 0);
                water->setGridTransform(0, 0, 0, 0);
                water->setGridResolution(0, 0, 0);
                water->setGridChangeAttenuationFactors(0, 0, 0, 0);
                water->reset();
                water->update();
                water->load();
                water->ReleaseResources();
                water->ReAcquireResources();
                bool water_grid_rejected = false;
                try { water->enableWaterGrid(TRUE); }
                catch (const std::runtime_error&) { water_grid_rejected = true; }
                require(water_grid_rejected && water->getWaterHeight(0, 0) == INVALID_WATER_HEIGHT,
                    "original no-water owner accepted grid rendering");
                auto* smudges = new W3DSmudgeManager;
                smudges->init();
                require(TheSmudgeManager == smudges &&
                    smudges->getSmudgeCountLastFrame() == 0 &&
                    !smudges->getHardwareSupport(),
                    "original empty smudge owner publication failed");
                smudges->init();
                W3DSmudgeManager duplicate_smudges;
                bool duplicate_smudge_rejected = false;
                try { duplicate_smudges.init(); }
                catch (const std::runtime_error&) { duplicate_smudge_rejected = true; }
                require(duplicate_smudge_rejected && TheSmudgeManager == smudges,
                    "original empty smudge duplicate displaced owner");
                smudges->reset();
                smudges->ReleaseResources();
                smudges->ReAcquireResources();
                auto* active_smudge_set = smudges->addSmudgeSet();
                bool active_smudge_reset_rejected = false;
                try { smudges->reset(); }
                catch (const std::runtime_error&) { active_smudge_reset_rejected = true; }
                require(active_smudge_reset_rejected && TheSmudgeManager == smudges,
                    "original active smudge set silently reset");
                smudges->removeSmudgeSet(*active_smudge_set);
                smudges->reset();
                TheWritableGlobalData->m_useShadowVolumes = TRUE;
                W3DShadowManager enabled_only;
                bool volume_init_rejected = false;
                try { enabled_only.init(); }
                catch (const std::runtime_error&) { volume_init_rejected = true; }
                require(volume_init_rejected && !TheW3DShadowManager,
                    "original volume-shadow owner initialized without derived manager");
                TheWritableGlobalData->m_useShadowVolumes = FALSE;
                TheWritableGlobalData->m_useShadowDecals = TRUE;
                bool decal_init_rejected = false;
                try { enabled_only.init(); }
                catch (const std::runtime_error&) { decal_init_rejected = true; }
                require(decal_init_rejected && !TheW3DShadowManager,
                    "original decal-shadow owner initialized without derived manager");
                TheWritableGlobalData->m_useShadowDecals = FALSE;
                W3DShadowManager shadows;
                require(shadows.init() && TheW3DShadowManager == &shadows &&
                    shadows.init(), "original disabled-shadow manager owner missing");
                W3DShadowManager duplicate_shadows;
                bool duplicate_shadow_rejected = false;
                try { duplicate_shadows.init(); }
                catch (const std::runtime_error&) { duplicate_shadow_rejected = true; }
                require(duplicate_shadow_rejected && TheW3DShadowManager == &shadows,
                    "original disabled-shadow duplicate displaced owner");
                shadows.queueShadows(TRUE);
                shadows.Reset();
                require(!shadows.isShadowScene() && shadows.ReAcquireResources(),
                    "original disabled-shadow reset/acquire retained render queue");
                shadows.ReleaseResources();
                TheWritableGlobalData->m_useShadowVolumes = TRUE;
                W3DShadowManager enabled_shadows;
                bool enabled_shadow_rejected = false;
                try { enabled_shadows.init(); }
                catch (const std::runtime_error&) { enabled_shadow_rejected = true; }
                require(enabled_shadow_rejected && TheW3DShadowManager == &shadows,
                    "original enabled-shadow path replaced disabled owner");
                bool enabled_reset_rejected = false;
                try { shadows.Reset(); }
                catch (const std::runtime_error&) { enabled_reset_rejected = true; }
                require(enabled_reset_rejected, "original enabled-shadow reset silently succeeded");
                TheWritableGlobalData->m_useShadowVolumes = FALSE;
                TheWritableGlobalData->m_useShadowDecals = TRUE;
                bool decal_shadow_rejected = false;
                try { enabled_shadows.init(); }
                catch (const std::runtime_error&) { decal_shadow_rejected = true; }
                require(decal_shadow_rejected && TheW3DShadowManager == &shadows,
                    "original decal-shadow path replaced disabled owner");
                TheWritableGlobalData->m_useShadowDecals = FALSE;
                auto* terrain = NEW_REF(HeightMapRenderObjClass, ());
                require(TheTerrainRenderObject == terrain && TheHeightMap == terrain &&
                    terrain->getMap() == NULL && !terrain->doesNeedFullUpdate() &&
                    terrain->Class_ID() == RenderObjClass::CLASSID_TILEMAP,
                    "original empty terrain owner publication failed");
                bool duplicate_terrain = false;
                try { auto* duplicate = NEW_REF(HeightMapRenderObjClass, ()); duplicate->Release_Ref(); }
                catch (const std::runtime_error&) { duplicate_terrain = true; }
                require(duplicate_terrain && TheTerrainRenderObject == terrain &&
                    TheHeightMap == terrain, "original empty terrain duplicate displaced owner");
                bool terrain_map_rejected = false;
                try { terrain->initHeightData(1, 1, NULL, NULL); }
                catch (const std::runtime_error&) { terrain_map_rejected = true; }
                require(terrain_map_rejected && !terrain->getMap(),
                    "original empty terrain accepted map data");
                bool height_rejected = false;
                try { (void)terrain->getClipHeight(0, 0); }
                catch (const std::runtime_error&) { height_rejected = true; }
                require(height_rejected, "original empty terrain sampled without map");
                terrain->updateCenter(NULL, NULL);
                terrain->reset();
                require(!terrain->doesNeedFullUpdate(),
                    "original empty terrain reset requested a map update");
                auto* view = new W3DView;
                view->init();
                auto* camera = view->get3DCamera();
                require(camera && camera->Num_Refs() == 1,
                    "original W3DView 3D camera owner missing");
                RenderInfoClass terrain_info(*camera);
                bool terrain_render_rejected = false;
                try { terrain->Render(terrain_info); }
                catch (const std::runtime_error&) { terrain_render_rejected = true; }
                require(terrain_render_rejected && !WW3D::Is_Rendering(),
                    "original empty terrain emitted a draw");
                bool water_render_rejected = false;
                try { water->Render(terrain_info); }
                catch (const std::runtime_error&) { water_render_rejected = true; }
                require(water_render_rejected && !WW3D::Is_Rendering(),
                    "original no-water owner emitted a draw");
                smudges->render(terrain_info);
                active_smudge_set = smudges->addSmudgeSet();
                bool active_smudge_render_rejected = false;
                try { smudges->render(terrain_info); }
                catch (const std::runtime_error&) { active_smudge_render_rejected = true; }
                require(active_smudge_render_rejected && !WW3D::Is_Rendering(),
                    "original active smudge set silently rendered");
                smudges->removeSmudgeSet(*active_smudge_set);
                smudges->reset();
                view->init();
                require(view->get3DCamera() == camera && camera->Num_Refs() == 1,
                    "original W3DView re-entry replaced 3D camera");
                camera->Set_Position(Vector3(0, 0, 1));
                camera->Set_View_Plane(Vector2(-1, -0.75f), Vector2(1, 0.75f));
                camera->Set_Clip_Planes(0.995f, 2.0f);
                display.attachView(view);
                require(display.getFirstView() == view,
                    "original display did not own its source view");
                auto* recorder = dynamic_cast<zh::renderer::RecordingGpuDevice*>(&device);
                auto frame = [&]() {
                    edge.bind_frame_targets(color, depth, width, target.height);
                    require(WW3D::Begin_Render(true, true, Vector3(0.2f, 0.4f, 0.6f), 1) ==
                        WW3D_ERROR_OK, "original view frame did not begin");
                    DX8Wrapper::Set_Transform(D3DTS_WORLD, Matrix3D(true));
                    display.drawViews();
                    require(WW3D::End_Render(false) == WW3D_ERROR_OK,
                        "original view frame did not end");
                    return readback(color);
                };
                if (recorder) {
                    TheWritableGlobalData->m_useWaterPlane = TRUE;
                    bool active_water_scene_rejected = false;
                    try { (void)frame(); }
                    catch (const std::runtime_error&) { active_water_scene_rejected = true; }
                    TheWritableGlobalData->m_useWaterPlane = FALSE;
                    require(active_water_scene_rejected && water->Num_Refs() == 2 &&
                        !WW3D::Is_Rendering() && !recorder->pass_active(),
                        "original enabled-water scene silently rendered or lost owner");
                    shadows.queueShadows(TRUE);
                    bool queued_shadow_rejected = false;
                    try { (void)frame(); }
                    catch (const std::runtime_error&) { queued_shadow_rejected = true; }
                    require(queued_shadow_rejected && !WW3D::Is_Rendering() &&
                        !recorder->pass_active(),
                        "original queued shadow escaped no-shadow scene guard");
                    shadows.Reset();
                }
                const auto absent_marker = recorder ? recorder->snapshot().size() : 0;
                const auto absent = frame();
                empty_baseline = absent;
                if (recorder)
                    require(recorder->snapshot().substr(absent_marker).find("draw pipeline=") ==
                        std::string::npos, "original empty W3DView frame drew");
                RAMFileClass packet(bytes.data(), static_cast<int>(bytes.size()));
                auto* assets = static_cast<WW3DAssetManager*>(W3DDisplay::m_assetManager);
                require(assets->Load_3D_Assets(packet), "original view W3D packet rejected");
                auto* mesh = W3DDisplay::m_assetManager->Create_Render_Obj("TEST.ZERO01");
                require(mesh && mesh->Num_Refs() == 1, "original view mesh absent");
                W3DDisplay::m_3DScene->Add_Render_Object(mesh);
                require(mesh->Num_Refs() == 2, "original view scene ref missing");
                const auto present_marker = recorder ? recorder->snapshot().size() : 0;
                const auto present = frame();
                if (physical) {
                    require(absent.size() == std::size_t(width) * target.height * 4 &&
                        present.size() == absent.size(), "original view readback extent drift");
                    unsigned changed = 0;
                    for (std::size_t i = 0; i < present.size(); i += 4)
                        changed += present[i] != absent[i] || present[i+1] != absent[i+1] ||
                            present[i+2] != absent[i+2];
                    require(changed > 0, "original view rigid object left no physical pixels");
                } else {
                    require(recorder->snapshot().substr(present_marker).find("draw pipeline=") !=
                        std::string::npos, "original W3DView source path omitted rigid draw");
                    auto* extra_mesh = W3DDisplay::m_assetManager->Create_Render_Obj("TEST.ZERO01");
                    require(extra_mesh && extra_mesh->Num_Refs() == 1,
                        "original disabled-water second-mesh control absent");
                    W3DDisplay::m_3DScene->Add_Render_Object(extra_mesh);
                    bool extra_rigid_rejected = false;
                    try { (void)frame(); }
                    catch (const std::runtime_error&) { extra_rigid_rejected = true; }
                    W3DDisplay::m_3DScene->Remove_Render_Object(extra_mesh);
                    require(extra_rigid_rejected && extra_mesh->Num_Refs() == 1 &&
                        !WW3D::Is_Rendering() && !recorder->pass_active(),
                        "original disabled-water exemption admitted second rigid object");
                    extra_mesh->Release_Ref();
                    recorder->fail_next_draw();
                    bool rejected_draw = false;
                    try { (void)frame(); }
                    catch (const std::runtime_error&) { rejected_draw = true; }
                    require(rejected_draw && !WW3D::Is_Rendering() && !recorder->pass_active() &&
                        mesh->Num_Refs() == 2, "original W3DView draw failure retained frame");
                    (void)frame();
                    device.destroy(depth);
                    bool stale = false;
                    try { (void)frame(); }
                    catch (const std::runtime_error&) { stale = true; }
                    require(stale && !WW3D::Is_Rendering() && !recorder->pass_active(),
                        "original W3DView accepted a stale depth target");
                    depth = device.create_texture(target, "rebound original W3DView depth");
                    require(static_cast<bool>(depth), "original W3DView depth rebind failed");
                    (void)frame();
                }
                W3DDisplay::m_3DScene->Remove_Render_Object(mesh);
                require(mesh->Num_Refs() == 1, "original W3DView detach retained scene ref");
                mesh->Release_Ref();
                const auto detached = frame();
                if (physical) require(detached == absent,
                    "original W3DView detach retained physical pixels");
                bool advanced = false;
                try { display.updateViews(); }
                catch (const std::runtime_error&) { advanced = true; }
                require(advanced, "original W3DView tactical update unexpectedly succeeded");
                bool wireframe = false;
                try { view->set3DWireFrameMode(TRUE); }
                catch (const std::runtime_error&) { wireframe = true; }
                require(wireframe, "original W3DView wireframe mode unexpectedly succeeded");
                bool filter = false;
                try { (void)view->setViewFilter(FT_VIEW_DEFAULT); }
                catch (const std::runtime_error&) { filter = true; }
                require(filter, "original W3DView filter setter unexpectedly succeeded");
                terrain->Release_Ref();
                require(!TheTerrainRenderObject && !TheHeightMap,
                    "original empty terrain teardown retained singleton");
                W3DDisplay::m_3DScene->Remove_Render_Object(water);
                require(!water->Peek_Scene() && water->Num_Refs() == 1,
                    "original disabled-water scene detach retained owner ref");
                water->Release_Ref();
                require(!TheWaterRenderObj,
                    "original no-water teardown retained singleton");
                delete smudges;
                require(!TheSmudgeManager,
                    "original empty smudge teardown retained singleton");
                shadows.Reset();
                TheWritableGlobalData->m_useShadowVolumes = saved_volumes;
                TheWritableGlobalData->m_useShadowDecals = saved_decals;
                TheWritableGlobalData->m_maxTerrainTracks = saved_track_count;
                TheWritableGlobalData->m_useWaterPlane = saved_water_plane;
                TheWritableGlobalData->m_useCloudPlane = saved_cloud_plane;
            }
            require(!W3DDisplay::m_3DScene && !WW3D::Is_Initted() &&
                !TheW3DShadowManager && !TheTerrainTracksRenderObjClassSystem &&
                !TheWaterRenderObj && !TheSmudgeManager,
                "original W3DView display teardown retained source owners");
            bool stale_terrain_owner = false;
            try { auto* terrain = NEW_REF(HeightMapRenderObjClass, ()); terrain->Release_Ref(); }
            catch (const std::runtime_error&) { stale_terrain_owner = true; }
            require(stale_terrain_owner && !TheTerrainRenderObject && !TheHeightMap,
                "original empty terrain accepted torn-down display owners");
            const Bool stale_volumes = TheGlobalData->m_useShadowVolumes;
            const Bool stale_decals = TheGlobalData->m_useShadowDecals;
            TheWritableGlobalData->m_useShadowVolumes = FALSE;
            TheWritableGlobalData->m_useShadowDecals = FALSE;
            W3DShadowManager stale_shadows;
            bool stale_shadow_owner = false;
            try { stale_shadows.init(); }
            catch (const std::runtime_error&) { stale_shadow_owner = true; }
            require(stale_shadow_owner && !TheW3DShadowManager,
                "original disabled-shadow owner accepted torn-down display");
            TheWritableGlobalData->m_useShadowVolumes = stale_volumes;
            TheWritableGlobalData->m_useShadowDecals = stale_decals;
            const Int stale_track_count = TheGlobalData->m_maxTerrainTracks;
            TheWritableGlobalData->m_maxTerrainTracks = 0;
            TerrainTracksRenderObjClassSystem stale_tracks;
            bool stale_track_owner = false;
            try { stale_tracks.init(NULL); }
            catch (const std::runtime_error&) { stale_track_owner = true; }
            require(stale_track_owner && !TheTerrainTracksRenderObjClassSystem,
                "original zero-track owner accepted torn-down display");
            TheWritableGlobalData->m_maxTerrainTracks = stale_track_count;
            const Bool stale_water_plane = TheGlobalData->m_useWaterPlane;
            const Bool stale_cloud_plane = TheGlobalData->m_useCloudPlane;
            TheWritableGlobalData->m_useWaterPlane = FALSE;
            TheWritableGlobalData->m_useCloudPlane = FALSE;
            auto* stale_water = NEW_REF(WaterRenderObjClass, ());
            bool stale_water_owner = false;
            try { stale_water->init(0, 0, 0, NULL,
                WaterRenderObjClass::WATER_TYPE_0_TRANSLUCENT); }
            catch (const std::runtime_error&) { stale_water_owner = true; }
            require(stale_water_owner && !TheWaterRenderObj,
                "original no-water owner accepted torn-down display");
            stale_water->Release_Ref();
            TheWritableGlobalData->m_useWaterPlane = stale_water_plane;
            TheWritableGlobalData->m_useCloudPlane = stale_cloud_plane;
            W3DSmudgeManager stale_smudges;
            bool stale_smudge_owner = false;
            try { stale_smudges.init(); }
            catch (const std::runtime_error&) { stale_smudge_owner = true; }
            require(stale_smudge_owner && !TheSmudgeManager,
                "original empty smudge owner accepted torn-down display");
            {
                W3DDisplay composed_display;
                composed_display.init();
                TerrainVisual *saved_visual = TheTerrainVisual;
                auto* composed = new W3DTerrainVisual;
                TheTerrainVisual = composed;
                const Bool visual_saved_water = TheGlobalData->m_useWaterPlane;
                TheWritableGlobalData->m_useWaterPlane = TRUE;
                bool enabled_visual_rejected = false;
                try { composed->init(); }
                catch (const std::runtime_error&) { enabled_visual_rejected = true; }
                TheWritableGlobalData->m_useWaterPlane = visual_saved_water;
                require(enabled_visual_rejected && TheTerrainVisual == composed &&
                    !TheTerrainRenderObject && !TheHeightMap &&
                    !TheTerrainTracksRenderObjClassSystem && !TheW3DShadowManager &&
                    !TheWaterRenderObj && !TheSmudgeManager,
                    "original enabled-water visual published partial owners");
                composed->init();
                require(TheTerrainVisual == composed && TheTerrainRenderObject && TheHeightMap &&
                    !TheHeightMap->getMap() && TheTerrainTracksRenderObjClassSystem &&
                    TheW3DShadowManager && TheWaterRenderObj && TheSmudgeManager &&
                    TheWaterRenderObj->Peek_Scene() == W3DDisplay::m_3DScene &&
                    TheWaterRenderObj->Num_Refs() == 2,
                    "original empty terrain visual did not compose native owners");
                composed->init();
                composed->reset();
                composed->update();
                W3DTerrainVisual duplicate_visual;
                bool duplicate_visual_rejected = false;
                try { duplicate_visual.init(); }
                catch (const std::runtime_error&) { duplicate_visual_rejected = true; }
                require(duplicate_visual_rejected && TheTerrainVisual == composed &&
                    TheWaterRenderObj->Num_Refs() == 2,
                    "original empty terrain visual duplicate displaced owner");
                bool map_visual_rejected = false;
                try { (void)composed->load(AsciiString("unsupported.map")); }
                catch (const std::runtime_error&) { map_visual_rejected = true; }
                require(map_visual_rejected && !TheHeightMap->getMap(),
                    "original empty terrain visual accepted map load");
                auto* composed_view = new W3DView;
                composed_view->init();
                auto* composed_camera = composed_view->get3DCamera();
                composed_camera->Set_Position(Vector3(0, 0, 1));
                composed_camera->Set_View_Plane(Vector2(-1, -0.75f), Vector2(1, 0.75f));
                composed_camera->Set_Clip_Planes(0.995f, 2.0f);
                composed_display.attachView(composed_view);
                auto* composed_recorder = dynamic_cast<zh::renderer::RecordingGpuDevice*>(&device);
                const auto composed_marker = composed_recorder ? composed_recorder->snapshot().size() : 0;
                edge.bind_frame_targets(color, depth, width, target.height);
                require(WW3D::Begin_Render(true, true, Vector3(0.2f, 0.4f, 0.6f), 1) ==
                    WW3D_ERROR_OK, "original composed terrain visual frame did not begin");
                DX8Wrapper::Set_Transform(D3DTS_WORLD, Matrix3D(true));
                composed_display.drawViews();
                require(WW3D::End_Render(false) == WW3D_ERROR_OK,
                    "original composed terrain visual frame did not end");
                const auto composed_pixels = readback(color);
                if (physical)
                    require(composed_pixels == empty_baseline,
                        "original composed empty terrain visual changed physical pixels");
                else
                    require(composed_recorder->snapshot().substr(composed_marker).find("draw pipeline=") ==
                        std::string::npos, "original composed empty terrain visual emitted a draw");
                delete composed;
                require(!TheTerrainVisual && !TheTerrainRenderObject && !TheHeightMap &&
                    !TheTerrainTracksRenderObjClassSystem && !TheW3DShadowManager &&
                    !TheWaterRenderObj && !TheSmudgeManager,
                    "original composed terrain visual teardown retained owners");
                TheTerrainVisual = saved_visual;
            }
            require(!W3DDisplay::m_3DScene && !WW3D::Is_Initted(),
                "original composed terrain visual display teardown retained WW3D");
        }
        device.destroy(depth);
        device.destroy(color);
        if (physical) {
            auto* bgfx = dynamic_cast<zh::renderer::BgfxGpuDevice*>(&device);
            require(bgfx && bgfx->live_resource_count() == 0,
                "original W3DView bgfx teardown retained resources");
        } else {
            auto* recorder = dynamic_cast<zh::renderer::RecordingGpuDevice*>(&device);
            require(recorder && recorder->resource_counts().total() == 0,
                "original W3DView Recording teardown retained resources");
        }
    };
    if (physical) {
        for (unsigned width : {160U, 200U})
            for (auto format : {zh::renderer::TextureFormat::bgra8,
                zh::renderer::TextureFormat::rgba8}) {
                zh::renderer::BgfxOptions options;
                options.shader_root = ZH_BGFX_SHADER_DIR;
                zh::renderer::BgfxGpuDevice device(options);
                run(device, width, format,
                    [&](auto color) { return device.readback_rgba(color); });
                require(device.wait_idle(), "original W3DView bgfx idle failed");
            }
    } else {
        zh::renderer::RecordingGpuDevice device;
        run(device, 160, zh::renderer::TextureFormat::bgra8,
            [](auto) { return std::vector<unsigned char>{}; });
    }
    std::puts(physical ? "original W3DView frame: physical generations=4 resources=0" :
        "original W3DView frame: source draw=1 resources=0");
}
