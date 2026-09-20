#include "zh/world/scene.h"

#include <cmath>
#include <limits>
#include <string>

namespace zh::world {
namespace {

renderer::ValidationResult failure(std::string message) { return {false, std::move(message)}; }

bool valid_family(GeometryFamily family) noexcept
{
    return static_cast<unsigned>(family) <= static_cast<unsigned>(GeometryFamily::selection_marker);
}

bool valid_faction(Faction faction) noexcept
{
    return static_cast<unsigned>(faction) <= static_cast<unsigned>(Faction::gla);
}

bool valid_address(renderer::AddressMode mode) noexcept
{
    return static_cast<unsigned>(mode) <= static_cast<unsigned>(renderer::AddressMode::clamp_border);
}

} // namespace

std::string_view geometry_family_name(GeometryFamily family) noexcept
{
    switch (family) {
    case GeometryFamily::mesh: return "mesh";
    case GeometryFamily::animated_mesh: return "animated-mesh";
    case GeometryFamily::terrain: return "terrain";
    case GeometryFamily::road_decal: return "road-decal";
    case GeometryFamily::tree: return "tree";
    case GeometryFamily::shadow: return "shadow";
    case GeometryFamily::faction_geometry: return "faction-geometry";
    case GeometryFamily::selection_marker: return "selection-marker";
    }
    return "unknown";
}

std::string_view faction_name(Faction faction) noexcept
{
    switch (faction) {
    case Faction::neutral: return "neutral";
    case Faction::usa: return "usa";
    case Faction::china: return "china";
    case Faction::gla: return "gla";
    }
    return "unknown";
}

renderer::VertexLayout vertex_layout(GeometryFamily family) noexcept
{
    return family == GeometryFamily::terrain ? renderer::VertexLayout::terrain : renderer::VertexLayout::world_mesh;
}

renderer::ValidationResult validate(const Matrix4& matrix)
{
    for (float value : matrix.elements)
        if (!std::isfinite(value)) return failure("transform contains a non-finite component");
    if (matrix.elements[15] == 0.0F) return failure("transform has a zero homogeneous component");
    return {};
}

renderer::ValidationResult validate(const Camera& camera)
{
    if (auto result = validate(camera.view); !result) return failure("camera view: " + result.error);
    if (auto result = validate(camera.projection); !result) return failure("camera projection: " + result.error);
    if (!std::isfinite(camera.near_depth) || !std::isfinite(camera.far_depth)
        || camera.near_depth < renderer::RendererConventions::depth_minimum || camera.near_depth >= camera.far_depth)
        return failure("camera depth range must be finite, ordered, and begin in 0..1 depth space");
    if (camera.viewport_width == 0 || camera.viewport_height == 0)
        return failure("camera viewport must be nonzero");
    return {};
}

renderer::ValidationResult validate(const LegacyMaterialState& material)
{
    if (material.texture_stage_count == 0 || material.texture_stage_count > material.texture_stages.size())
        return failure("material texture stage count must be in range 1..4");
    if ((material.color_write_mask & 0xf0U) != 0 || material.color_write_mask == 0)
        return failure("material color write mask must select at least one RGBA channel");
    if (!std::isfinite(material.depth_bias)) return failure("material depth bias must be finite");
    if (material.lighting.directional_light_count > 4)
        return failure("material directional light count exceeds four");
    if (material.fog.enabled && (!std::isfinite(material.fog.start) || !std::isfinite(material.fog.end)
        || material.fog.start < 0.0F || material.fog.end <= material.fog.start))
        return failure("material fog range must be finite, nonnegative, and ordered");
    if (material.alpha_test.enabled && (!std::isfinite(material.alpha_test.reference)
        || material.alpha_test.reference < 0.0F || material.alpha_test.reference > 1.0F))
        return failure("material alpha-test reference must be in range 0..1");
    for (UInt8 index = 0; index < material.texture_stage_count; ++index) {
        const auto& stage = material.texture_stages[index];
        if (!valid_address(stage.address_u) || !valid_address(stage.address_v))
            return failure("material texture stage " + std::to_string(index) + " has an unknown address mode");
    }
    return {};
}

renderer::ValidationResult validate(const WorldScene& scene)
{
    if (scene.map_name.empty()) return failure("world scene map name must not be empty");
    if (scene.items.empty()) return failure("world scene must contain at least one item");
    if (auto result = validate(scene.camera); !result) return result;
    for (std::size_t index = 0; index < scene.items.size(); ++index) {
        const auto& item = scene.items[index];
        const auto prefix = "world item " + std::to_string(index) + ": ";
        if (item.label.empty()) return failure(prefix + "label must not be empty");
        if (!valid_family(item.family)) return failure(prefix + "unmapped geometry family");
        if (!valid_faction(item.faction)) return failure(prefix + "unmapped faction");
        if (item.vertex_count == 0 || item.index_count == 0)
            return failure(prefix + "vertex and index counts must be nonzero");
        if (item.vertex_count > 1'000'000 || item.index_count > 3'000'000)
            return failure(prefix + "stream geometry exceeds bounded map limits");
        if (item.family == GeometryFamily::faction_geometry && item.faction == Faction::neutral)
            return failure(prefix + "faction geometry requires usa, china, or gla");
        if (item.family != GeometryFamily::animated_mesh && item.animation_frame != 0)
            return failure(prefix + "animation frame is only valid for animated geometry");
        if (auto result = validate(item.transform); !result) return failure(prefix + result.error);
        if (auto result = validate(item.material); !result) return failure(prefix + result.error);
    }
    return {};
}

renderer::ValidationResult translate_material(
    const LegacyMaterialState& material,
    GeometryFamily family,
    renderer::ShaderHandle vertex_shader,
    renderer::ShaderHandle fragment_shader,
    MaterialTranslation& output)
{
    if (!valid_family(family)) return failure("cannot translate unmapped geometry family");
    if (!vertex_shader || !fragment_shader) return failure("material translation requires both shader handles");
    if (auto result = validate(material); !result) return result;

    MaterialTranslation translated;
    translated.pipeline.vertex_shader = vertex_shader;
    translated.pipeline.fragment_shader = fragment_shader;
    translated.pipeline.vertex_layout = vertex_layout(family);
    translated.pipeline.topology = renderer::PrimitiveTopology::triangle_list;
    translated.pipeline.blend.enabled = material.blending;
    translated.pipeline.blend.source_color = material.source_blend;
    translated.pipeline.blend.destination_color = material.destination_blend;
    translated.pipeline.blend.source_alpha = material.source_blend;
    translated.pipeline.blend.destination_alpha = material.destination_blend;
    translated.pipeline.blend.color_write_mask = material.color_write_mask;
    translated.pipeline.depth_stencil.depth_test = material.depth_test;
    translated.pipeline.depth_stencil.depth_write = material.depth_write;
    translated.pipeline.depth_stencil.depth_compare = material.depth_compare;
    translated.pipeline.raster.cull = material.cull;
    translated.pipeline.raster.front_face = renderer::RendererConventions::front_face;
    translated.pipeline.raster.depth_bias = material.depth_bias;
    translated.pipeline.fog_enabled = material.fog.enabled;
    translated.features.lighting = material.lighting.enabled;
    translated.features.fog = material.fog.enabled;
    translated.features.alpha_test = material.alpha_test.enabled;
    translated.features.alpha_compare = material.alpha_test.enabled ? material.alpha_test.compare : renderer::CompareOp::always;
    translated.features.alpha_reference = material.alpha_test.enabled ? material.alpha_test.reference : 0.0F;
    translated.features.texture_stage_count = material.texture_stage_count;
    for (UInt8 index = 0; index < material.texture_stage_count; ++index) {
        translated.samplers[index].min_filter = material.texture_stages[index].filter;
        translated.samplers[index].mag_filter = material.texture_stages[index].filter;
        translated.samplers[index].mip_filter = material.texture_stages[index].filter;
        translated.samplers[index].address_u = material.texture_stages[index].address_u;
        translated.samplers[index].address_v = material.texture_stages[index].address_v;
    }
    if (auto result = renderer::validate(translated.pipeline); !result)
        return failure("translated world pipeline: " + result.error);
    output = translated;
    return {};
}

} // namespace zh::world
