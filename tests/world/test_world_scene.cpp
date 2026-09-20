#include "zh/world/scene.h"

#include <cmath>
#include <iostream>
#include <stdexcept>
#include <string>

using namespace zh;

namespace {

void check(bool condition, const char* message) { if (!condition) throw std::runtime_error(message); }

world::WorldScene complete_scene()
{
    world::WorldScene scene;
    scene.map_name = "Tournament Desert";
    for (unsigned value = 0; value <= static_cast<unsigned>(world::GeometryFamily::selection_marker); ++value) {
        world::WorldItem item;
        item.family = static_cast<world::GeometryFamily>(value);
        item.label = std::string(world::geometry_family_name(item.family));
        item.faction = item.family == world::GeometryFamily::faction_geometry ? world::Faction::usa : world::Faction::neutral;
        item.animation_frame = item.family == world::GeometryFamily::animated_mesh ? 7 : 0;
        item.material.fog.enabled = true;
        item.material.fog.start = 20.0F;
        item.material.fog.end = 600.0F;
        scene.items.push_back(item);
    }
    return scene;
}

void test_complete_world_vocabulary()
{
    auto scene = complete_scene();
    check(world::validate(scene), "complete representative scene was rejected");
    check(world::geometry_family_name(world::GeometryFamily::road_decal) == "road-decal", "road/decal mapping changed");
    check(world::faction_name(world::Faction::gla) == "gla", "faction mapping changed");
    check(world::vertex_layout(world::GeometryFamily::terrain) == renderer::VertexLayout::terrain, "terrain layout changed");
    check(world::vertex_layout(world::GeometryFamily::tree) == renderer::VertexLayout::world_mesh, "mesh layout changed");
}

void test_fixed_function_translation()
{
    world::LegacyMaterialState state;
    state.blending = true;
    state.color_write_mask = 0x07;
    state.depth_write = false;
    state.cull = renderer::CullMode::clockwise;
    state.depth_bias = -0.25F;
    state.fog = {true, 10.0F, 900.0F, renderer::pack_argb8(255, 1, 2, 3)};
    state.alpha_test = {true, renderer::CompareOp::greater_equal, 0.375F};
    state.texture_stage_count = 4;
    state.texture_stages[1].address_u = renderer::AddressMode::clamp_edge;
    state.texture_stages[2].address_v = renderer::AddressMode::mirrored_repeat;
    state.texture_stages[3].filter = renderer::Filter::nearest;

    world::MaterialTranslation first, second;
    const renderer::ShaderHandle vertex(1), fragment(2);
    check(world::translate_material(state, world::GeometryFamily::terrain, vertex, fragment, first), "translation failed");
    check(world::translate_material(state, world::GeometryFamily::terrain, vertex, fragment, second), "repeat translation failed");
    check(first.pipeline.vertex_layout == renderer::VertexLayout::terrain, "terrain pipeline layout missing");
    check(first.pipeline.blend.enabled && first.pipeline.blend.color_write_mask == 0x07, "blend/color mask not translated");
    check(!first.pipeline.depth_stencil.depth_write && first.pipeline.raster.cull == renderer::CullMode::clockwise,
        "depth/cull state not translated");
    check(first.pipeline.fog_enabled && first.features.lighting && first.features.fog, "lighting/fog feature missing");
    check(first.features.alpha_test && first.features.alpha_compare == renderer::CompareOp::greater_equal,
        "alpha-test feature missing");
    check(first.features.texture_stage_count == 4 && first.samplers[3].min_filter == renderer::Filter::nearest,
        "texture stages/addressing not translated");
    check(renderer::PipelineKey(first.pipeline) == renderer::PipelineKey(second.pipeline), "translation is not deterministic");
}

void test_conventions_and_fail_closed_validation()
{
    check(renderer::RendererConventions::handedness == renderer::CoordinateHandedness::left_handed, "handedness changed");
    check(renderer::RendererConventions::depth_minimum == 0.0F && renderer::RendererConventions::depth_maximum == 1.0F,
        "depth convention changed");
    check(renderer::RendererConventions::front_face == renderer::Winding::clockwise, "front face changed");
    check(renderer::RendererConventions::texture_origin == renderer::TextureOrigin::top_left, "texture origin changed");
    check(renderer::RendererConventions::colors_are_argb8 && renderer::RendererConventions::fog_distance_is_view_space,
        "color/fog convention changed");

    auto scene = complete_scene();
    scene.camera.viewport_width = 0;
    check(!world::validate(scene), "zero viewport accepted");
    scene = complete_scene();
    scene.camera.near_depth = 2.0F; scene.camera.far_depth = 1.0F;
    check(!world::validate(scene), "reversed camera depth accepted");
    scene = complete_scene();
    scene.items[0].family = static_cast<world::GeometryFamily>(255);
    check(!world::validate(scene), "unknown geometry family accepted");
    scene = complete_scene();
    scene.items[0].transform.elements[0] = std::nanf("");
    check(!world::validate(scene), "non-finite transform accepted");
    scene = complete_scene();
    scene.items[0].material.texture_stage_count = 5;
    check(!world::validate(scene), "excess texture stages accepted");
    scene = complete_scene();
    scene.items[0].material.alpha_test = {true, renderer::CompareOp::greater, 1.1F};
    check(!world::validate(scene), "out-of-range alpha reference accepted");
    scene = complete_scene();
    scene.items[0].material.fog = {true, 30.0F, 20.0F, 0};
    check(!world::validate(scene), "reversed fog range accepted");

    world::MaterialTranslation output;
    check(!world::translate_material({}, world::GeometryFamily::mesh, {}, renderer::ShaderHandle(2), output),
        "missing shader accepted");
}

} // namespace

int main()
{
    try {
        test_complete_world_vocabulary();
        test_fixed_function_translation();
        test_conventions_and_fail_closed_validation();
        std::cout << "world scene contract tests: ok\n";
        return 0;
    } catch (const std::exception& error) {
        std::cerr << error.what() << '\n';
        return 1;
    }
}
