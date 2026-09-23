#include "PreRTS.h"
#include "OriginalW3DDeviceUnavailable.h"
#include "Common/GameMemory.h"
#include "Common/GlobalData.h"
#include "W3DDevice/GameClient/W3DAssetManager.h"
#include "W3DDevice/GameClient/W3DDisplay.h"
#include "W3DDevice/GameClient/W3DScene.h"
#include "W3DDevice/GameClient/W3DStatusCircle.h"
#include "W3DDevice/GameClient/W3DShadow.h"
#include "W3DDevice/GameClient/W3DCustomScene.h"
#include "W3DDevice/GameClient/W3DShroud.h"
#include "W3DDevice/GameClient/W3DTerrainTracks.h"
#include "GameLogic/TerrainLogic.h"
#include "WW3D2/nullrobj.h"
#include "WW3D2/vertmaterial.h"
#include "WW3D2/DX8Wrapper.h"
#include "WW3D2/ww3d.h"
#include "WW3D2/camera.h"
#include "mempool.h"
#include "multilist.h"
#include "original_gpu_edge.h"
#include "zh/renderer/recording_device.h"

#include <iostream>
#include <cstdlib>
#include <filesystem>
#include <stdexcept>
#include <string>
#include <vector>

struct PoolProbeNode { void *next; std::uint64_t originalPayload; };
struct PoolProbe : ObjectPoolClass<PoolProbeNode, 7> {
    int freeCount() const { return FreeObjectCount; }
    int totalCount() const { return TotalObjectCount; }
};
struct Inspect2DScene : RTS2DScene {
    RenderObjClass *status() const { return m_status; }
};
struct WritableTestDirectory {
    char path[sizeof("/tmp/zh-m22-presentation-XXXXXX")] = "/tmp/zh-m22-presentation-XXXXXX";
    WritableTestDirectory()
    {
        if (!mkdtemp(path)) throw std::runtime_error("cannot create presentation test directory");
        setenv("XDG_DATA_HOME", path, 1);
        setenv("XDG_STATE_HOME", path, 1);
    }
    ~WritableTestDirectory()
    {
        std::error_code ignored;
        std::filesystem::remove_all(path, ignored);
    }
};

static void require(bool ok, const char *message)
{
    if (!ok) throw std::runtime_error(message);
}

