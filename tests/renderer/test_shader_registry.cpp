#include "zh/renderer/shader_registry.h"
#include "zh/renderer/uniforms.h"

#include <filesystem>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

using namespace zh::renderer;

namespace {

void check(bool condition, const char* message)
{
    if (!condition) throw std::runtime_error(message);
}

void test_registry_files()
{
    const auto registry = shader_registry();
    check(validate_shader_registry(registry), "canonical shader registry rejected");
    check(provisional_backend_name() == "SDL_GPU", "provisional backend changed");
    for (std::size_t index = 0; index < registry.size; ++index) {
        const auto& entry = registry.entries[index];
        const auto vertex_source = std::filesystem::path(ZH_SOURCE_DIR) / "shaders/renderer" / entry.vertex_source;
        const auto fragment_source = std::filesystem::path(ZH_SOURCE_DIR) / "shaders/renderer" / entry.fragment_source;
        const auto vertex_output = std::filesystem::path(ZH_BINARY_DIR) / "generated/shaders/renderer" / (std::string(entry.vertex_source) + ".spv");
        const auto fragment_output = std::filesystem::path(ZH_BINARY_DIR) / "generated/shaders/renderer" / (std::string(entry.fragment_source) + ".spv");
        check(std::filesystem::is_regular_file(vertex_source), "registered vertex source missing");
        check(std::filesystem::is_regular_file(fragment_source), "registered fragment source missing");
        check(std::filesystem::is_regular_file(vertex_output), "compiled vertex module missing");
        check(std::filesystem::is_regular_file(fragment_output), "compiled fragment module missing");
    }
}

void test_registry_rejections()
{
    const auto canonical = shader_registry();
    std::vector<ShaderFamilyDesc> entries(canonical.entries, canonical.entries + canonical.size);
    entries[1].family = ShaderFamily::ui;
    check(!validate_shader_registry({entries.data(), entries.size()}), "duplicate shader family accepted");
    entries.assign(canonical.entries, canonical.entries + canonical.size);
    entries[0].fragment_uniforms = 5;
    check(!validate_shader_registry({entries.data(), entries.size()}), "excess shader uniforms accepted");
    entries.assign(canonical.entries, canonical.entries + canonical.size);
    entries[3].point_size = false;
    check(!validate_shader_registry({entries.data(), entries.size()}), "point shader without point size accepted");
    check(!validate_shader_registry({canonical.entries, canonical.size - 1}), "missing shader family accepted");
}

void test_uniform_layouts()
{
    check(alignof(FrameUniforms) == 16 && sizeof(FrameUniforms) % 16 == 0, "frame uniform packing changed");
    check(alignof(MaterialUniforms) == 16 && sizeof(MaterialUniforms) % 16 == 0, "material uniform packing changed");
    check(alignof(ObjectUniforms) == 16 && sizeof(ObjectUniforms) % 16 == 0, "object uniform packing changed");
    check(alignof(EffectUniforms) == 16 && sizeof(EffectUniforms) % 16 == 0, "effect uniform packing changed");
}

} // namespace

int main()
{
    try {
        test_registry_files();
        test_registry_rejections();
        test_uniform_layouts();
        std::cout << "shader registry tests: ok\n";
        return 0;
    } catch (const std::exception& error) {
        std::cerr << error.what() << '\n';
        return 1;
    }
}
