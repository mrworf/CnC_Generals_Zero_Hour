#include "zh/simulation/determinism.h"

#include "zh/foundation/unicode.h"

#include <cfenv>
#include <cmath>
#include <limits>
#include <sstream>

namespace zh::simulation {
namespace {

using namespace foundation;
using namespace persistence;

void validate_text(std::u16string_view value, const char* field)
{
    if (value.size() > maximum_save_text_code_units) {
        throw DeterminismError(std::string(field) + " exceeds UTF-16 code-unit limit");
    }
    try { (void)foundation::utf16_to_utf8(value); }
    catch (const foundation::UnicodeError& error) {
        throw DeterminismError(std::string(field) + " contains malformed UTF-16: " + error.what());
    }
}

void require_round_to_nearest()
{
    if (std::fegetround() != FE_TONEAREST) {
        throw DeterminismError("simulation requires FE_TONEAREST before mutation");
    }
}

Snapshot starting_snapshot(const ScenarioConfig& config)
{
    return {0, config.seed, 0, {
        {1, 0, 1000, 0.0F, 0.0F, u"Alpha"},
        {2, 1, 1000, 8.0F, -8.0F, u"Bravo"},
    }};
}

EntityState& find_entity(Snapshot& state, UInt32 id)
{
    for (auto& entity : state.entities) if (entity.id == id) return entity;
    throw DeterminismError("scenario command names unknown entity " + std::to_string(id));
}

Int32 checked_add(Int32 left, Int32 right, const char* field)
{
    const Int64 value = static_cast<Int64>(left) + right;
    if (value < std::numeric_limits<Int32>::min() || value > std::numeric_limits<Int32>::max()) {
        throw DeterminismError(std::string(field) + " overflow");
    }
    return static_cast<Int32>(value);
}

void apply_command(Snapshot& state, const ReplayCommand& command)
{
    switch (command.kind) {
    case ReplayCommandKind::move: {
        auto& entity = find_entity(state, command.target);
        entity.x += static_cast<float>(command.value) * 0.25F;
        if (!std::isfinite(entity.x)) throw DeterminismError("move produced non-finite position");
        break;
    }
    case ReplayCommandKind::damage: {
        auto& entity = find_entity(state, command.target);
        entity.health = checked_add(entity.health, command.value, "entity health");
        break;
    }
    case ReplayCommandKind::grant_resource:
        state.score = checked_add(state.score, command.value, "scenario score");
        break;
    default:
        throw DeterminismError("unsupported scenario command kind");
    }
}

std::string checkpoint_context(const ScenarioConfig& config, UInt32 tick)
{
    return "scenario=" + foundation::utf16_to_utf8(config.scenario) +
        " config=" + foundation::utf16_to_utf8(config.configuration) +
        " checkpoint=" + std::to_string(tick);
}

} // namespace

ScenarioRun run_scenario(
    const ScenarioConfig& config,
    const std::vector<ReplayCommand>& commands,
    const std::optional<Snapshot>& initial)
{
    require_round_to_nearest();
    validate_text(config.scenario, "scenario name");
    validate_text(config.configuration, "scenario configuration");
    if (config.final_tick > maximum_scenario_ticks) throw DeterminismError("scenario tick count exceeds limit");
    if (config.checkpoint_interval == 0) throw DeterminismError("checkpoint interval must be nonzero");
    if (commands.size() > maximum_replay_commands) throw DeterminismError("scenario command count exceeds limit");

    Snapshot state = initial.value_or(starting_snapshot(config));
    if (state.tick > config.final_tick) throw DeterminismError("initial snapshot is beyond final tick");
    UInt32 prior_tick = 0;
    for (std::size_t index = 0; index < commands.size(); ++index) {
        const auto& command = commands[index];
        if (index != 0 && command.tick < prior_tick) throw DeterminismError("scenario command ticks are decreasing");
        if (command.tick == 0 || command.tick > config.final_tick) throw DeterminismError("scenario command tick is outside run");
        prior_tick = command.tick;
    }

    ScenarioRun result;
    std::size_t command_index = 0;
    while (command_index < commands.size() && commands[command_index].tick <= state.tick) ++command_index;
    for (UInt32 tick = state.tick + 1; tick <= config.final_tick; ++tick) {
        require_round_to_nearest();
        state.tick = tick;
        state.random_state = state.random_state * 6364136223846793005ULL + 1442695040888963407ULL;
        const Int32 drift = static_cast<Int32>((state.random_state >> 60U) & 0x0fU) - 8;
        find_entity(state, 1).y += static_cast<float>(drift) * 0.125F;
        while (command_index < commands.size() && commands[command_index].tick == tick) {
            apply_command(state, commands[command_index++]);
        }
        if (tick % config.checkpoint_interval == 0 || tick == config.final_tick) {
            result.checkpoints.push_back(make_checkpoint(state));
        }
    }
    result.final_snapshot = std::move(state);
    return result;
}

void verify_replay(const ScenarioConfig& config, const Replay& replay)
{
    if (replay.scenario != config.scenario || replay.configuration != config.configuration) {
        throw DeterminismError("replay scenario/config does not match requested run");
    }
    const auto actual = run_scenario(config, replay.commands);
    if (actual.checkpoints.size() != replay.checkpoints.size()) {
        throw DeterminismError("replay checkpoint count divergence for scenario=" +
            foundation::utf16_to_utf8(config.scenario) + " config=" +
            foundation::utf16_to_utf8(config.configuration) + ": expected=" +
            std::to_string(replay.checkpoints.size()) + " actual=" + std::to_string(actual.checkpoints.size()));
    }
    for (std::size_t index = 0; index < actual.checkpoints.size(); ++index) {
        const auto& expected = replay.checkpoints[index];
        const auto& observed = actual.checkpoints[index];
        if (expected.tick != observed.tick || expected.crc32 != observed.crc32) {
            throw DeterminismError("replay CRC divergence " + checkpoint_context(config, observed.tick) +
                " expected=" + std::to_string(expected.crc32) + " actual=" + std::to_string(observed.crc32));
        }
    }
}

SaveState save_from_snapshot(
    const Snapshot& snapshot,
    std::u16string scenario,
    std::optional<AutosaveMetadata> autosave)
{
    SaveState save;
    save.scenario = std::move(scenario);
    save.tick = snapshot.tick;
    save.random_state = snapshot.random_state;
    save.score = snapshot.score;
    save.entities = snapshot.entities;
    save.autosave = std::move(autosave);
    return save;
}

Snapshot snapshot_from_save(const SaveState& save)
{
    return {save.tick, save.random_state, save.score, save.entities};
}

void with_isolated_floating_environment(const std::function<void()>& callback)
{
    if (!callback) throw DeterminismError("floating-environment callback is empty");
    std::fenv_t simulation_environment;
    if (std::fegetenv(&simulation_environment) != 0) {
        throw DeterminismError("cannot capture simulation floating environment");
    }
    try {
        callback();
    } catch (...) {
        if (std::fesetenv(&simulation_environment) != 0) std::terminate();
        throw;
    }
    if (std::fesetenv(&simulation_environment) != 0) {
        throw DeterminismError("cannot restore simulation floating environment");
    }
}

std::string format_checkpoint_report(
    const ScenarioConfig& config,
    const std::vector<ReplayCheckpoint>& checkpoints)
{
    std::ostringstream output;
    output << "format=zh-determinism-v1\n";
    output << "scenario=" << foundation::utf16_to_utf8(config.scenario) << '\n';
    output << "config=" << foundation::utf16_to_utf8(config.configuration) << '\n';
    for (const auto& checkpoint : checkpoints) {
        output << "checkpoint=" << checkpoint.tick << " crc=" << checkpoint.crc32 << '\n';
    }
    return output.str();
}

} // namespace zh::simulation
