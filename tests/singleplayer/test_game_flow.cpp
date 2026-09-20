#include "zh/singleplayer/game_flow.h"

#include <algorithm>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <map>
#include <optional>
#include <stdexcept>
#include <string>
#include <unistd.h>

using namespace zh;

namespace {
int failures = 0;
void check(bool condition, const char* message) { if (!condition) { std::cerr << "FAIL: " << message << '\n'; ++failures; } }
void write(const std::filesystem::path& path, std::string_view bytes)
{
    std::filesystem::create_directories(path.parent_path());
    std::ofstream(path, std::ios::binary) << bytes;
}
foundation::EnvironmentLookup environment(const std::filesystem::path& root)
{
    return [root](std::string_view name) -> std::optional<std::string> {
        if (name == "XDG_CONFIG_HOME") return (root / "cfg").string();
        if (name == "XDG_DATA_HOME") return (root / "data").string();
        if (name == "XDG_STATE_HOME") return (root / "state").string();
        if (name == "XDG_CACHE_HOME") return (root / "cache").string();
        return std::nullopt;
    };
}
bool contains(const std::vector<std::string>& values, std::string_view value)
{
    return std::find(values.begin(), values.end(), value) != values.end();
}

singleplayer::FlowReport run_flow(const data::VirtualFileSystem& vfs, const std::filesystem::path& state,
    singleplayer::Mode mode, singleplayer::Faction faction, std::string scenario,
    std::string map = {}, singleplayer::Outcome outcome = singleplayer::Outcome::victory)
{
    renderer::RecordingGpuDevice device(64);
    singleplayer::Request request{mode, faction, std::move(scenario), std::move(map), 42, 80, 20};
    singleplayer::FlowReport report;
    {
        singleplayer::GameFlow flow(device, vfs, request, environment(state));
        report = flow.run(outcome);
    }
    const auto commands = device.snapshot();
    check(commands.find("english:Main Menu") != std::string::npos, "main menu UI missing");
    check(commands.find("world-load map=") != std::string::npos, "world scene missing");
    check(commands.find("effect kind=") != std::string::npos, "effects missing");
    check(commands.find("single-player options language=English") != std::string::npos, "options marker missing");
    check(device.resource_counts().total() == 0, "flow leaked GPU resources after unload");
    return report;
}
}

int main()
{
    const auto root = std::filesystem::temp_directory_path() / ("zh-game-flow-" + std::to_string(::getpid()));
    std::error_code ignored; std::filesystem::remove_all(root, ignored);
    const auto zh_root = root / "zh"; const auto generals_root = root / "generals";
    write(zh_root / "Maps/User/Acceptance/map.ini", "synthetic-map");
    write(zh_root / "Data/English/Movies/Briefing.bik", "resolver-only-synthetic-movie");
    std::filesystem::create_directories(generals_root);
    try {
        const auto vfs = data::VirtualFileSystem::mount({zh_root, generals_root, "English", {}});
        const auto campaign = run_flow(vfs, root / "campaign", singleplayer::Mode::campaign,
            singleplayer::Faction::usa, "usa-campaign-01");
        check(campaign.final_tick == 80 && contains(campaign.events, "victory"), "campaign did not complete");
        check(contains(campaign.events, "save-load-replay"), "persistence flow missing");
        check(contains(campaign.events, "return-to-menu") && contains(campaign.events, "clean-exit"), "exit flow missing");
        check(contains(campaign.events, "movie:Data/English/Movies/Briefing.bik"), "localized movie selection missing");
        check(campaign.warnings.size() == 3, "missing optional audio warnings not bounded");

        for (const auto faction : {singleplayer::Faction::usa, singleplayer::Faction::china, singleplayer::Faction::gla}) {
            const auto report = run_flow(vfs, root / std::string(singleplayer::faction_name(faction)),
                singleplayer::Mode::skirmish, faction, "skirmish-" + std::string(singleplayer::faction_name(faction)));
            check(report.faction == faction && report.final_crc != 0, "faction skirmish did not complete");
        }
        const auto user = run_flow(vfs, root / "user", singleplayer::Mode::user_map,
            singleplayer::Faction::china, "user-acceptance", "Maps/User/Acceptance/map.ini",
            singleplayer::Outcome::defeat);
        check(contains(user.events, "defeat"), "user-map defeat flow missing");

        renderer::RecordingGpuDevice device;
        singleplayer::GameFlow missing(device, vfs,
            {singleplayer::Mode::user_map, singleplayer::Faction::usa, "missing-map",
                "Maps/User/Missing/map.ini", 1, 20, 5}, environment(root / "missing"));
        try { (void)missing.run(singleplayer::Outcome::victory); check(false, "missing required map accepted"); }
        catch (const std::exception& error) {
            check(std::string_view(error.what()).find("required user map") != std::string_view::npos,
                "missing map diagnostic is not actionable");
        }

        std::filesystem::remove_all(root, ignored);
        std::cout << "single-player game-flow tests: " << (failures == 0 ? "ok" : "failed") << '\n';
        return failures == 0 ? 0 : 1;
    } catch (const std::exception& error) {
        std::cerr << error.what() << '\n';
        std::filesystem::remove_all(root, ignored);
        return 1;
    }
}
