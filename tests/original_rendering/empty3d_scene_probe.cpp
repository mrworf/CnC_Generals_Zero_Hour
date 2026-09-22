#include "PreRTS.h"
#include "Common/GlobalData.h"
#include "GameLogic/GameLogic.h"
#include "W3DDevice/GameClient/W3DDisplay.h"
#include "W3DDevice/GameClient/W3DScene.h"
#include "WW3D2/assetmgr.h"
#include "WW3D2/mesh.h"
#include "WW3D2/meshmdl.h"
#include "WWLib/RAMFILE.H"
#include "WW3D2/camera.h"
#include "WW3D2/dx8wrapper.h"
#include "WW3D2/ww3d.h"
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
    const char* rigid_path = std::getenv("ZH_M22_RIGID_3D_ASSET");
    const bool rigid = rigid_path != nullptr;
    std::vector<char> rigid_bytes;
    if (rigid) {
        std::ifstream input(rigid_path, std::ios::binary);
        require(input.good(), "original rigid W3D packet missing");
        rigid_bytes.assign(std::istreambuf_iterator<char>(input), std::istreambuf_iterator<char>());
        require(!rigid_bytes.empty(), "original rigid W3D packet empty");
    }
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
            MeshClass* mesh = nullptr;
            if (rigid) {
                RAMFileClass packet(rigid_bytes.data(), static_cast<int>(rigid_bytes.size()));
                require(assets.Load_3D_Assets(packet), "original rigid W3D packet rejected");
                mesh = static_cast<MeshClass*>(assets.Create_Render_Obj("TEST.ZERO01"));
                require(mesh && mesh->Class_ID() == RenderObjClass::CLASSID_MESH,
                    "original rigid W3D object missing");
                mesh->Peek_Model()->Peek_Single_Material()->Set_Lighting(false);
            }
            {
                RTS3DScene scene;
                CameraClass camera;
                camera.Set_Position(Vector3(0, 0, 1));
                camera.Set_View_Plane(Vector2(-1, -0.75f), Vector2(1, 0.75f));
                camera.Set_Clip_Planes(0.995f, 2.0f);
                auto* recorder = dynamic_cast<zh::renderer::RecordingGpuDevice*>(&device);
                auto frame = [&]() {
                    edge.bind_frame_targets(color, depth, width, target.height);
                    require(WW3D::Begin_Render(true, true, Vector3(0.2f, 0.4f, 0.6f), 1) ==
                        WW3D_ERROR_OK, "original 3D frame did not begin");
                    DX8Wrapper::Set_Transform(D3DTS_WORLD, Matrix3D(true));
                    scene.doRender(&camera);
                    require(WW3D::End_Render(false) == WW3D_ERROR_OK,
                        "original 3D frame did not end");
                    return readback(color);
                };
                const auto absent_marker = recorder ? recorder->snapshot().size() : 0;
                const auto absent = frame();
                if (recorder)
                    require(recorder->snapshot().substr(absent_marker).find("draw pipeline=") ==
                        std::string::npos, "original empty 3D frame submitted a draw");
                if (mesh) {
                    require(mesh->Num_Refs() == 1, "original rigid initial refcount drift");
                    scene.Add_Render_Object(mesh);
                    require(mesh->Num_Refs() == 2, "original rigid scene ref missing");
                    if (recorder) {
                        recorder->fail_next_buffer_upload();
                        bool rejected = false;
                        try { (void)frame(); }
                        catch (const std::runtime_error& error) {
                            rejected = std::string(error.what()).find("upload") != std::string::npos;
                        }
                        require(rejected && !WW3D::Is_Rendering() && !recorder->pass_active() &&
                            mesh->Num_Refs() == 2,
                            "original rigid injected upload did not abort and retire its frame");
                    }
                }
                const auto present_marker = recorder ? recorder->snapshot().size() : 0;
                const auto pixels = mesh ? frame() : absent;
                if (physical) {
                    require(pixels.size() == std::size_t(width) * target.height * 4 &&
                        absent.size() == pixels.size(),
                        "original empty 3D readback extent mismatch");
                    unsigned changed = 0;
                    for (std::size_t i = 0; i < pixels.size(); i += 4)
                        changed += pixels[i] != absent[i] || pixels[i+1] != absent[i+1] ||
                            pixels[i+2] != absent[i+2];
                    require(rigid ? changed > 0 : changed == 0,
                        "original 3D rigid/empty physical pixel expectation failed");
                } else {
                    require(recorder && (recorder->snapshot().substr(present_marker).find("draw pipeline=") !=
                        std::string::npos) == rigid, "original 3D rigid/empty draw expectation failed");
                    if (mesh) {
                        recorder->fail_next_draw();
                        bool rejected = false;
                        try { (void)frame(); }
                        catch (const std::runtime_error& error) {
                            rejected = std::string(error.what()).find("draw") != std::string::npos;
                        }
                        require(rejected && !WW3D::Is_Rendering() && !recorder->pass_active() &&
                            mesh->Num_Refs() == 2,
                            "original rigid injected draw did not abort and retire its frame");
                        (void)frame();
                    }
                }
                if (mesh) {
                    require(assets.Create_Render_Obj("TEST.MISSING") == nullptr,
                        "original rigid missing asset unexpectedly resolved");
                    auto* second = assets.Create_Render_Obj("TEST.ZERO01");
                    require(second && second->Num_Refs() == 1,
                        "original rigid second object was not independently owned");
                    scene.Add_Render_Object(second);
                    bool rejected_second = false;
                    try { (void)frame(); }
                    catch (const std::runtime_error& error) {
                        rejected_second = std::string(error.what()).find("non-rigid") !=
                            std::string::npos;
                    }
                    require(rejected_second && !WW3D::Is_Rendering() &&
                        (!recorder || !recorder->pass_active()),
                        "original rigid scene accepted a second object");
                    scene.Remove_Render_Object(second);
                    require(second->Num_Refs() == 1, "original rigid second scene ref remained");
                    second->Release_Ref();
                    mesh->Set_Position(Vector3(100, 100, 0));
                    const auto hidden_marker = recorder ? recorder->snapshot().size() : 0;
                    const auto hidden = frame();
                    if (physical) require(hidden == absent,
                        "original rigid camera-hidden object changed physical pixels");
                    else require(recorder->snapshot().substr(hidden_marker).find("draw pipeline=") ==
                        std::string::npos, "original rigid camera-hidden object drew");
                    mesh->Set_Position(Vector3(0, 0, 0));
                    (void)frame();
                    scene.Remove_Render_Object(mesh);
                    require(mesh->Num_Refs() == 1, "original rigid scene ref not retired");
                    const auto detached_marker = recorder ? recorder->snapshot().size() : 0;
                    const auto detached = frame();
                    if (physical) require(detached == absent,
                        "original rigid detach left stale physical pixels");
                    else require(recorder->snapshot().substr(detached_marker).find("draw pipeline=") ==
                        std::string::npos, "original rigid detach left a stale draw");
                }
            }
            if (mesh) mesh->Release_Ref();
            require(WW3D::Shutdown() == WW3D_ERROR_OK,
                "original empty 3D WW3D shutdown failed");
            if (rigid) assets.Free_Assets();
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
    std::puts(rigid ? (physical ? "original rigid 3D scene: physical generations=4 resources=0" :
        "original rigid 3D scene: source draw=1 resources=0") :
        (physical ? "original empty 3D scene: physical generations=4 resources=0" :
        "original empty 3D scene: source draws=0 resources=0"));
}
