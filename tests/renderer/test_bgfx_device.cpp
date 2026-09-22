#include "zh/platform/bgfx_device.h"

#include <bgfx/bgfx.h>

#include <array>
#include <iostream>
#include <stdexcept>
#include <string>

namespace {

void check(bool condition, const char* message)
{
    if (!condition) throw std::runtime_error(message);
}

template <class F> std::string failure(F action)
{
    try { action(); } catch (const std::exception& error) { return error.what(); }
    throw std::runtime_error("expected construction failure");
}

void test_options()
{
    zh::renderer::BgfxOptions options;
    options.shader_root = "/project-owned/shaders";
    options.driver = "direct3d12";
    check(failure([&] { zh::renderer::BgfxGpuDevice device(options); }).find("must be 'vulkan'") != std::string::npos,
        "non-Vulkan backend did not fail closed");
    options.driver = "vulkan";
    options.shader_root.clear();
    check(failure([&] { zh::renderer::BgfxGpuDevice device(options); }).find("shader root") != std::string::npos,
        "empty shader root did not fail closed");
}

void test_resources()
{
    using namespace zh::renderer;
    BgfxOptions options;
    options.shader_root = "/project-owned/shaders";
    for (int generation = 0; generation != 2; ++generation) {
        BgfxGpuDevice device(options);
        check(bgfx::getRendererType() == bgfx::RendererType::Vulkan, "bgfx did not select Vulkan");
        check(!device.create_texture({}, "invalid texture"), "zero-extent texture accepted");
        check(device.last_error().find("nonzero") != std::string::npos, "texture error unavailable");

        TextureDesc desc;
        desc.width = 8; desc.height = 8;
        check(device.supports_texture_format(desc.format, desc.dimension, true, false), "RGBA8 unsupported");
        auto texture = device.create_texture(desc, "owned test texture");
        check(bool(texture), "physical texture allocation failed");
        std::array<unsigned char, 8 * 8 * 4> pixels{};
        pixels.fill(17);
        check(device.upload_texture({texture, 8, 8, 8 * 4, pixels.size(), 0}, pixels.data()),
            "texture upload failed");
        check(!device.upload_texture({texture, 8, 8, 7, pixels.size(), 0}, pixels.data()),
            "short row pitch accepted");
        device.destroy(texture);
        check(!device.upload_texture({texture, 8, 8, 8 * 4, pixels.size(), 0}, pixels.data()),
            "stale texture accepted");
        auto replacement = device.create_texture(desc, "replacement");
        check(bool(replacement) && replacement != texture, "texture generation did not advance");
        device.destroy(replacement);

        auto buffer = device.create_buffer({32, BufferUsage::vertex, true}, "owned test buffer");
        check(bool(buffer), "buffer creation failed");
        check(device.upload({buffer, 32, 0, 4}, pixels.data()), "buffer upload failed");
        check(!device.upload({buffer, 32, 31, 4}, pixels.data()), "overflow buffer upload accepted");
        device.destroy(buffer);
        check(!device.upload({buffer, 32, 0, 4}, pixels.data()), "stale buffer accepted");
        check(!device.clear_viewport({}), "unimplemented target clear silently accepted");
        check(!device.pass_active(), "pass unexpectedly active");
        check(device.wait_idle(), "wait_idle failed");

        TextureDesc color_desc;
        color_desc.width = 160; color_desc.height = 120;
        color_desc.format = TextureFormat::bgra8; color_desc.render_target = true;
        auto color = device.create_texture(color_desc, "clear test color");
        auto depth_desc = color_desc;
        depth_desc.format = TextureFormat::depth24_stencil8;
        auto depth = device.create_texture(depth_desc, "clear test depth-stencil");
        check(bool(color) && bool(depth), "physical clear targets unavailable");
        RenderPassDesc pass;
        pass.color_targets[0] = color; pass.color_target_count = 1; pass.depth_target = depth;
        pass.width = 160; pass.height = 120; pass.target_generation = 5;
        pass.clear_color = {1, 0, 0, 1}; pass.clear_depth = 0.25f;
        pass.color_load = AttachmentLoad::load;
        check(!device.begin_pass(pass, "uninitialized color load"), "uninitialized color LOAD accepted");
        pass.color_load = AttachmentLoad::clear;
        pass.depth_load = AttachmentLoad::load;
        check(!device.begin_pass(pass, "uninitialized depth load"), "uninitialized depth LOAD accepted");
        pass.depth_load = AttachmentLoad::clear;
        check(device.begin_pass(pass, "outer clear"), "outer pass failed");
        check(device.pass_active() && device.active_pass_extent() == std::pair<UInt32,UInt32>{160,120},
            "active pass state/extent wrong");
        check(!device.begin_pass(pass, "illegal nested pass"), "nested pass accepted");
        check(!device.set_viewport({40.5f,30,80,60,0,1}), "fractional viewport silently rounded");
        check(!device.set_viewport({40,30,80,60,0.25f,1}), "unsupported depth range silently ignored");
        check(device.set_viewport({40,30,80,60,0,1}), "camera viewport failed");
        ViewportClearDesc inset;
        inset.color_target = color; inset.depth_target = depth;
        inset.target_generation = 5; inset.x = 40; inset.y = 30;
        inset.width = 80; inset.height = 60;
        inset.color = inset.depth = inset.stencil = true;
        inset.color_value = {0, 0, 1, 1}; inset.depth_value = 1; inset.stencil_value = 0;
        auto stale = inset; stale.target_generation = 4;
        check(!device.clear_viewport(stale), "stale target generation accepted");
        check(device.clear_viewport(inset), "ordered inset clear failed");
        check(device.end_pass(), "end pass failed");
        check(!device.end_pass(), "inactive end pass accepted");
        auto result = device.readback_rgba(color);
        check(result.size() == 160U * 120U * 4U, "clear readback failed");
        const auto pixel = [&](int x, int y) { return &result[(y * 160 + x) * 4]; };
        const auto red = [&](int x, int y) {
            const auto* p = pixel(x,y); return p[0] == 255 && p[1] == 0 && p[2] == 0 && p[3] == 255;
        };
        const auto blue = [&](int x, int y) {
            const auto* p = pixel(x,y); return p[0] == 0 && p[1] == 0 && p[2] == 255 && p[3] == 255;
        };
        check(red(0,0) && red(39,30) && blue(40,30) && blue(119,89)
            && red(120,89) && red(159,119), "outer/inset ordered color pixels differ");

        pass.color_load = pass.depth_load = AttachmentLoad::load;
        check(device.begin_pass(pass, "preserving load"), "initialized LOAD rejected");
        auto stencil_only = inset;
        stencil_only.color = false; stencil_only.depth = false; stencil_only.stencil = true;
        stencil_only.stencil_value = 3;
        check(device.clear_viewport(stencil_only), "stencil-only clear failed");
        auto depth_only = inset;
        depth_only.color = false; depth_only.depth = true; depth_only.stencil = false;
        depth_only.depth_value = 0.5f;
        check(device.clear_viewport(depth_only), "depth-only clear failed");
        check(device.end_pass(), "preservation pass end failed");
        result = device.readback_rgba(color);
        check(result.size() == 160U * 120U * 4U && red(0,0) && blue(40,30) && red(159,119),
            "independent depth/stencil clear changed color attachment");

        check(device.begin_pass(pass, "view budget"), "post-readback pass failed");
        for (UInt32 view = 1; view < RendererLimits::ordered_views; ++view)
            check(device.clear_viewport(stencil_only), "view budget rejected a legal view");
        check(!device.clear_viewport(stencil_only), "view budget exhaustion accepted");
        check(device.end_pass() && device.wait_idle(), "view-budget frame did not end");
        check(device.begin_pass(pass, "reclaimed view budget"), "view budget did not reset next frame");
        check(device.end_pass() && device.wait_idle(), "reclaimed frame did not end");
        device.destroy(color); device.destroy(depth);
    }
}

} // namespace

int main(int argc, char** argv)
{
    try {
        test_options();
        if (argc > 1 && std::string(argv[1]) == "--gpu") test_resources();
        std::cout << "bgfx resource/device contract tests: ok\n";
        return 0;
    } catch (const std::exception& error) {
        std::cerr << error.what() << '\n';
        return 1;
    }
}
