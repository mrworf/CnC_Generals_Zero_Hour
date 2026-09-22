#include "zh/world/corpus.h"
#include "zh/world/recorder.h"

#include <fstream>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>

using namespace zh;

namespace {

void check(bool condition, const char* message) { if (!condition) throw std::runtime_error(message); }

std::string manifest_text()
{
    std::ifstream input(std::string(ZH_SOURCE_DIR) + "/data/corpus/world-rendering-manifest.tsv", std::ios::binary);
    if (!input) throw std::runtime_error("cannot open project-owned world corpus manifest");
    std::ostringstream contents;
    contents << input.rdbuf();
    return contents.str();
}

world::WorldScene scene_from(const world::WorldCorpusCoverage& coverage)
{
    world::WorldScene scene;
    scene.map_name = coverage.maps.front();
    for (auto family : coverage.families) {
        world::WorldItem item;
        item.family = family;
        item.label = std::string(world::geometry_family_name(family));
        item.animation_frame = family == world::GeometryFamily::animated_mesh ? 1 : 0;
        item.faction = family == world::GeometryFamily::faction_geometry ? world::Faction::usa : world::Faction::neutral;
        item.material.fog.enabled = family == world::GeometryFamily::terrain;
        item.material.alpha_test.enabled = family == world::GeometryFamily::tree;
        item.material.blending = family == world::GeometryFamily::road_decal || family == world::GeometryFamily::selection_marker;
        scene.items.push_back(item);
    }
    for (auto faction : {world::Faction::china, world::Faction::gla}) {
        world::WorldItem item;
        item.family = world::GeometryFamily::faction_geometry;
        item.faction = faction;
        item.label = "faction-" + std::string(world::faction_name(faction));
        scene.items.push_back(item);
    }
    return scene;
}

void test_complete_logical_corpus()
{
    world::WorldCorpusCoverage coverage;
    check(world::parse_world_corpus(manifest_text(), coverage), "checked-in world corpus was rejected");
    check(coverage.maps.size() == 1, "representative map count changed");
    check(coverage.factions.size() == 3, "faction corpus is incomplete");
    check(coverage.families.size() == 8, "world family corpus is incomplete");
    check(coverage.states.size() == 13, "fixed-function/state corpus is incomplete");
    check(world::validate(scene_from(coverage)), "corpus could not produce a complete representative scene");
}

void test_unmapped_duplicate_and_missing_state_fail_closed()
{
    const auto complete = manifest_text();
    world::WorldCorpusCoverage output;
    check(!world::parse_world_corpus(complete + "state\tbump-environment\tnot in M9\n", output),
        "unmapped required state was accepted");
    check(!world::parse_world_corpus(complete + "faction\tusa\tduplicate\n", output), "duplicate mapping was accepted");
    check(!world::parse_world_corpus("kind-without-tabs\n", output), "malformed row was accepted");

    auto missing = complete;
    const auto row = std::string("family\ttree\talpha-tested vegetation commands\n");
    missing.erase(missing.find(row), row.size());
    check(!world::parse_world_corpus(missing, output), "missing world family was accepted");
    check(output.maps.empty(), "failed parse exposed partial coverage");
}

void test_resources_are_bounded_across_frames_and_reload()
{
    world::WorldCorpusCoverage coverage;
    check(world::parse_world_corpus(manifest_text(), coverage), "world corpus parse failed");
    renderer::RecordingGpuDevice device(32);
    world::WorldRecorder recorder(device);
    const auto scene = scene_from(coverage);

    check(recorder.load(scene), "first world load failed");
    const auto first_load = device.resource_counts();
    check(first_load.buffers == 5 && first_load.textures == 7 && first_load.samplers == 4
            && first_load.shaders == 2 && first_load.pipelines > 0 && first_load.pipelines <= 16,
        "world load exceeded declared resource bounds");
    for (renderer::UInt32 tick = 0; tick < 64; ++tick) {
        check(recorder.record_frame(tick), "repeated world frame failed");
        check(device.present(recorder.color_target()), "completed world frame was not presented");
        check(device.resource_counts() == first_load, "repeated frame grew the live resource set");
    }
    check(recorder.teardown(), "first teardown failed");
    check(!recorder.color_target(), "teardown exposed a stale world color target");
    check(device.resource_counts().total() == 0, "first teardown leaked renderer handles");

    check(recorder.load(scene), "world reload failed");
    const auto second_load = device.resource_counts();
    check(second_load == first_load, "reload changed the bounded live resource set");
    check(recorder.record_frame(0) && device.present(recorder.color_target())
            && recorder.record_frame(1) && device.present(recorder.color_target()), "reloaded frames failed");
    check(device.resource_counts() == first_load, "reload frames grew the live resource set");
    check(recorder.teardown(), "reload teardown failed");
    check(!recorder.color_target(), "reload teardown exposed a stale world color target");
    check(device.resource_counts().total() == 0, "reload teardown leaked renderer handles");
}

} // namespace

int main()
{
    try {
        test_complete_logical_corpus();
        test_unmapped_duplicate_and_missing_state_fail_closed();
        test_resources_are_bounded_across_frames_and_reload();
        std::cout << "world corpus/lifecycle tests: ok\n";
        return 0;
    } catch (const std::exception& error) {
        std::cerr << error.what() << '\n';
        return 1;
    }
}
