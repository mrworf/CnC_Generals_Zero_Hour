#include "zh/singleplayer/session.h"

#include "zh/foundation/unicode.h"

#include <algorithm>
#include <cstdlib>
#include <fstream>
#include <sstream>
#include <system_error>

namespace zh::singleplayer {
namespace {

foundation::EnvironmentLookup process_environment()
{
    return [](std::string_view name) -> std::optional<std::string> {
        const std::string key(name);
        const char* value = std::getenv(key.c_str());
        return value == nullptr ? std::nullopt : std::optional<std::string>(value);
    };
}

std::u16string utf16(std::string_view value)
{
    try { return foundation::utf8_to_utf16(value); }
    catch (const foundation::UnicodeError& error) {
        throw SessionError(std::string("single-player scenario text is invalid UTF-8: ") + error.what());
    }
}

void create_owned_directory(const std::filesystem::path& path, std::string_view purpose)
{
    std::error_code error;
    std::filesystem::create_directories(path, error);
    if (error) throw SessionError("single-player cannot create " + std::string(purpose) + " directory: " + error.message());
}

bool valid_owned_name(std::string_view value)
{
    if (value.empty() || value.size() > 64) return false;
    return std::all_of(value.begin(), value.end(), [](unsigned char character) {
        return (character >= 'a' && character <= 'z') || (character >= 'A' && character <= 'Z') ||
            (character >= '0' && character <= '9') || character == '-' || character == '_';
    });
}

std::string configuration(const Request& request)
{
    return "mode=" + std::string(mode_name(request.mode)) + ";faction=" +
        std::string(faction_name(request.faction)) + ";seed=" + std::to_string(request.seed);
}

} // namespace

std::string_view mode_name(Mode mode) noexcept
{
    switch (mode) {
    case Mode::campaign: return "campaign";
    case Mode::skirmish: return "skirmish";
    case Mode::user_map: return "user-map";
    }
    return "unknown";
}

std::string_view faction_name(Faction faction) noexcept
{
    switch (faction) {
    case Faction::usa: return "usa";
    case Faction::china: return "china";
    case Faction::gla: return "gla";
    }
    return "unknown";
}

std::string_view state_name(SessionState state) noexcept
{
    switch (state) {
    case SessionState::created: return "created";
    case SessionState::loading: return "loading";
    case SessionState::playing: return "playing";
    case SessionState::paused: return "paused";
    case SessionState::victory: return "victory";
    case SessionState::defeat: return "defeat";
    case SessionState::stopped: return "stopped";
    case SessionState::error: return "error";
    }
    return "unknown";
}

Session::Session(Request request, const data::VirtualFileSystem* vfs, foundation::EnvironmentLookup environment)
    : request_(std::move(request)), vfs_(vfs), environment_(std::move(environment))
{
}

simulation::ScenarioConfig Session::scenario_config() const
{
    return {utf16(request_.scenario), utf16(configuration(request_)), request_.seed,
        request_.final_tick, request_.checkpoint_interval};
}

std::vector<persistence::ReplayCommand> Session::scenario_commands() const
{
    const auto quarter = std::max<foundation::UInt32>(1, request_.final_tick / 4);
    return {
        {quarter, persistence::ReplayCommandKind::move, 0, 1, 4, u"advance"},
        {quarter * 2, persistence::ReplayCommandKind::grant_resource, 0, 0,
            static_cast<foundation::Int32>(100 + static_cast<int>(request_.faction) * 25), u"supply"},
        {quarter * 3, persistence::ReplayCommandKind::damage, 0, 2, -125, u"attack"},
    };
}

void Session::start()
{
    if (state_ != SessionState::created) throw SessionError("single-player start requires created state");
    state_ = SessionState::loading;
    try {
        if (request_.scenario.empty()) throw SessionError("single-player scenario name is required");
        if (request_.final_tick < 4 || request_.final_tick > simulation::maximum_scenario_ticks) {
            throw SessionError("single-player final tick must be from 4 through 1000000");
        }
        if (request_.checkpoint_interval == 0) throw SessionError("single-player checkpoint interval must be nonzero");
        if (request_.mode == Mode::user_map) {
            if (request_.user_map.empty()) throw SessionError("single-player user-map logical path is required");
            const auto logical = foundation::normalize_logical_path(request_.user_map);
            if (vfs_ == nullptr || vfs_->find(logical) == nullptr) {
                throw SessionError("single-player required user map is missing: " + logical);
            }
            request_.user_map = logical;
        }

        const auto xdg = foundation::resolve_xdg_paths(environment_ ? environment_ : process_environment());
        paths_ = {xdg.config, xdg.data, xdg.state, xdg.cache};
        create_owned_directory(paths_.config, "config");
        create_owned_directory(paths_.data / "saves", "save");
        create_owned_directory(paths_.data / "replays", "replay");
        create_owned_directory(paths_.state / "logs", "state/log");
        create_owned_directory(paths_.cache, "cache");

        const auto config = scenario_config();
        const auto initial = simulation::run_scenario({config.scenario, config.configuration,
            config.seed, 0, config.checkpoint_interval}, {});
        snapshot_ = initial.final_snapshot;
        checkpoints_.clear();
        events_.push_back("loading:" + request_.scenario);
        events_.push_back("playing:" + std::string(mode_name(request_.mode)));
        state_ = SessionState::playing;
        last_error_.clear();
    } catch (const std::exception& error) {
        record_error(error.what());
        throw;
    }
}

void Session::require_started(std::string_view operation) const
{
    if (state_ != SessionState::playing && state_ != SessionState::paused) {
        throw SessionError("single-player " + std::string(operation) + " requires an active session");
    }
}

void Session::advance(foundation::UInt32 ticks)
{
    require_started("advance");
    if (state_ == SessionState::paused || ticks == 0) return;
    const auto remaining = request_.final_tick - snapshot_.tick;
    const auto target = snapshot_.tick + std::min(ticks, remaining);
    if (target == snapshot_.tick) return;
    auto config = scenario_config();
    config.final_tick = target;
    const auto all_commands = scenario_commands();
    std::vector<persistence::ReplayCommand> commands;
    std::copy_if(all_commands.begin(), all_commands.end(), std::back_inserter(commands),
        [&](const auto& command) { return command.tick <= target; });
    const auto run = simulation::run_scenario(config, commands, snapshot_);
    snapshot_ = run.final_snapshot;
    checkpoints_.insert(checkpoints_.end(), run.checkpoints.begin(), run.checkpoints.end());
    events_.push_back("advanced:" + std::to_string(snapshot_.tick));
}

void Session::set_paused(bool paused)
{
    require_started("pause");
    state_ = paused ? SessionState::paused : SessionState::playing;
    events_.push_back(paused ? "paused" : "resumed");
}

std::filesystem::path Session::owned_file(
    std::string_view category, std::string_view name, std::string_view extension) const
{
    if (!valid_owned_name(name)) throw SessionError("single-player owned file name must use 1-64 ASCII letters, digits, '-' or '_'");
    return paths_.data / std::string(category) / (std::string(name) + std::string(extension));
}

void Session::save(std::string_view name, bool autosave)
{
    require_started("save");
    auto value = simulation::save_from_snapshot(snapshot_, scenario_config().scenario);
    if (autosave) value.autosave = persistence::AutosaveMetadata{1, 0, u"Linux autosave"};
    const auto bytes = persistence::encode_save(value);
    try { foundation::write_binary_file_atomic(owned_file("saves", name, ".zhs"), {bytes.data(), bytes.size()}); }
    catch (const std::exception& error) { throw SessionError(std::string("single-player save failed: ") + error.what()); }
    events_.push_back("saved:" + std::string(name));
}

void Session::load(std::string_view name)
{
    require_started("load");
    try {
        const auto bytes = foundation::read_binary_file(owned_file("saves", name, ".zhs"), persistence::maximum_save_bytes);
        persistence::SaveState candidate;
        persistence::decode_save({bytes.data(), bytes.size()}, candidate);
        if (candidate.scenario != scenario_config().scenario) throw SessionError("single-player save scenario does not match active session");
        if (candidate.tick > request_.final_tick) throw SessionError("single-player save tick exceeds active scenario");
        snapshot_ = simulation::snapshot_from_save(candidate);
        checkpoints_.clear();
        events_.push_back("loaded:" + std::string(name));
    } catch (const SessionError&) { throw; }
    catch (const std::exception& error) { throw SessionError(std::string("single-player load failed: ") + error.what()); }
}

void Session::write_replay(std::string_view name)
{
    require_started("replay write");
    const auto config = scenario_config();
    const auto commands = scenario_commands();
    const auto run = simulation::run_scenario(config, commands);
    persistence::Replay replay{config.scenario, config.configuration, commands, run.checkpoints};
    const auto bytes = persistence::encode_replay(replay);
    try { foundation::write_binary_file_atomic(owned_file("replays", name, ".zhr"), {bytes.data(), bytes.size()}); }
    catch (const std::exception& error) { throw SessionError(std::string("single-player replay write failed: ") + error.what()); }
    events_.push_back("replay-written:" + std::string(name));
}

void Session::verify_replay(std::string_view name) const
{
    require_started("replay verification");
    try {
        const auto bytes = foundation::read_binary_file(owned_file("replays", name, ".zhr"), persistence::maximum_replay_bytes);
        persistence::Replay replay;
        persistence::decode_replay({bytes.data(), bytes.size()}, replay);
        simulation::verify_replay(scenario_config(), replay);
    } catch (const std::exception& error) { throw SessionError(std::string("single-player replay verification failed: ") + error.what()); }
}

void Session::finish(Outcome outcome)
{
    require_started("finish");
    if (state_ == SessionState::paused) set_paused(false);
    advance(request_.final_tick - snapshot_.tick);
    state_ = outcome == Outcome::victory ? SessionState::victory : SessionState::defeat;
    events_.push_back(std::string(state_name(state_)));
    const std::string progression = "version=1\nscenario=" + request_.scenario + "\noutcome=" +
        std::string(state_name(state_)) + "\ntick=" + std::to_string(snapshot_.tick) + "\n";
    try {
        foundation::write_binary_file_atomic(paths_.data / "progression.txt",
            {reinterpret_cast<const foundation::UInt8*>(progression.data()), progression.size()});
    } catch (const std::exception& error) {
        throw SessionError(std::string("single-player progression write failed: ") + error.what());
    }
}

void Session::shutdown() noexcept
{
    if (state_ == SessionState::stopped) return;
    events_.push_back("shutdown");
    state_ = SessionState::stopped;
}

void Session::record_error(std::string message)
{
    last_error_ = std::move(message);
    events_.push_back("error:" + last_error_);
    state_ = SessionState::error;
}

} // namespace zh::singleplayer
