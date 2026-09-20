#include "zh/effects/registry.h"

#include <array>
#include <cmath>
#include <set>
#include <sstream>
#include <string>
#include <utility>

namespace zh::effects {
namespace {

constexpr std::array<EffectMapping, static_cast<std::size_t>(EffectKind::count)> mappings{{
    {EffectKind::water, "water-and-reflection", "renderer/water.vert", "renderer/water.frag",
        renderer::VertexLayout::water, renderer::PrimitiveTopology::triangle_list, 2, 2, 2, 1, true,
        renderer::BlendFactor::src_alpha, renderer::BlendFactor::inv_src_alpha, true, true, 0.0F, false, false, true},
    {EffectKind::particle_points, "particle-point-list", "renderer/points.vert", "renderer/points.frag",
        renderer::VertexLayout::point_sprite, renderer::PrimitiveTopology::point_list, 2, 1, 1, 1, true,
        renderer::BlendFactor::one, renderer::BlendFactor::inv_src_alpha, true, false, 0.0F, true, true, false},
    {EffectKind::projected_texture, "projected-texture", "effects/projected.vert", "effects/projected.frag",
        renderer::VertexLayout::wwshade, renderer::PrimitiveTopology::triangle_list, 2, 1, 1, 1, true,
        renderer::BlendFactor::src_alpha, renderer::BlendFactor::inv_src_alpha, true, false, -1.0F, false, false, false},
    {EffectKind::bump_environment, "bump-environment", "effects/bump_environment.vert", "effects/bump_environment.frag",
        renderer::VertexLayout::wwshade, renderer::PrimitiveTopology::triangle_list, 2, 1, 3, 1, false,
        renderer::BlendFactor::one, renderer::BlendFactor::zero, true, true, 0.0F, false, false, false},
    {EffectKind::stealth, "stealth-distortion", "effects/stealth.vert", "effects/stealth.frag",
        renderer::VertexLayout::wwshade, renderer::PrimitiveTopology::triangle_list, 2, 1, 2, 1, true,
        renderer::BlendFactor::src_alpha, renderer::BlendFactor::inv_src_alpha, true, false, 0.25F, false, false, true},
    {EffectKind::post_effect, "scene-post-effect", "effects/post_effect.vert", "effects/post_effect.frag",
        renderer::VertexLayout::position_color_uv, renderer::PrimitiveTopology::triangle_strip, 1, 1, 1, 1, false,
        renderer::BlendFactor::one, renderer::BlendFactor::zero, false, false, 0.0F, false, false, true},
    {EffectKind::wwshade_multipass, "wwshade-multipass", "renderer/wwshade.vert", "renderer/wwshade.frag",
        renderer::VertexLayout::wwshade, renderer::PrimitiveTopology::triangle_list, 3, 4, 3, 2, true,
        renderer::BlendFactor::src_alpha, renderer::BlendFactor::inv_src_alpha, true, true, 0.5F, false, false, false},
}};

renderer::ValidationResult failure(std::string message) { return {false, std::move(message)}; }

std::vector<std::string> fields(const std::string& line)
{
    std::vector<std::string> result;
    std::size_t begin = 0;
    while (true) {
        const auto end = line.find('\t', begin);
        result.push_back(line.substr(begin, end == std::string::npos ? std::string::npos : end - begin));
        if (end == std::string::npos) return result;
        begin = end + 1;
    }
}

} // namespace

EffectRegistryView effect_registry() noexcept { return {mappings.data(), mappings.size()}; }

std::string_view effect_kind_name(EffectKind kind) noexcept
{
    switch (kind) {
    case EffectKind::water: return "water";
    case EffectKind::particle_points: return "particle-points";
    case EffectKind::projected_texture: return "projected-texture";
    case EffectKind::bump_environment: return "bump-environment";
    case EffectKind::stealth: return "stealth";
    case EffectKind::post_effect: return "post-effect";
    case EffectKind::wwshade_multipass: return "wwshade-multipass";
    case EffectKind::count: break;
    }
    return "unknown";
}

renderer::ValidationResult validate_effect_registry(EffectRegistryView registry)
{
    constexpr auto required_size = static_cast<std::size_t>(EffectKind::count);
    if (registry.entries == nullptr) return failure("effect registry entries are null");
    if (registry.size != required_size) return failure("effect registry must contain exactly seven required effect families");
    std::array<bool, required_size> seen{};
    std::set<std::string_view> names;
    for (std::size_t index = 0; index < registry.size; ++index) {
        const auto& entry = registry.entries[index];
        const auto kind = static_cast<std::size_t>(entry.kind);
        if (kind >= required_size) return failure("effect registry contains an unknown family");
        if (seen[kind]) return failure("effect registry contains duplicate family " + std::string(effect_kind_name(entry.kind)));
        seen[kind] = true;
        if (entry.legacy_name.empty() || entry.vertex_source.empty() || entry.fragment_source.empty())
            return failure("effect registry family has an empty legacy name or GLSL source");
        if (!names.insert(entry.legacy_name).second)
            return failure("effect registry contains duplicate legacy effect " + std::string(entry.legacy_name));
        if (entry.vertex_uniforms > renderer::RendererLimits::uniform_buffers_per_stage
            || entry.fragment_uniforms > renderer::RendererLimits::uniform_buffers_per_stage
            || entry.samplers > renderer::RendererLimits::sampled_textures_per_stage)
            return failure("effect " + std::string(entry.legacy_name) + " exceeds renderer binding limits");
        if (entry.pass_count == 0 || entry.pass_count > 4)
            return failure("effect " + std::string(entry.legacy_name) + " pass count must be in range 1..4");
        if (!std::isfinite(entry.depth_bias)) return failure("effect " + std::string(entry.legacy_name) + " depth bias is not finite");
        if (entry.render_target_input && entry.samplers == 0)
            return failure("effect " + std::string(entry.legacy_name) + " requires a render target but has no sampler");
        if (entry.premultiplied_alpha && (!entry.blending || entry.source_blend != renderer::BlendFactor::one))
            return failure("premultiplied effect " + std::string(entry.legacy_name) + " must blend with source one");
        if (entry.kind == EffectKind::particle_points
            && (entry.topology != renderer::PrimitiveTopology::point_list || !entry.point_size))
            return failure("particle-point-list must preserve point-list topology and point size");
        if (entry.kind == EffectKind::projected_texture && entry.depth_bias == 0.0F)
            return failure("projected-texture must preserve a nonzero depth bias");
        if (entry.kind == EffectKind::wwshade_multipass && entry.pass_count < 2)
            return failure("wwshade-multipass must contain at least two ordered passes");
    }
    for (bool present : seen) if (!present) return failure("effect registry is missing a required family");
    return {};
}

renderer::ValidationResult find_effect(
    EffectRegistryView registry,
    std::string_view legacy_name,
    std::string_view logical_asset,
    std::string_view material,
    const EffectMapping*& output)
{
    output = nullptr;
    if (auto result = validate_effect_registry(registry); !result) return result;
    for (std::size_t index = 0; index < registry.size; ++index) {
        if (registry.entries[index].legacy_name == legacy_name) {
            output = &registry.entries[index];
            return {};
        }
    }
    return failure("unknown required effect '" + std::string(legacy_name) + "' for logical asset '"
        + std::string(logical_asset) + "' material '" + std::string(material) + "'");
}

renderer::ValidationResult parse_effect_corpus(
    std::string_view text,
    EffectRegistryView registry,
    EffectCorpusCoverage& output)
{
    if (auto result = validate_effect_registry(registry); !result) return result;
    EffectCorpusCoverage parsed;
    std::set<std::string> seen_effects;
    std::set<std::pair<std::string, std::string>> seen_assets;
    std::istringstream stream{std::string(text)};
    std::string line;
    std::size_t line_number = 0;
    while (std::getline(stream, line)) {
        ++line_number;
        if (line.empty() || line[0] == '#') continue;
        const auto columns = fields(line);
        if (columns.size() != 4 || columns[0] != "effect" || columns[1].empty() || columns[2].empty() || columns[3].empty())
            return failure("effects corpus line " + std::to_string(line_number)
                + " must contain effect, logical asset, material, and legacy effect");
        if (!seen_assets.emplace(columns[1], columns[2]).second)
            return failure("effects corpus line " + std::to_string(line_number) + " duplicates logical asset '"
                + columns[1] + "' material '" + columns[2] + "'");
        if (!seen_effects.insert(columns[3]).second)
            return failure("effects corpus line " + std::to_string(line_number) + " duplicates legacy effect '" + columns[3] + "'");
        const EffectMapping* mapping = nullptr;
        if (auto result = find_effect(registry, columns[3], columns[1], columns[2], mapping); !result)
            return failure("effects corpus line " + std::to_string(line_number) + ": " + result.error);
        parsed.entries.push_back({columns[1], columns[2], columns[3]});
    }
    if (parsed.entries.size() != registry.size)
        return failure("effects corpus must map every required effect exactly once; expected "
            + std::to_string(registry.size) + ", found " + std::to_string(parsed.entries.size()));
    output = std::move(parsed);
    return {};
}

} // namespace zh::effects
