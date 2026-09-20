#pragma once

#include "zh/renderer/contract.h"

#include <cstddef>
#include <string>
#include <string_view>
#include <vector>

namespace zh::effects {

using renderer::UInt8;

enum class EffectKind : UInt8 {
    water,
    particle_points,
    projected_texture,
    bump_environment,
    stealth,
    post_effect,
    wwshade_multipass,
    count,
};

struct EffectMapping {
    EffectKind kind;
    std::string_view legacy_name;
    std::string_view vertex_source;
    std::string_view fragment_source;
    renderer::VertexLayout vertex_layout;
    renderer::PrimitiveTopology topology;
    renderer::UInt32 vertex_uniforms;
    renderer::UInt32 fragment_uniforms;
    renderer::UInt32 samplers;
    renderer::UInt32 pass_count;
    bool blending;
    renderer::BlendFactor source_blend;
    renderer::BlendFactor destination_blend;
    bool depth_test;
    bool depth_write;
    float depth_bias;
    bool point_size;
    bool premultiplied_alpha;
    bool render_target_input;
};

struct EffectRegistryView {
    const EffectMapping* entries = nullptr;
    std::size_t size = 0;
};

struct EffectCorpusEntry {
    std::string logical_asset;
    std::string material;
    std::string legacy_effect;
};

struct EffectCorpusCoverage {
    std::vector<EffectCorpusEntry> entries;
};

EffectRegistryView effect_registry() noexcept;
std::string_view effect_kind_name(EffectKind kind) noexcept;
renderer::ValidationResult validate_effect_registry(EffectRegistryView registry);
renderer::ValidationResult find_effect(
    EffectRegistryView registry,
    std::string_view legacy_name,
    std::string_view logical_asset,
    std::string_view material,
    const EffectMapping*& output);
renderer::ValidationResult parse_effect_corpus(
    std::string_view text,
    EffectRegistryView registry,
    EffectCorpusCoverage& output);

} // namespace zh::effects
