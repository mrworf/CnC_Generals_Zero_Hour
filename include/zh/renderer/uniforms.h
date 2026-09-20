#pragma once

#include <array>
#include <cstddef>
#include <type_traits>

namespace zh::renderer {

using Float4 = std::array<float, 4>;
using Matrix4 = std::array<float, 16>;

struct alignas(16) FrameUniforms {
    Matrix4 view_projection{};
    Float4 viewport_and_time{};
    Float4 fog_color{};
    Float4 fog_start_end_density{};
};

struct alignas(16) MaterialUniforms {
    Float4 diffuse{};
    Float4 ambient{};
    Float4 emissive{};
    Float4 alpha_test_and_flags{};
};

struct alignas(16) ObjectUniforms {
    Matrix4 model{};
    Matrix4 model_view{};
    Float4 point_size_and_depth_bias{};
    Float4 texture_transform{};
};

struct alignas(16) EffectUniforms {
    Float4 parameters[4]{};
};

static_assert(std::is_standard_layout_v<FrameUniforms> && sizeof(FrameUniforms) % 16 == 0);
static_assert(std::is_standard_layout_v<MaterialUniforms> && sizeof(MaterialUniforms) % 16 == 0);
static_assert(std::is_standard_layout_v<ObjectUniforms> && sizeof(ObjectUniforms) % 16 == 0);
static_assert(std::is_standard_layout_v<EffectUniforms> && sizeof(EffectUniforms) % 16 == 0);

} // namespace zh::renderer
