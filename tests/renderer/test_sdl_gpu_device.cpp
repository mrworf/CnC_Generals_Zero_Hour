#include "zh/platform/sdl_gpu_device.h"

#include <iostream>
#include <stdexcept>
#include <string>

namespace {

void check(bool condition, const char* message) { if (!condition) throw std::runtime_error(message); }

template <typename Callback>
std::string failure(Callback callback)
{
    try { callback(); }
    catch (const std::exception& error) { return error.what(); }
    throw std::runtime_error("expected SDL_GPU option validation failure");
}

void test_vulkan_only_and_shader_root_validation()
{
    zh::renderer::SdlGpuOptions options;
    options.shader_root = "/project-owned/shaders";
    options.driver = "direct3d12";
    check(failure([&] { zh::renderer::SdlGpuDevice device(options); }).find("must be 'vulkan'") != std::string::npos,
        "non-Vulkan backend did not fail closed");

    options.driver = "vulkan";
    options.shader_root.clear();
    check(failure([&] { zh::renderer::SdlGpuDevice device(options); }).find("shader root") != std::string::npos,
        "empty shader root did not produce an actionable error");
}

} // namespace

int main()
{
    try {
        test_vulkan_only_and_shader_root_validation();
        std::cout << "SDL_GPU device contract tests: ok\n";
        return 0;
    } catch (const std::exception& error) {
        std::cerr << error.what() << '\n';
        return 1;
    }
}
