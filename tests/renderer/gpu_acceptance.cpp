#include "zh/platform/sdl_gpu_device.h"

#include <SDL3/SDL.h>

#include <array>
#include <iostream>
#include <stdexcept>
#include <string>

namespace {

using namespace zh::renderer;

void check(bool condition, const std::string& message) { if (!condition) throw std::runtime_error(message); }

struct Vertex { float x, y; std::uint32_t color; float u, v; };

void render_generation(SDL_Window* window, int generation)
{
    SdlGpuOptions options;
    options.shader_root = ZH_GPU_SHADER_DIR;
    options.debug = true;
    SdlGpuDevice device(options);
    check(device.capabilities().backend == "vulkan", "SDL_GPU did not select Vulkan");
    check(!device.claim_window(nullptr) && device.last_error().find("null") != std::string::npos,
        "null window was not rejected");
    check(device.claim_window(window), device.last_error());
    check(!device.present({}) && device.last_error().find("source texture") != std::string::npos,
        "stale presentation source was not rejected");

    const auto vertex_shader = device.create_shader({ShaderStage::vertex, "acceptance.vert", 0, 0}, "M14 acceptance vertex");
    const auto fragment_shader = device.create_shader({ShaderStage::fragment, "acceptance.frag", 0, 0}, "M14 acceptance fragment");
    const auto vertices = device.create_buffer({sizeof(Vertex) * 3, BufferUsage::vertex, true}, "M14 acceptance vertices");
    TextureDesc color_desc;
    color_desc.width = 320; color_desc.height = 240; color_desc.render_target = true;
    const auto color = device.create_texture(color_desc, "M14 acceptance color");
    auto depth_desc = color_desc;
    depth_desc.format = TextureFormat::depth24_stencil8; depth_desc.sampled = false;
    const auto depth = device.create_texture(depth_desc, "M14 acceptance depth");
    PipelineDesc pipeline_desc;
    pipeline_desc.vertex_shader = vertex_shader; pipeline_desc.fragment_shader = fragment_shader;
    pipeline_desc.vertex_layout = VertexLayout::position_color_uv;
    pipeline_desc.raster.cull = CullMode::none;
    const auto pipeline = device.create_pipeline(PipelineKey(pipeline_desc), "M14 acceptance pipeline");
    check(vertex_shader && fragment_shader && vertices && color && depth && pipeline, device.last_error());

    const std::array<Vertex, 3> triangle{{
        {-0.8F, 0.7F, 0xff2020ffU, 0.0F, 0.0F},
        {0.8F, 0.7F, 0xff20ff20U, 1.0F, 0.0F},
        {0.0F, -0.8F, 0xffff2020U, 0.5F, 1.0F},
    }};
    check(device.upload({vertices, sizeof(triangle), 0, sizeof(triangle)}, triangle.data()), device.last_error());
    RenderPassDesc pass;
    pass.color_targets[0] = color; pass.color_target_count = 1; pass.depth_target = depth;
    pass.width = 320; pass.height = 240;
    check(device.begin_pass(pass, "M14 generation " + std::to_string(generation)), device.last_error());
    check(!device.begin_pass(pass, "nested") && device.last_error().find("already active") != std::string::npos,
        "nested render pass was not rejected");
    DrawDesc draw;
    draw.pipeline = pipeline; draw.vertex_buffer = vertices; draw.vertex_or_index_count = 3;
    check(device.draw(draw), device.last_error());
    check(device.end_pass(), device.last_error());
    check(!device.end_pass() && device.last_error().find("no render pass") != std::string::npos,
        "end without pass was not rejected");
    check(device.present(color), device.last_error());
    check(device.wait_idle(), device.last_error());

    std::cout << "generation=" << generation << " backend=" << device.capabilities().backend
              << " device=" << device.capabilities().device_name
              << " driver=" << device.capabilities().driver_name << " " << device.capabilities().driver_version
              << " sdl=" << device.capabilities().sdl_version
              << " bc=" << device.capabilities().bc1 << device.capabilities().bc2 << device.capabilities().bc3 << '\n';

    device.destroy(pipeline); device.destroy(vertices); device.destroy(fragment_shader); device.destroy(vertex_shader);
    device.destroy(depth); device.destroy(color);
    device.release_window();
}

} // namespace

int main()
{
    SDL_Window* window = nullptr;
    try {
        check(SDL_Init(SDL_INIT_VIDEO), std::string("SDL video init failed: ") + SDL_GetError());
        window = SDL_CreateWindow("Zero Hour M14 SDL_GPU acceptance", 320, 240, SDL_WINDOW_RESIZABLE);
        check(window != nullptr, std::string("SDL window creation failed: ") + SDL_GetError());
        render_generation(window, 1);
        check(SDL_SetWindowSize(window, 400, 300) && SDL_SyncWindow(window),
            std::string("resize transition failed: ") + SDL_GetError());
        check(SDL_SetWindowFullscreen(window, true) && SDL_SyncWindow(window),
            std::string("fullscreen enter failed: ") + SDL_GetError());
        check(SDL_SetWindowFullscreen(window, false) && SDL_SyncWindow(window),
            std::string("fullscreen leave failed: ") + SDL_GetError());
        SDL_HideWindow(window);
        SDL_ShowWindow(window);
        render_generation(window, 2);
        SDL_DestroyWindow(window);
        SDL_QuitSubSystem(SDL_INIT_VIDEO);
        std::cout << "SDL_GPU resource/pass/presentation/resize/focus/fullscreen/recreation: ok\n";
        return 0;
    } catch (const std::exception& error) {
        if (window) SDL_DestroyWindow(window);
        SDL_QuitSubSystem(SDL_INIT_VIDEO);
        std::cerr << error.what() << '\n';
        return 1;
    }
}
