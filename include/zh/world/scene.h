#pragma once

#include "zh/renderer/contract.h"

#include <array>
#include <string>
#include <string_view>
#include <vector>

namespace zh::world {

using renderer::UInt8;
using renderer::UInt32;

enum class GeometryFamily : UInt8 {
    mesh,
    animated_mesh,
    terrain,
    road_decal,
    tree,
    shadow,
    faction_geometry,
    selection_marker,
};

enum class Faction : UInt8 { neutral, usa, china, gla };

struct Matrix4 {
    std::array<float, 16> elements{
        1.0F, 0.0F, 0.0F, 0.0F,
        0.0F, 1.0F, 0.0F, 0.0F,
        0.0F, 0.0F, 1.0F, 0.0F,
        0.0F, 0.0F, 0.0F, 1.0F,
    };
};

struct Camera {
    Matrix4 view;
    Matrix4 projection;
    float near_depth = 0.1F;
    float far_depth = 10000.0F;
    UInt32 viewport_width = 1280;
    UInt32 viewport_height = 720;
};

struct LightingState {
    bool enabled = true;
    UInt8 directional_light_count = 1;
    UInt32 ambient_argb = renderer::pack_argb8(255, 64, 64, 64);
};

struct FogState {
    bool enabled = false;
    float start = 0.0F;
    float end = 1000.0F;
    UInt32 color_argb = renderer::pack_argb8(255, 128, 128, 128);
};

struct AlphaTestState {
    bool enabled = false;
    renderer::CompareOp compare = renderer::CompareOp::greater;
    float reference = 0.5F;
};

struct TextureStage {
    renderer::AddressMode address_u = renderer::AddressMode::repeat;
    renderer::AddressMode address_v = renderer::AddressMode::repeat;
    renderer::Filter filter = renderer::Filter::linear;
    bool modulate = true;
};

struct LegacyMaterialState {
    bool blending = false;
    renderer::BlendFactor source_blend = renderer::BlendFactor::src_alpha;
    renderer::BlendFactor destination_blend = renderer::BlendFactor::inv_src_alpha;
    UInt8 color_write_mask = 0x0f;
    bool depth_test = true;
    bool depth_write = true;
    renderer::CompareOp depth_compare = renderer::CompareOp::less_equal;
    renderer::CullMode cull = renderer::CullMode::counter_clockwise;
    float depth_bias = 0.0F;
    LightingState lighting;
    FogState fog;
    AlphaTestState alpha_test;
    std::array<TextureStage, 4> texture_stages{};
    UInt8 texture_stage_count = 1;
};

struct WorldItem {
    std::string label;
    GeometryFamily family = GeometryFamily::mesh;
    Faction faction = Faction::neutral;
    Matrix4 transform;
    LegacyMaterialState material;
    UInt32 vertex_count = 3;
    UInt32 index_count = 3;
    UInt32 animation_frame = 0;
    bool casts_shadow = false;
};

struct WorldScene {
    std::string map_name;
    Camera camera;
    std::vector<WorldItem> items;
};

struct ShaderFeatures {
    bool transforms = true;
    bool lighting = false;
    bool fog = false;
    bool alpha_test = false;
    renderer::CompareOp alpha_compare = renderer::CompareOp::always;
    float alpha_reference = 0.0F;
    UInt8 texture_stage_count = 0;
};

struct MaterialTranslation {
    renderer::PipelineDesc pipeline;
    std::array<renderer::SamplerDesc, 4> samplers{};
    ShaderFeatures features;
};

renderer::ValidationResult validate(const Matrix4& matrix);
renderer::ValidationResult validate(const Camera& camera);
renderer::ValidationResult validate(const LegacyMaterialState& material);
renderer::ValidationResult validate(const WorldScene& scene);
renderer::ValidationResult translate_material(
    const LegacyMaterialState& material,
    GeometryFamily family,
    renderer::ShaderHandle vertex_shader,
    renderer::ShaderHandle fragment_shader,
    MaterialTranslation& output);

std::string_view geometry_family_name(GeometryFamily family) noexcept;
std::string_view faction_name(Faction faction) noexcept;
renderer::VertexLayout vertex_layout(GeometryFamily family) noexcept;

} // namespace zh::world
