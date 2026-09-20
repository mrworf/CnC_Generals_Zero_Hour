#include "zh/singleplayer/session.h"

#include <filesystem>
#include <fstream>
#include <iostream>
#include <map>
#include <stdexcept>
#include <string>
#include <unistd.h>

using namespace zh;

namespace {
int failures = 0;
void check(bool condition, const char* message) { if (!condition) { std::cerr << "FAIL: " << message << '\n'; ++failures; } }

foundation::EnvironmentLookup environment(const std::filesystem::path& root)
{
    const std::map<std::string, std::string> values{
        {"XDG_CONFIG_HOME", (root / "cfg").string()}, {"XDG_DATA_HOME", (root / "data").string()},
        {"XDG_STATE_HOME", (root / "state").string()}, {"XDG_CACHE_HOME", (root / "cache").string()},
    };
    return [values](std::string_view key) -> std::optional<std::string> {
        const auto found = values.find(std::string(key));
        return found == values.end() ? std::nullopt : std::optional<std::string>(found->second);
    };
}
template <typename Function>
void rejects(Function&& function, std::string_view expected)
{
    try { function(); check(false, "invalid single-player operation accepted"); }
    catch (const std::exception& error) {
        check(std::string_view(error.what()).find(expected) != std::string_view::npos, "single-player diagnostic");
    }
}

void write_text(const std::filesystem::path& path, std::string_view text)
{
    std::filesystem::create_directories(path.parent_path());
    std::ofstream(path, std::ios::binary) << text;
}
}

int main()
{
    const auto root = std::filesystem::temp_directory_path() / ("zh-singleplayer-session-" + std::to_string(::getpid()));
    std::error_code ignored;
    std::filesystem::remove_all(root, ignored);
    std::filesystem::create_directories(root);
    try {
        singleplayer::Request campaign{singleplayer::Mode::campaign, singleplayer::Faction::usa,
            "usa-campaign-01", {}, 41, 120, 20};
        singleplayer::Session session(campaign, nullptr, environment(root));
        session.start();
        check(session.state() == singleplayer::SessionState::playing, "campaign starts playing");
        session.advance(60);
        const auto saved_crc = persistence::snapshot_crc32(session.snapshot());
        session.save("mission", true);
        session.advance(20);
        session.load("mission");
        check(persistence::snapshot_crc32(session.snapshot()) == saved_crc, "save restores equivalent snapshot");
        session.write_replay("mission");
        session.verify_replay("mission");
        session.set_paused(true);
        const auto paused_tick = session.snapshot().tick;
        session.advance(10);
        check(session.snapshot().tick == paused_tick, "pause freezes simulation");
        session.set_paused(false);
        session.finish(singleplayer::Outcome::victory);
        check(session.state() == singleplayer::SessionState::victory, "campaign victory");
        check(std::filesystem::exists(session.paths().data / "progression.txt"), "progression persisted");
        session.shutdown(); session.shutdown();
        check(session.state() == singleplayer::SessionState::stopped, "shutdown idempotent");

        singleplayer::Request skirmish{singleplayer::Mode::skirmish, singleplayer::Faction::gla,
            "skirmish-gla", {}, 99, 40, 10};
        singleplayer::Session defeat(skirmish, nullptr, environment(root / "skirmish"));
        defeat.start(); defeat.finish(singleplayer::Outcome::defeat);
        check(defeat.state() == singleplayer::SessionState::defeat, "skirmish defeat");

        const auto zh_root = root / "retail-zh";
        const auto generals_root = root / "retail-generals";
        write_text(zh_root / "Maps/User/Acceptance/map.ini", "synthetic-map");
        std::filesystem::create_directories(generals_root);
        const auto vfs = data::VirtualFileSystem::mount({zh_root, generals_root, "English", {}});
        singleplayer::Request user{singleplayer::Mode::user_map, singleplayer::Faction::china,
            "user-acceptance", "Maps/User/Acceptance/map.ini", 7, 24, 6};
        singleplayer::Session user_session(user, &vfs, environment(root / "user"));
        user_session.start(); user_session.finish(singleplayer::Outcome::victory); user_session.shutdown();

        singleplayer::Session missing({singleplayer::Mode::user_map, singleplayer::Faction::usa,
            "missing", "maps/user/missing/map.ini", 1, 20, 5}, &vfs, environment(root / "missing"));
        rejects([&] { missing.start(); }, "required user map is missing");
        check(missing.state() == singleplayer::SessionState::error, "start failure records error state");

        singleplayer::Session recovery(campaign, nullptr, environment(root / "recovery"));
        recovery.start(); recovery.advance(40); recovery.save("valid");
        const auto before = persistence::snapshot_crc32(recovery.snapshot());
        write_text(recovery.paths().data / "saves/corrupt.zhs", "not-a-save");
        rejects([&] { recovery.load("corrupt"); }, "single-player load failed");
        check(persistence::snapshot_crc32(recovery.snapshot()) == before, "corrupt load preserves live state");
        write_text(recovery.paths().data / "replays/corrupt.zhr", "not-a-replay");
        rejects([&] { recovery.verify_replay("corrupt"); }, "replay verification failed");
        rejects([&] { recovery.save("../escape"); }, "owned file name");
        recovery.shutdown();

        singleplayer::Session bad_ticks({singleplayer::Mode::campaign, singleplayer::Faction::usa,
            "bad", {}, 1, 3, 1}, nullptr, environment(root / "bad"));
        rejects([&] { bad_ticks.start(); }, "final tick");

        std::filesystem::remove_all(root, ignored);
        std::cout << "single-player lifecycle tests: " << (failures == 0 ? "ok" : "failed") << '\n';
        return failures == 0 ? 0 : 1;
    } catch (const std::exception& error) {
        std::cerr << error.what() << '\n';
        std::filesystem::remove_all(root, ignored);
        return 1;
    }
}
