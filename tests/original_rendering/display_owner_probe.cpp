#include "PreRTS.h"
#include "GameLogic/GameLogic.h"
#include "W3DDevice/GameClient/W3DDisplay.h"
#include "W3DDevice/GameClient/W3DScene.h"
#include "W3DDevice/GameClient/W3DAssetManager.h"
#include "Common/FileSystem.h"
#include "WW3D2/assetmgr.h"
#include "WW3D2/mesh.h"
#include "WW3D2/ww3d.h"
#include "WWLib/RAMFILE.H"
#include "original_gpu_edge.h"
#include "zh/platform/bgfx_device.h"
#include "zh/renderer/recording_device.h"

#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <iterator>
#include <memory>
#include <stdexcept>
#include <string>
#include <vector>

namespace {
void require(bool value, const char* message)
{
    if (!value) throw std::runtime_error(message);
}
}

extern "C" void zh_probe_display_owner()
{
    require(TheGameLogic && TheGameLogic->isInGame(),
        "original display owner requires initialized GameClient scenario");
    require(!W3DDisplay::m_3DScene && !W3DDisplay::m_2DScene &&
        !W3DDisplay::m_3DInterfaceScene && !W3DDisplay::m_assetManager,
        "original display owner slots already occupied");
    const char* packet_path = std::getenv("ZH_M22_DISPLAY_OWNER_ASSET");
    require(packet_path, "original display owner packet path missing");
    std::ifstream input(packet_path, std::ios::binary);
    require(input.good(), "original display owner packet unreadable");
    std::vector<char> bytes(std::istreambuf_iterator<char>{input}, std::istreambuf_iterator<char>{});
    require(!bytes.empty(), "original display owner packet empty");
    const bool physical = std::getenv("ZH_M22_DISPLAY_OWNER_PHYSICAL") != nullptr;
    auto run = [&](zh::renderer::GpuDevice& device) {
        auto display = std::make_unique<W3DDisplay>();
        bool no_edge = false;
        try { display->init(); }
        catch (const std::runtime_error& error) {
            no_edge = std::string(error.what()).find("edge") != std::string::npos;
        }
        require(no_edge && !W3DDisplay::m_3DScene && !W3DDisplay::m_assetManager,
            "original display missing-edge init published owners");
        bool no_reset = false;
        try { display->reset(); }
        catch (const std::runtime_error&) { no_reset = true; }
        require(no_reset, "original display reset before bootstrap succeeded");
        display->doSmartAssetPurgeAndPreload("Maps\\Owned\\AssetUsage.txt");
        {
            zh::original_runtime::OriginalGpuEdge edge(device);
            display->init();
            auto* scene3d = W3DDisplay::m_3DScene;
            auto* scene2d = W3DDisplay::m_2DScene;
            auto* interface_scene = W3DDisplay::m_3DInterfaceScene;
            auto* assets = W3DDisplay::m_assetManager;
            require(scene3d && scene2d && interface_scene && assets && WW3D::Is_Initted(),
                "original display init omitted source owners");
            display->init();
            require(W3DDisplay::m_3DScene == scene3d && W3DDisplay::m_2DScene == scene2d &&
                W3DDisplay::m_3DInterfaceScene == interface_scene &&
                W3DDisplay::m_assetManager == assets,
                "original display re-entry replaced source owners");
            bool duplicate = false;
            try { W3DDisplay second; }
            catch (const std::runtime_error&) { duplicate = true; }
            require(duplicate && W3DDisplay::m_3DScene == scene3d,
                "original display duplicate owner construction changed slots");
            bool draw_pending = false;
            try { display->draw(); }
            catch (const std::runtime_error&) { draw_pending = true; }
            require(draw_pending, "original display draw unexpectedly succeeded");
            auto load_packet = [&] {
                RAMFileClass packet(bytes.data(), static_cast<int>(bytes.size()));
                require(static_cast<WW3DAssetManager*>(assets)->Load_3D_Assets(packet),
                    "original display asset manager rejected generated packet");
            };
            auto has_prototype = [&](const char* name) {
                return assets->Find_Prototype(name) != nullptr;
            };
            FileSystem *saved_file_system = TheFileSystem;
            TheFileSystem = nullptr;
            bool missing_provider = false;
            try { display->doSmartAssetPurgeAndPreload("Maps\\Owned\\AssetUsage.txt"); }
            catch (const std::runtime_error&) { missing_provider = true; }
            TheFileSystem = saved_file_system;
            require(missing_provider && W3DDisplay::m_assetManager == assets,
                "original display purge accepted a missing source file-system provider");
            W3DDisplay::m_assetManager = nullptr;
            bool missing_assets = false;
            try { display->doSmartAssetPurgeAndPreload("Maps\\Owned\\AssetUsage.txt"); }
            catch (const std::runtime_error&) { missing_assets = true; }
            W3DDisplay::m_assetManager = assets;
            require(missing_assets, "original display purge accepted a broken published asset owner");

            load_packet();
            require(has_prototype("TEST.ZERO01") && has_prototype("ALT0.ZERO01"),
                "generated display purge prototypes missing");
            display->doSmartAssetPurgeAndPreload(nullptr);
            display->doSmartAssetPurgeAndPreload("");
            require(has_prototype("TEST.ZERO01") && has_prototype("ALT0.ZERO01"),
                "original display empty usage name changed assets");
            display->doSmartAssetPurgeAndPreload("Maps\\Owned\\AssetUsage.txt");
            require(has_prototype("TEST.ZERO01") && !has_prototype("ALT0.ZERO01"),
                "original display usage list did not preserve exactly the named source assets");
            display->doSmartAssetPurgeAndPreload("Maps\\Owned\\MissingAssetUsage.txt");
            require(!has_prototype("TEST.ZERO01") && !has_prototype("ALT0.ZERO01"),
                "original display absent usage list did not purge all source assets");
            load_packet();
            require(has_prototype("TEST.ZERO01") && has_prototype("ALT0.ZERO01"),
                "original display purge retry did not reload source assets");
            display->doSmartAssetPurgeAndPreload("Maps\\Owned\\CommentsOnly.txt");
            require(!has_prototype("TEST.ZERO01") && !has_prototype("ALT0.ZERO01"),
                "original display comment tokens entered the exclusion list");
            load_packet();
            auto* mesh = assets->Create_Render_Obj("TEST.ZERO01");
            auto* second_mesh = assets->Create_Render_Obj("TEST.ZERO01");
            require(mesh && second_mesh && mesh != second_mesh &&
                mesh->Num_Refs() == 1 && second_mesh->Num_Refs() == 1,
                "original display independent created meshes missing");
            scene3d->Add_Render_Object(mesh);
            scene3d->Add_Render_Object(second_mesh);
            require(mesh->Num_Refs() == 2 && second_mesh->Num_Refs() == 2,
                "original display scene refs missing");
            display->reset();
            require(mesh->Num_Refs() == 1 && second_mesh->Num_Refs() == 1 &&
                !mesh->Peek_Scene() && !second_mesh->Peek_Scene() &&
                W3DDisplay::m_3DScene == scene3d,
                "original display reset failed to detach objects while preserving owners");
            display->reset();
            mesh->Release_Ref();
            second_mesh->Release_Ref();
            display.reset();
            require(!W3DDisplay::m_3DScene && !W3DDisplay::m_2DScene &&
                !W3DDisplay::m_3DInterfaceScene && !W3DDisplay::m_assetManager &&
                !WW3D::Is_Initted(), "original display teardown retained source owners");
        }
    };
    if (physical) {
        for (int generation = 0; generation != 2; ++generation) {
            zh::renderer::BgfxOptions options;
            options.shader_root = ZH_BGFX_SHADER_DIR;
            zh::renderer::BgfxGpuDevice device(options);
            run(device);
            require(device.live_resource_count() == 0,
                "original display bgfx teardown retained resources");
        }
    } else {
        for (int generation = 0; generation != 2; ++generation) {
            zh::renderer::RecordingGpuDevice device;
            run(device);
            require(device.resource_counts().total() == 0,
                "original display Recording teardown retained resources");
        }
    }
    require(!W3DDisplay::m_3DScene && !W3DDisplay::m_2DScene &&
        !W3DDisplay::m_3DInterfaceScene && !W3DDisplay::m_assetManager && !WW3D::Is_Initted(),
        "original display teardown retained owners");
    std::puts(physical ? "original display owner: bgfx generations=2 resources=0" :
        "original display owner: source generations=2 resources=0");
}
