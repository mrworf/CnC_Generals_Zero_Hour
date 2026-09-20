#pragma once

#include "zh/renderer/contract.h"

#include <cstddef>
#include <string_view>

namespace zh::renderer {

enum class ShaderFamily : UInt8 { ui, terrain, water, points, wwshade };

struct ShaderFamilyDesc {
    ShaderFamily family;
    std::string_view legacy_name;
    std::string_view vertex_source;
    std::string_view fragment_source;
    VertexLayout vertex_layout;
    UInt32 vertex_uniforms;
    UInt32 fragment_uniforms;
    UInt32 samplers;
    bool blending;
    bool depth_test;
    bool point_size;
    bool premultiplied_alpha;
};

struct ShaderRegistryView {
    const ShaderFamilyDesc* entries = nullptr;
    std::size_t size = 0;
};

ShaderRegistryView shader_registry() noexcept;
ValidationResult validate_shader_registry(ShaderRegistryView registry);
std::string_view provisional_backend_name() noexcept;

} // namespace zh::renderer
