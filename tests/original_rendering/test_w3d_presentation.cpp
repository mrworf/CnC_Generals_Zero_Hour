#include "PreRTS.h"
#include "OriginalW3DDeviceUnavailable.h"
#include "Common/GameMemory.h"
#include "Common/GlobalData.h"
#include "W3DDevice/GameClient/W3DAssetManager.h"
#include "W3DDevice/GameClient/W3DDisplay.h"
#include "W3DDevice/GameClient/W3DScene.h"
#include "W3DDevice/GameClient/W3DShadow.h"
#include "W3DDevice/GameClient/W3DShroud.h"
#include "W3DDevice/GameClient/W3DTerrainTracks.h"
#include "GameLogic/TerrainLogic.h"
#include "WW3D2/nullrobj.h"
#include "WW3D2/vertmaterial.h"
#include "WW3D2/DX8Wrapper.h"
#include "mempool.h"

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
            for (auto *node : nodes) pool.Free_Object(node);
            require(pool.freeCount() == pool.totalCount(),
                "original WWLib pool teardown retained allocated nodes");
        }
        WritableTestDirectory writable;
        initMemoryManager();
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
