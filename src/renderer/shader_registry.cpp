#include "zh/renderer/shader_registry.h"

#include <array>

namespace zh::renderer {
namespace {

constexpr std::array<ShaderFamilyDesc, 5> registry{{
    {ShaderFamily::ui, "fixed-function-ui", "ui.vert", "ui.frag", VertexLayout::position_color_uv, 1, 1, 1, true, false, false, true},
    {ShaderFamily::terrain, "terrain-base-and-lightmap", "terrain.vert", "terrain.frag", VertexLayout::terrain, 2, 1, 2, false, true, false, false},
    {ShaderFamily::water, "water-and-reflection", "water.vert", "water.frag", VertexLayout::water, 2, 2, 2, true, true, false, false},
    {ShaderFamily::points, "particle-point-list", "points.vert", "points.frag", VertexLayout::point_sprite, 2, 1, 1, true, true, true, true},
    {ShaderFamily::wwshade, "wwshade-multipass", "wwshade.vert", "wwshade.frag", VertexLayout::wwshade, 3, 4, 3, true, true, false, false},
}};

ValidationResult failure(std::string message) { return {false, std::move(message)}; }

} // namespace

ShaderRegistryView shader_registry() noexcept { return {registry.data(), registry.size()}; }

ValidationResult validate_shader_registry(ShaderRegistryView view)
{
    if (view.entries == nullptr) return failure("shader registry entries are null");
    if (view.size != registry.size()) return failure("shader registry must contain exactly five required families");
    std::array<bool, registry.size()> seen{};
    for (std::size_t index = 0; index < view.size; ++index) {
        const auto& entry = view.entries[index];
        const auto family_index = static_cast<std::size_t>(entry.family);
        if (family_index >= seen.size()) return failure("shader registry contains an unknown family");
        if (seen[family_index]) return failure("shader registry contains a duplicate family");
        seen[family_index] = true;
        if (entry.legacy_name.empty() || entry.vertex_source.empty() || entry.fragment_source.empty())
            return failure("shader registry family has an empty name or stage source");
        if (entry.vertex_uniforms > RendererLimits::uniform_buffers_per_stage
            || entry.fragment_uniforms > RendererLimits::uniform_buffers_per_stage)
            return failure("shader registry family exceeds four uniform buffers per stage");
        if (entry.samplers > RendererLimits::sampled_textures_per_stage)
            return failure("shader registry family exceeds sampled texture limit");
        if (entry.family == ShaderFamily::points && !entry.point_size)
            return failure("points shader family requires explicit point-size behavior");
    }
    for (bool present : seen)
        if (!present) return failure("shader registry is missing a required family");
    return {};
}

std::string_view provisional_backend_name() noexcept { return "SDL_GPU"; }

} // namespace zh::renderer