int main()
{
    try {
        {
            PoolProbe pool;
            std::vector<PoolProbeNode *> nodes;
            for (int i = 0; i != 22; ++i) nodes.push_back(pool.Allocate_Object());
            require(pool.totalCount() == 28 && pool.freeCount() == 6,
                "original x86_64 WWLib pool allocation counts mismatch");
            require(!pool.Release_Empty_Blocks(),
                "active original WWLib pool retired a live node");
            require(pool.totalCount() == 28 && pool.freeCount() == 6,
                "active original WWLib pool changed on retirement refusal");
            for (auto *node : nodes) pool.Free_Object(node);
            require(pool.freeCount() == pool.totalCount(),
                "original WWLib pool teardown retained allocated nodes");
            require(pool.Release_Empty_Blocks() && pool.totalCount() == 0 && pool.freeCount() == 0,
                "empty original WWLib pool did not retire its blocks");
            require(pool.Release_Empty_Blocks(), "empty original WWLib pool retirement was not idempotent");
            auto *second_generation = pool.Allocate_Object();
            require(second_generation && pool.totalCount() == 7 && pool.freeCount() == 6,
                "original WWLib pool failed after retirement");
            pool.Free_Object(second_generation);
            require(pool.Release_Empty_Blocks() && pool.totalCount() == 0,
                "second-generation original WWLib pool blocks survived retirement");
        }
        WritableTestDirectory writable;
        initMemoryManager();
        auto *live_multilist_node = new MultiListNodeClass;
        require(!MultiListNodeClass::Release_Empty_Blocks(),
            "original multilist cache retired an active node");
        require(live_multilist_node->Next == nullptr && live_multilist_node->Object == nullptr,
            "original multilist node was damaged by refused retirement");
        delete live_multilist_node;
        require(MultiListNodeClass::Release_Empty_Blocks(),
            "original multilist cache retained its empty slab");
        TheWritableGlobalData = new GlobalData;
        TheWritableGlobalData->m_terrainLightPos[0].z = 1;
        TheWritableGlobalData->m_clearAlpha = 255;
        TheWritableGlobalData->m_maxTerrainTracks = 1;
        TheWritableGlobalData->m_maxTankTrackEdges = 8;
        TheWritableGlobalData->m_maxTankTrackOpaqueEdges = 4;
        TheWritableGlobalData->m_maxTankTrackFadeDelay = 1;
        require(DX8Wrapper::Convert_Color_Clamp(Vector4(0.5f, 0.501f, 1.5f, -0.2f))
                == 0x007f7fff,
            "original DX8 ARGB clamp/truncate semantics drifted");
        const Vector4 decoded = DX8Wrapper::Convert_Color(0x80a02010u);
        require(decoded[0] == 160.0f / 255.0f && decoded[1] == 32.0f / 255.0f
                && decoded[2] == 16.0f / 255.0f && decoded[3] == 128.0f / 255.0f,
            "original DX8 ARGB component order drifted");

        // The original display's first physical 2D creation rejects before
        // it could publish a 3D scene or asset manager; retry below owns both.
        try {
            RTS2DScene statusScene;
            throw std::runtime_error("original 2D device creation silently succeeded");
        } catch (const OriginalW3DDeviceUnavailable &error) {
            require(std::string(error.what()).find("status-circle GPU") != std::string::npos,
                "original 2D scene selected wrong device edge");
        }
        require(W3DDisplay::m_3DScene == nullptr && W3DDisplay::m_assetManager == nullptr,
            "failed 2D setup published original 3D scene owners");

        for (int generation = 0; generation != 2; ++generation) {
            zh::renderer::RecordingGpuDevice device;
            {
                zh::original_runtime::OriginalGpuEdge edge(device);
                Inspect2DScene statusScene;
                require(statusScene.status() != nullptr &&
                    dynamic_cast<W3DStatusCircle *>(statusScene.status()) != nullptr &&
                    statusScene.status()->Get_Scene() == &statusScene &&
                    statusScene.status()->Num_Refs() == 2,
                    "original 2D status-circle owner was not attached exactly once");
                try {
                    statusScene.draw();
                    throw std::runtime_error("original 2D draw accepted a missing camera");
                } catch (const OriginalW3DDeviceUnavailable &error) {
                    require(std::string(error.what()).find("camera missing") != std::string::npos,
                        "original 2D scene selected wrong missing-camera guard");
                }
            }
            require(device.resource_counts().total() == 0,
                "original 2D owner teardown retained device resources");
        }
        require(W3DDisplay::m_2DScene == nullptr && W3DDisplay::m_3DScene == nullptr &&
            W3DDisplay::m_assetManager == nullptr,
            "owned 2D fixture published production display slots");

        // This is test-only publication in the original display's ownership
        // slots. Production init() still stops at the earlier 2D device edge.
        auto *assets = new W3DAssetManager;
        auto *scene = NEW_REF(RTS3DScene, ());
        W3DDisplay::m_assetManager = assets;
        W3DDisplay::m_3DScene = scene;
        require(W3DDisplay::m_assetManager == assets &&
            W3DDisplay::m_3DScene == scene, "original display ownership missing");

        auto *renderObject = NEW_REF(Null3DObjClass, ("M22OwnedSceneObject"));
        scene->Add_Render_Object(renderObject);
        require(renderObject->Get_Scene() == scene, "original scene attach missing");
        scene->Remove_Render_Object(renderObject);
        require(renderObject->Get_Scene() == nullptr, "original scene detach missing");
        try {
            scene->draw();
            throw std::runtime_error("original 3D draw accepted a missing GPU edge");
        } catch (const OriginalW3DDeviceUnavailable &error) {
            require(std::string(error.what()).find("GPU session missing") != std::string::npos,
                "original 3D scene selected wrong missing-edge guard");
        }
        {
            zh::renderer::RecordingGpuDevice device;
            zh::renderer::TextureDesc target;
            target.width = 160; target.height = 120; target.render_target = true;
            target.format = zh::renderer::TextureFormat::bgra8;
            const auto color = device.create_texture(target, "original empty 3D color");
            target.format = zh::renderer::TextureFormat::depth24_stencil8;
            auto depth = device.create_texture(target, "original empty 3D depth");
            require(color && depth, "original empty 3D frame targets missing");
            {
                zh::original_runtime::OriginalGpuEdge edge(device);
                require(WW3D::Init(nullptr, nullptr, false) == WW3D_ERROR_OK,
                    "original empty 3D WW3D init failed");
                try {
                    scene->draw();
                    throw std::runtime_error("original 3D draw accepted a missing camera");
                } catch (const OriginalW3DDeviceUnavailable &error) {
                    require(std::string(error.what()).find("camera missing") != std::string::npos,
                        "original 3D scene selected wrong missing-camera guard");
                }
                CameraClass camera;
                camera.Set_Position(Vector3(0, 0, 1));
                camera.Set_View_Plane(Vector2(-1, -0.75f), Vector2(1, 0.75f));
                camera.Set_Clip_Planes(0.995f, 2.0f);
                auto frame = [&] {
                    edge.bind_frame_targets(color, depth, 160, 120);
                    require(WW3D::Begin_Render(true, true, Vector3(0, 0, 0), 1) == WW3D_ERROR_OK,
                        "original empty 3D frame did not begin");
                    DX8Wrapper::Set_Transform(D3DTS_WORLD, Matrix3D(true));
                    scene->doRender(&camera);
                    require(WW3D::End_Render(false) == WW3D_ERROR_OK,
                        "original empty 3D frame did not end");
                };
                const Bool old_markers = TheWritableGlobalData->m_enableBehindBuildingMarkers;
                TheWritableGlobalData->m_enableBehindBuildingMarkers = TRUE;
                frame();
                require(TheWritableGlobalData->m_enableBehindBuildingMarkers,
                    "original 3D D24S8 lost stencil-backed markers");
                require(!device.pass_active() &&
                    device.snapshot().find("draw pipeline=") == std::string::npos,
                    "original empty 3D scene submitted a draw");
                scene->setCustomPassMode(SCENE_PASS_ALPHA_MASK);
                bool unsupported = false;
                try { frame(); }
                catch (const OriginalW3DDeviceUnavailable&) { unsupported = true; }
                require(unsupported && !WW3D::Is_Rendering() && !device.pass_active(),
                    "original 3D custom pass did not abort source frame");
                scene->setCustomPassMode(SCENE_PASS_DEFAULT);
                frame();
                scene->Add_Render_Object(renderObject);
                bool populated = false;
                try { frame(); }
                catch (const OriginalW3DDeviceUnavailable&) { populated = true; }
                require(populated && !WW3D::Is_Rendering() && !device.pass_active(),
                    "original populated 3D scene bypassed narrow source gate");
                scene->Remove_Render_Object(renderObject);
                frame();
                device.destroy(depth);
                bool stale = false;
                try { frame(); }
                catch (const std::runtime_error&) { stale = true; }
                require(stale && !WW3D::Is_Rendering(),
                    "original empty 3D stale depth target was accepted");
                depth = device.create_texture(target, "rebound empty 3D depth");
                require(static_cast<bool>(depth), "original empty 3D rebound depth missing");
                frame();
                device.destroy(depth);
                target.format = zh::renderer::TextureFormat::depth16;
                depth = device.create_texture(target, "original empty 3D stencil-free depth");
                require(static_cast<bool>(depth), "original empty 3D stencil-free depth missing");
                TheWritableGlobalData->m_enableBehindBuildingMarkers = TRUE;
                frame();
                require(!TheWritableGlobalData->m_enableBehindBuildingMarkers,
                    "original 3D stencil-free frame retained behind-building markers");
                TheWritableGlobalData->m_enableBehindBuildingMarkers = old_markers;
                require(WW3D::Shutdown() == WW3D_ERROR_OK,
                    "original empty 3D WW3D shutdown failed");
            }
            device.destroy(depth); device.destroy(color);
            require(device.resource_counts().total() == 0,
                "original empty 3D teardown retained device resources");
        }
        require(!zh::original_runtime::OriginalGpuEdge::active(),
            "original shroud material retained a stale fixture edge");
        W3DShroudMaterialPassClass shroudPass;
        try {
            shroudPass.Install_Materials();
            throw std::runtime_error("original shroud material silently installed without GPU");
        } catch (const OriginalW3DDeviceUnavailable &error) {
            require(std::string(error.what()).find("shroud material GPU") != std::string::npos,
                "original shroud material selected wrong device edge");
        }

        {
        TerrainLogic terrain;
        TheTerrainLogic = &terrain;
        VertexMaterialClass::Init();
        TerrainTracksRenderObjClassSystem tracks;
        TheTerrainTracksRenderObjClassSystem = &tracks;
        tracks.init(scene);
        require(tracks.hasPendingGpuResources(), "original track resource edge missing");
        auto *track = tracks.bindTrack(renderObject, 1.0f, "");
        require(track && track->Get_Scene() == scene,
            "original track binding did not attach to original scene");
        track->addEdgeToTrack(0.0f, 0.0f);
        track->addEdgeToTrack(3.0f, 0.0f);
        track->addCapEdgeToTrack(6.0f, 0.0f);
        try {
            tracks.flush();
            throw std::runtime_error("original active track silently flushed without GPU");
        } catch (const OriginalW3DDeviceUnavailable &error) {
            require(std::string(error.what()).find("terrain track GPU") != std::string::npos,
                "original track selected wrong device edge");
        }
        tracks.unbindTrack(track);
        tracks.Reset();
        require(track->Get_Scene() == nullptr, "original track reset leaked scene owner");
        tracks.shutdown();
        TheTerrainTracksRenderObjClassSystem = nullptr;
        VertexMaterialClass::Shutdown();
        TheTerrainLogic = nullptr;
        }
        renderObject->Release_Ref();

        W3DShadowManager shadows;
        require(shadows.getShadowColor() == 0x7fa0a0a0,
            "original shadow default color missing");
        shadows.queueShadows(TRUE);
        require(shadows.isShadowScene(), "original shadow scene queue missing");
        const struct Route { ShadowType type; const char *diagnostic; } routes[] = {
            { SHADOW_VOLUME, "volumetric shadow" },
            { SHADOW_PROJECTION, "projected shadow" },
            { SHADOW_DECAL, "decal shadow" },
        };
        for (const auto &route : routes) {
            Shadow::ShadowTypeInfo info{};
            info.m_type = route.type;
            try {
                (void)shadows.addShadow(nullptr, &info);
                throw std::runtime_error("shadow route silently succeeded");
            } catch (const OriginalW3DDeviceUnavailable &error) {
                require(std::string(error.what()).find(route.diagnostic) != std::string::npos,
                    "original shadow selected wrong route");
            }
        }

        W3DDisplay::m_3DScene = nullptr;
        W3DDisplay::m_assetManager = nullptr;
        scene->Release_Ref();
        assets->Free_Assets();
        delete assets;
        delete TheWritableGlobalData;
        TheWritableGlobalData = nullptr;
        std::cout << "original-rendering runtime provider=GeneralsMD GameClient CPU presentation\n";
        return 0;
    } catch (const std::exception &error) {
        std::cerr << "original W3D presentation: " << error.what() << '\n';
        return 1;
    }
}
