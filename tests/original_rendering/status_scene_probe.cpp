#include "PreRTS.h"
#include "Common/GlobalData.h"
#include "GameLogic/GameLogic.h"
#include "GameLogic/ScriptEngine.h"
#include "W3DDevice/GameClient/W3DScene.h"
#include "WW3D2/camera.h"
#include "WW3D2/assetmgr.h"
#include "WW3D2/ww3d.h"
#include "WW3D2/dx8wrapper.h"
#include "original_gpu_edge.h"
#include "zh/renderer/recording_device.h"
#include "zh/platform/bgfx_device.h"

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

extern "C" void zh_probe_status_scene()
{
    require(TheGameLogic && TheGameLogic->isInGame() && TheScriptEngine && TheWritableGlobalData,
        "original status scene requires initialized in-game logic and script owners");
    for (unsigned frame = 0; frame != 256 &&
        TheScriptEngine->getFade() != ScriptEngine::FADE_NONE; ++frame)
        TheScriptEngine->update();
    require(TheScriptEngine->getFade() == ScriptEngine::FADE_NONE,
        "original status scene startup fade did not complete within 256 source updates");
    const Bool old_dot = TheWritableGlobalData->m_showTeamDot;
    WW3DAssetManager assets;
    const bool physical = std::getenv("ZH_M22_STATUS_PHYSICAL") != nullptr;
    auto run = [&](zh::renderer::GpuDevice& device, unsigned width,
        zh::renderer::TextureFormat format, auto readback) {
        zh::renderer::TextureDesc target;
        target.width = width;
        target.height = width * 3 / 4;
        target.render_target = true;
        target.format = format;
        const auto color = device.create_texture(target, "original 2D status color");
        target.format = zh::renderer::TextureFormat::depth24_stencil8;
        auto depth = device.create_texture(target, "original 2D status depth");
        require(color && depth, "original 2D status targets are missing");
        {
            zh::original_runtime::OriginalGpuEdge edge(device);
            require(WW3D::Init(nullptr, nullptr, false) == WW3D_ERROR_OK,
                "original 2D status WW3D init failed");
            {
                RTS2DScene scene;
                CameraClass camera;
                camera.Set_Position(Vector3(0, 0, 1));
                camera.Set_View_Plane(Vector2(-1, -0.75f), Vector2(1, 0.75f));
                camera.Set_Clip_Planes(0.995f, 2.0f);
                auto frame = [&](Bool show_dot) {
                    TheWritableGlobalData->m_showTeamDot = show_dot;
                    edge.bind_frame_targets(color, depth, width, target.height);
                    require(WW3D::Begin_Render(true, true, Vector3(0, 0, 0), 1) == WW3D_ERROR_OK,
                        "original 2D status frame did not begin");
                    DX8Wrapper::Set_Transform(D3DTS_WORLD, Matrix3D(true));
                    scene.doRender(&camera);
                    require(WW3D::End_Render(false) == WW3D_ERROR_OK,
                        "original 2D status frame did not end");
                    return readback(color);
                };
                auto* recorder = dynamic_cast<zh::renderer::RecordingGpuDevice*>(&device);
                const auto absent_trace = recorder ? recorder->snapshot().size() : 0;
                const auto without_dot = frame(FALSE);
                if (recorder)
                    require(recorder->snapshot().substr(absent_trace).find("draw pipeline=") == std::string::npos,
                        "original team-dot-off frame submitted a draw");
                if (recorder) {
                    recorder->fail_next_buffer_upload();
                    bool rejected = false;
                    try { (void)frame(TRUE); }
                    catch (const std::runtime_error& error) {
                        rejected = std::string(error.what()).find("upload") != std::string::npos;
                    }
                    require(rejected && !WW3D::Is_Rendering() && !recorder->pass_active(),
                        "original 2D status injected upload did not abort its source frame");
                }
                const auto with_dot = frame(TRUE);
                const Int old_mode = TheGameLogic->getGameMode();
                TheGameLogic->setGameMode(GAME_SHELL);
                const auto shell_trace = recorder ? recorder->snapshot().size() : 0;
                const auto shell_pixels = frame(TRUE);
                TheGameLogic->setGameMode(old_mode);
                if (recorder)
                    require(recorder->snapshot().substr(shell_trace).find("draw pipeline=") == std::string::npos,
                        "original shell status frame submitted a draw");
                if (!physical) {
                    require(recorder && recorder->snapshot().find("draw pipeline=") != std::string::npos,
                        "original 2D status source omitted its physical draw");
                    recorder->fail_next_draw();
                    bool rejected = false;
                    try { (void)frame(TRUE); }
                    catch (const std::runtime_error& error) {
                        rejected = std::string(error.what()).find("draw") != std::string::npos;
                    }
                    require(rejected && !WW3D::Is_Rendering() && !recorder->pass_active(),
                        "original 2D status injected draw did not abort its source frame");
                    (void)frame(TRUE);
                    bool unsupported_fade_state = false;
                    try {
                        DX8Wrapper::Set_DX8_Render_State(D3DRS_BLENDOP,
                            D3DBLENDOP_REVSUBTRACT);
                    } catch (const std::runtime_error& error) {
                        unsupported_fade_state =
                            std::string(error.what()).find("unsupported") != std::string::npos;
                    }
                    require(unsupported_fade_state && !WW3D::Is_Rendering(),
                        "original subtract-fade blend operation was not rejected before a frame");
                    (void)frame(TRUE);
                    device.destroy(depth);
                    bool stale = false;
                    try { (void)frame(TRUE); }
                    catch (const std::runtime_error&) { stale = true; }
                    require(stale && !WW3D::Is_Rendering(),
                        "original 2D status stale depth target was accepted");
                    depth = device.create_texture(target, "rebound original 2D status depth");
                    require(static_cast<bool>(depth), "original 2D status rebound depth is missing");
                    (void)frame(TRUE);
                }
                if (physical) {
                    require(with_dot.size() == std::size_t(width) * target.height * 4 &&
                        without_dot.size() == with_dot.size() && shell_pixels == without_dot,
                        "original status absence/readback extent mismatch");
                    unsigned changed = 0, upper_right = 0;
                    for (std::size_t i = 0; i < with_dot.size(); i += 4) {
                        const bool different = with_dot[i] != without_dot[i] ||
                            with_dot[i+1] != without_dot[i+1] || with_dot[i+2] != without_dot[i+2];
                        changed += different;
                        upper_right += different && (i / 4) % width >= width * 3 / 4 &&
                            (i / 4) / width < target.height / 4;
                    }
                    require(changed > 1 && upper_right > 0,
                        "original 2D team-dot source left no expected upper-right pixels");
                }
            }
            require(WW3D::Shutdown() == WW3D_ERROR_OK,
                "original 2D status WW3D shutdown failed");
        }
        device.destroy(depth);
        device.destroy(color);
        if (physical) {
            auto* bgfx = dynamic_cast<zh::renderer::BgfxGpuDevice*>(&device);
            require(bgfx && bgfx->live_resource_count() == 0,
                "original 2D status teardown retained bgfx resources");
        } else {
            auto* recorder = dynamic_cast<zh::renderer::RecordingGpuDevice*>(&device);
            require(recorder && recorder->resource_counts().total() == 0,
                "original 2D status teardown retained Recording resources");
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
                require(device.wait_idle(), "original 2D status bgfx idle failed");
            }
    } else {
        zh::renderer::RecordingGpuDevice device;
        run(device, 160, zh::renderer::TextureFormat::bgra8,
            [](auto) { return std::vector<unsigned char>{}; });
    }
    TheWritableGlobalData->m_showTeamDot = old_dot;
    std::puts(physical ? "original 2D status scene: physical generations=4 resources=0" :
        "original 2D status scene: source draw=1 resources=0");
}
