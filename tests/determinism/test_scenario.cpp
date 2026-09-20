#include "zh/persistence/save.h"
#include "zh/simulation/determinism.h"

#include <cfenv>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <string_view>

using namespace zh;

namespace {

int failures = 0;

void check(bool condition, const char* message)
{
    if (!condition) { std::cerr << "FAIL: " << message << '\n'; ++failures; }
}

simulation::ScenarioConfig config(foundation::UInt32 final_tick = 8)
{
    return {u"asset-free-skirmish", u"seed=42;players=2;rate=30", 42, final_tick, 2};
}

std::vector<persistence::ReplayCommand> commands()
{
    return {
        {1, persistence::ReplayCommandKind::move, 0, 1, 4, u"advance"},
        {2, persistence::ReplayCommandKind::damage, 1, 1, -75, u"impact"},
        {4, persistence::ReplayCommandKind::grant_resource, 0, 0, 250, u"supply"},
        {5, persistence::ReplayCommandKind::move, 1, 2, -8, u"retreat"},
        {7, persistence::ReplayCommandKind::damage, 0, 2, -125, u"counter"},
    };
}

bool same_checkpoints(
    const std::vector<persistence::ReplayCheckpoint>& left,
    const std::vector<persistence::ReplayCheckpoint>& right)
{
    if (left.size() != right.size()) return false;
    for (std::size_t index = 0; index < left.size(); ++index) {
        if (left[index].tick != right[index].tick || left[index].crc32 != right[index].crc32) return false;
    }
    return true;
}

template <typename Function>
void rejects(Function&& function, std::string_view diagnostic)
{
    try { function(); check(false, "determinism failure accepted"); }
    catch (const simulation::DeterminismError& error) {
        check(std::string_view(error.what()).find(diagnostic) != std::string_view::npos, "determinism diagnostic");
    }
}

} // namespace

int main(int argc, char** argv)
{
    std::optional<std::filesystem::path> output_path;
    if (argc == 3 && std::string_view(argv[1]) == "--emit-result") output_path = argv[2];
    else if (argc != 1) { std::cerr << "usage: determinism_scenario_tests [--emit-result PATH]\n"; return 2; }

    check(std::fesetround(FE_TONEAREST) == 0, "establish round-to-nearest");
    const auto scenario_config = config();
    const auto scenario_commands = commands();
    const auto first = simulation::run_scenario(scenario_config, scenario_commands);
    const auto second = simulation::run_scenario(scenario_config, scenario_commands);
    check(same_checkpoints(first.checkpoints, second.checkpoints), "repeat run checkpoints match");

    persistence::Replay replay{scenario_config.scenario, scenario_config.configuration,
        scenario_commands, first.checkpoints};
    const auto replay_bytes = persistence::encode_replay(replay);
    persistence::Replay decoded_replay;
    persistence::decode_replay({replay_bytes.data(), replay_bytes.size()}, decoded_replay);
    try { simulation::verify_replay(scenario_config, decoded_replay); }
    catch (const std::exception& error) { std::cerr << error.what() << '\n'; check(false, "replay checkpoints match"); }

    const std::vector<persistence::ReplayCommand> first_half_commands(
        scenario_commands.begin(), scenario_commands.begin() + 3);
    const auto first_half = simulation::run_scenario(config(4), first_half_commands);
    auto save = simulation::save_from_snapshot(first_half.final_snapshot, scenario_config.scenario,
        persistence::AutosaveMetadata{1, 1'700'000'000ULL, u"determinism checkpoint"});
    const auto save_bytes = persistence::encode_save(save);
    persistence::SaveState loaded;
    persistence::decode_save({save_bytes.data(), save_bytes.size()}, loaded);
    const auto resumed = simulation::run_scenario(scenario_config, scenario_commands,
        simulation::snapshot_from_save(loaded));
    auto combined = first_half.checkpoints;
    combined.insert(combined.end(), resumed.checkpoints.begin(), resumed.checkpoints.end());
    check(same_checkpoints(first.checkpoints, combined), "save/load continuation checkpoints match");

    auto divergent = replay;
    ++divergent.checkpoints[1].crc32;
    rejects([&] { simulation::verify_replay(scenario_config, divergent); },
        "scenario=asset-free-skirmish config=seed=42;players=2;rate=30 checkpoint=4");

    check(std::fesetround(FE_DOWNWARD) == 0, "change rounding mode");
    rejects([&] { (void)simulation::run_scenario(scenario_config, scenario_commands); }, "FE_TONEAREST before mutation");
    check(std::fesetround(FE_TONEAREST) == 0, "restore round-to-nearest after negative test");

    simulation::with_isolated_floating_environment([] { std::fesetround(FE_UPWARD); });
    check(std::fegetround() == FE_TONEAREST, "third-party rounding change isolated");
    try {
        simulation::with_isolated_floating_environment([] {
            std::fesetround(FE_DOWNWARD);
            throw std::runtime_error("third-party failure");
        });
        check(false, "throwing third-party callback returned");
    } catch (const std::runtime_error&) {
        check(std::fegetround() == FE_TONEAREST, "throwing callback floating environment restored");
    }

    auto bad_ticks = scenario_config;
    bad_ticks.final_tick = simulation::maximum_scenario_ticks + 1;
    rejects([&] { (void)simulation::run_scenario(bad_ticks, scenario_commands); }, "tick count exceeds limit");
    auto bad_commands = scenario_commands;
    bad_commands[1].tick = 0;
    rejects([&] { (void)simulation::run_scenario(scenario_config, bad_commands); }, "ticks are decreasing");
    bad_commands = scenario_commands;
    bad_commands[0].kind = static_cast<persistence::ReplayCommandKind>(99);
    rejects([&] { (void)simulation::run_scenario(scenario_config, bad_commands); }, "unsupported scenario command kind");

    const auto report = simulation::format_checkpoint_report(scenario_config, first.checkpoints);
    if (output_path) {
        std::ofstream output(*output_path, std::ios::binary | std::ios::trunc);
        output << report;
        check(static_cast<bool>(output), "write deterministic result");
    }
    std::cout << report;
    return failures == 0 ? 0 : 1;
}
