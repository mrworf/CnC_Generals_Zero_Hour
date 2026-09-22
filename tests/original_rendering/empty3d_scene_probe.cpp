#include "PreRTS.h"
#include "Common/GlobalData.h"
#include "GameLogic/GameLogic.h"
#include "W3DDevice/GameClient/W3DDisplay.h"
#include "W3DDevice/GameClient/W3DScene.h"
#include "WW3D2/assetmgr.h"
#include "WW3D2/camera.h"
#include "WW3D2/dx8wrapper.h"
#include "WW3D2/ww3d.h"
#include "original_gpu_edge.h"
#include "zh/platform/bgfx_device.h"
#include "zh/renderer/recording_device.h"

#include <cstdio>
#include <cstdlib>
#include <stdexcept>
#include <string>
#include <vector>

namespace {
void require(bool condition, const char* message)
{
    if (!condition) throw std::runtime_error(message);
}
}

extern "C" void zh_probe_empty_3d_scene()
{
    require(TheGameLogic && TheGameLogic->isInGame() && TheWritableGlobalData,
        "original empty 3D scene requires initialized game owners");
    require(W3DDisplay::m_3DScene == NULL && W3DDisplay::m_assetManager == NULL,
        "original empty 3D probe cannot borrow published display owners");
    WW3DAssetManager assets;
    const bool physical = std::getenv("ZH_M22_EMPTY_3D_PHYSICAL") != nullptr;
    auto run = [&](zh::renderer::GpuDevice& device, unsigned width,
        zh::renderer::TextureFormat format, auto readback) {
        zh::renderer::TextureDesc target;
        target.width = width;
        target.height = width * 3 / 4;
        target.render_target = true;
        target.format = format;
        const auto color = device.create_texture(target, "original empty 3D color");
        target.format = zh::renderer::TextureFormat::depth24_stencil8;
        const auto depth = device.create_texture(target, "original empty 3D depth");
        require(color && depth, "original empty 3D targets are missing");
        {
            zh::original_runtime::OriginalGpuEdge edge(device);
            require(WW3D::Init(nullptr, nullptr, false) == WW3D_ERROR_OK,
                "original empty 3D WW3D init failed");
            {
                RTS3DScene scene;
                CameraClass camera;
                camera.Set_Position(Vector3(0, 0, 1));
                camera.Set_View_Plane(Vector2(-1, -0.75f), Vector2(1, 0.75f));
                camera.Set_Clip_Planes(0.995f, 2.0f);
                edge.bind_frame_targets(color, depth, width, target.height);
                require(WW3D::Begin_Render(true, true, Vector3(0.2f, 0.4f, 0.6f), 1) ==
                    WW3D_ERROR_OK, "original empty 3D frame did not begin");
                DX8Wrapper::Set_Transform(D3DTS_WORLD, Matrix3D(true));
                scene.doRender(&camera);
                require(WW3D::End_Render(false) == WW3D_ERROR_OK,
                    "original empty 3D frame did not end");
                if (physical) {
                    const auto pixels = readback(color);
                    require(pixels.size() == std::size_t(width) * target.height * 4,
                        "original empty 3D readback extent mismatch");
                    for (std::size_t i : {std::size_t(0), pixels.size() / 2 / 4 * 4,
                        pixels.size() - 4})
                        require(pixels[i] == 51 && pixels[i+1] == 102 && pixels[i+2] == 153,
                            "original empty 3D source clear pixel drifted");
                } else {
                    auto* recorder = dynamic_cast<zh::renderer::RecordingGpuDevice*>(&device);
                    require(recorder && recorder->snapshot().find("draw pipeline=") == std::string::npos,
                        "original empty 3D source submitted a draw");
                }
            }
            require(WW3D::Shutdown() == WW3D_ERROR_OK,
                "original empty 3D WW3D shutdown failed");
        }
        device.destroy(depth);
        device.destroy(color);
        if (physical) {
            auto* bgfx = dynamic_cast<zh::renderer::BgfxGpuDevice*>(&device);
            require(bgfx && bgfx->live_resource_count() == 0,
                "original empty 3D bgfx teardown retained resources");
        } else {
            auto* recorder = dynamic_cast<zh::renderer::RecordingGpuDevice*>(&device);
            require(recorder && recorder->resource_counts().total() == 0,
                "original empty 3D Recording teardown retained resources");
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
                require(device.wait_idle(), "original empty 3D bgfx idle failed");
            }
    } else {
        zh::renderer::RecordingGpuDevice device;
        run(device, 160, zh::renderer::TextureFormat::bgra8,
            [](auto) { return std::vector<unsigned char>{}; });
    }
    std::puts(physical ? "original empty 3D scene: physical generations=4 resources=0" :
        "original empty 3D scene: source draws=0 resources=0");
}
