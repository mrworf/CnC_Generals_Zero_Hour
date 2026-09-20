#include "zh/world/recorder.h"

#include <algorithm>
#include <iostream>
#include <stdexcept>
#include <string>

using namespace zh;

namespace {

void check(bool condition, const char* message) { if (!condition) throw std::runtime_error(message); }

world::WorldScene representative_scene(bool reverse = false)
{
    world::WorldScene scene;
    scene.map_name = "Tournament Desert factions";
    for (unsigned value = 0; value <= static_cast<unsigned>(world::GeometryFamily::selection_marker); ++value) {
        world::WorldItem item;
        item.family = static_cast<world::GeometryFamily>(value);
        item.label = std::string(world::geometry_family_name(item.family));
        item.vertex_count = 3 + value;
        item.index_count = 3 + value * 3;
        item.animation_frame = item.family == world::GeometryFamily::animated_mesh ? 12 : 0;
        item.faction = item.family == world::GeometryFamily::faction_geometry ? world::Faction::usa : world::Faction::neutral;
        item.material.fog.enabled = value % 2 == 0;
        item.material.fog.start = 10.0F;
        item.material.fog.end = 900.0F;
        item.material.alpha_test.enabled = item.family == world::GeometryFamily::tree;
        item.material.texture_stage_count = item.family == world::GeometryFamily::terrain ? 2 : 1;
        item.material.blending = item.family == world::GeometryFamily::road_decal
            || item.family == world::GeometryFamily::selection_marker;
        scene.items.push_back(item);
    }
    for (auto faction : {world::Faction::china, world::Faction::gla}) {
        auto item = scene.items[static_cast<unsigned>(world::GeometryFamily::faction_geometry)];
        item.faction = faction;
        item.label = "faction-" + std::string(world::faction_name(faction));
        scene.items.push_back(item);
    }
    if (reverse) std::reverse(scene.items.begin(), scene.items.end());
    return scene;
}

std::string record_scene(bool reverse)
{
    renderer::RecordingGpuDevice device(32);
    world::WorldRecorder recorder(device);
    check(recorder.load(representative_scene(reverse)), "representative scene load failed");
    check(recorder.record_frame(7), "representative frame failed");
    check(recorder.record_frame(8), "second representative frame failed");
    check(recorder.owned_pipeline_count() > 0 && device.pipeline_count() == recorder.owned_pipeline_count(),
        "pipeline ownership accounting diverged");
    check(recorder.teardown(), "representative teardown failed");
    check(device.pipeline_count() == 0, "pipeline handles leaked after teardown");
    return device.snapshot();
}

void test_deterministic_complete_command_stream()
{
    const auto first = record_scene(false);
    const auto reordered = record_scene(true);
    check(first == reordered, "caller item order changed normalized command stream");
    check(first.find("render-target dependency shadow-map -> main-world") != std::string::npos,
        "render-target dependency was not recorded");
    check(first.find("begin_pass label=\"world shadow dependency\"") < first.find("begin_pass label=\"world main pass\""),
        "shadow dependency pass did not precede the main pass");
    for (unsigned value = 0; value <= static_cast<unsigned>(world::GeometryFamily::selection_marker); ++value) {
        const auto marker = "family=" + std::string(world::geometry_family_name(static_cast<world::GeometryFamily>(value)));
        check(first.find(marker) != std::string::npos, "world family missing from command stream");
    }
    for (auto faction : {world::Faction::usa, world::Faction::china, world::Faction::gla}) {
        const auto marker = "faction=" + std::string(world::faction_name(faction));
        check(first.find(marker) != std::string::npos, "faction missing from command stream");
    }
    check(first.find("animation-frame=12") != std::string::npos, "animation-visible geometry state missing");
    check(first.find("texture-stages=2") != std::string::npos, "terrain texture-stage state missing");
    check(first.find("alpha-test=on") != std::string::npos, "tree alpha-test state missing");
    check(first.find("world-camera handedness=left depth=0..1 winding=clockwise origin=top-left") != std::string::npos,
        "camera/coordinate conventions missing");
    const auto shadow = first.find("family=shadow");
    const auto shadow_draw = first.find("fragment_textures=T1/S1,T2/S2,T3/S3,T1/S4", shadow);
    check(shadow != std::string::npos && shadow_draw != std::string::npos && shadow < shadow_draw,
        "shadow pass sampled its active shadow-map attachment");
    const auto terrain = first.find("family=terrain");
    const auto terrain_draw = first.find("fragment_textures=T1/S1,T2/S2,T3/S3,T6/S4", terrain);
    check(terrain != std::string::npos && terrain_draw != std::string::npos && terrain < terrain_draw,
        "main world pass did not sample the completed shadow map");
}

void test_lifecycle_errors_recover()
{
    renderer::RecordingGpuDevice device(32);
    world::WorldRecorder recorder(device);
    check(!recorder.record_frame(0) && recorder.last_error().find("loaded map") != std::string::npos,
        "frame without load was accepted");
    check(recorder.load(representative_scene()), "load after rejected frame failed");
    check(!recorder.load(representative_scene()) && recorder.last_error().find("teardown") != std::string::npos,
        "double load was accepted");
    check(recorder.record_frame(9), "first frame failed");
    check(!recorder.record_frame(8) && recorder.last_error().find("regressed") != std::string::npos,
        "tick regression was accepted");
    check(recorder.record_frame(10), "recorder did not recover after tick rejection");
    check(recorder.teardown(), "teardown failed");
    check(!recorder.teardown(), "double teardown was accepted");
    check(recorder.load(representative_scene()), "reload after teardown failed");
    check(recorder.record_frame(0), "frame after reload failed");
    check(recorder.teardown(), "second teardown failed");
}

void test_pipeline_capacity_failure_is_bounded()
{
    renderer::RecordingGpuDevice device(1);
    world::WorldRecorder recorder(device);
    check(!recorder.load(representative_scene()), "insufficient pipeline capacity was accepted");
    check(recorder.last_error().find("capacity exceeded") != std::string::npos, "capacity diagnostic was not actionable");
    check(!recorder.loaded() && device.pipeline_count() == 0, "failed load retained partial map/pipelines");
}

} // namespace

int main()
{
    try {
        test_deterministic_complete_command_stream();
        test_lifecycle_errors_recover();
        test_pipeline_capacity_failure_is_bounded();
        std::cout << "world recorder tests: ok\n";
        return 0;
    } catch (const std::exception& error) {
        std::cerr << error.what() << '\n';
        return 1;
    }
}
