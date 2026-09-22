#include "PreRTS.h"
#include "GameLogic/GameLogic.h"
#include "GameClient/CommandXlat.h"
#include "W3DDevice/GameClient/W3DDisplay.h"
#include "W3DDevice/GameClient/W3DScene.h"
#include "W3DDevice/GameClient/W3DView.h"
#include "W3DDevice/GameClient/W3DAssetManager.h"
#include "WW3D2/assetmgr.h"
#include "WW3D2/camera.h"
#include "WW3D2/dx8wrapper.h"
#include "WW3D2/rendobj.h"
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
        {
            W3DView no_edge;
            bool rejected = false;
            try { no_edge.init(); }
            catch (const std::runtime_error&) { rejected = true; }
            require(rejected && !no_edge.get3DCamera(),
                "original W3DView initialized without source display/edge");
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
                auto* view = new W3DView;
                view->init();
                auto* camera = view->get3DCamera();
                require(camera && camera->Num_Refs() == 1,
                    "original W3DView 3D camera owner missing");
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
                const auto absent_marker = recorder ? recorder->snapshot().size() : 0;
                const auto absent = frame();
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
            }
            require(!W3DDisplay::m_3DScene && !WW3D::Is_Initted(),
                "original W3DView display teardown retained source owners");
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
