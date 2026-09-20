#pragma once

#include "zh/persistence/replay.h"

#include <functional>
#include <optional>
#include <stdexcept>
#include <string>
#include <vector>

namespace zh::simulation {

class DeterminismError : public std::runtime_error {
public:
    using std::runtime_error::runtime_error;
};

struct ScenarioConfig {
    std::u16string scenario;
    std::u16string configuration;
    foundation::UInt64 seed = 0;
    foundation::UInt32 final_tick = 0;
    foundation::UInt32 checkpoint_interval = 1;
};

struct ScenarioRun {
    persistence::Snapshot final_snapshot;
    std::vector<persistence::ReplayCheckpoint> checkpoints;
};

inline constexpr foundation::UInt32 maximum_scenario_ticks = 1'000'000;

ScenarioRun run_scenario(
    const ScenarioConfig& config,
    const std::vector<persistence::ReplayCommand>& commands,
    const std::optional<persistence::Snapshot>& initial = std::nullopt);

void verify_replay(const ScenarioConfig& config, const persistence::Replay& replay);

persistence::SaveState save_from_snapshot(
    const persistence::Snapshot& snapshot,
    std::u16string scenario,
    std::optional<persistence::AutosaveMetadata> autosave = std::nullopt);
persistence::Snapshot snapshot_from_save(const persistence::SaveState& save);

void with_isolated_floating_environment(const std::function<void()>& callback);

std::string format_checkpoint_report(
    const ScenarioConfig& config,
    const std::vector<persistence::ReplayCheckpoint>& checkpoints);

} // namespace zh::simulation
