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
