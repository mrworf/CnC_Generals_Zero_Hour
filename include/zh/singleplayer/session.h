#pragma once

#include "zh/data/vfs.h"
#include "zh/foundation/platform.h"
#include "zh/simulation/determinism.h"

#include <filesystem>
#include <optional>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

namespace zh::singleplayer {

class SessionError : public std::runtime_error {
public:
    using std::runtime_error::runtime_error;
};

enum class Mode { campaign, skirmish, user_map };
enum class Faction { usa, china, gla };
enum class Outcome { victory, defeat };
enum class SessionState { created, loading, playing, paused, victory, defeat, stopped, error };

struct Request {
    Mode mode = Mode::skirmish;
    Faction faction = Faction::usa;
    std::string scenario = "skirmish-usa";
    std::string user_map;
    foundation::UInt64 seed = 42;
    foundation::UInt32 final_tick = 120;
    foundation::UInt32 checkpoint_interval = 30;
};

struct OwnedPaths {
    std::filesystem::path config;
    std::filesystem::path data;
    std::filesystem::path state;
    std::filesystem::path cache;
};

class Session {
public:
    Session(Request request, const data::VirtualFileSystem* vfs = nullptr,
        foundation::EnvironmentLookup environment = {});

    void start();
    void advance(foundation::UInt32 ticks);
    void set_paused(bool paused);
    void save(std::string_view name, bool autosave = false);
    void load(std::string_view name);
    void write_replay(std::string_view name);
    void verify_replay(std::string_view name) const;
    void finish(Outcome outcome);
    void shutdown() noexcept;

    SessionState state() const noexcept { return state_; }
    const Request& request() const noexcept { return request_; }
    const OwnedPaths& paths() const noexcept { return paths_; }
    const persistence::Snapshot& snapshot() const noexcept { return snapshot_; }
    const std::vector<persistence::ReplayCheckpoint>& checkpoints() const noexcept { return checkpoints_; }
    const std::vector<std::string>& events() const noexcept { return events_; }
    std::string_view last_error() const noexcept { return last_error_; }

private:
    std::filesystem::path owned_file(std::string_view category, std::string_view name, std::string_view extension) const;
    simulation::ScenarioConfig scenario_config() const;
    std::vector<persistence::ReplayCommand> scenario_commands() const;
    void require_started(std::string_view operation) const;
    void record_error(std::string message);

    Request request_;
    const data::VirtualFileSystem* vfs_;
    foundation::EnvironmentLookup environment_;
    OwnedPaths paths_;
    SessionState state_ = SessionState::created;
    persistence::Snapshot snapshot_;
    std::vector<persistence::ReplayCheckpoint> checkpoints_;
    std::vector<std::string> events_;
    std::string last_error_;
};

std::string_view mode_name(Mode mode) noexcept;
std::string_view faction_name(Faction faction) noexcept;
std::string_view state_name(SessionState state) noexcept;

} // namespace zh::singleplayer
