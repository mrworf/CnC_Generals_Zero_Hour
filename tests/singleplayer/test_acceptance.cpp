#include "zh/singleplayer/session.h"

#include <filesystem>
#include <functional>
#include <iostream>
#include <optional>
#include <stdexcept>
#include <string>
#include <unistd.h>

using namespace zh;

namespace {
foundation::EnvironmentLookup environment(const std::filesystem::path& root)
{
    return [root](std::string_view name) -> std::optional<std::string> {
        if (name == "XDG_CONFIG_HOME") return (root / "config").string();
        if (name == "XDG_DATA_HOME") return (root / "data").string();
        if (name == "XDG_STATE_HOME") return (root / "state").string();
        if (name == "XDG_CACHE_HOME") return (root / "cache").string();
        return std::nullopt;
    };
}
}

int main()
{
    const auto root = std::filesystem::temp_directory_path() / ("zh-singleplayer-acceptance-" +
        std::to_string(::getpid()) + "-" + std::to_string(std::hash<std::string>{}(ZH_BINARY_DIR)));
    std::error_code ignored; std::filesystem::remove_all(root, ignored);
    try {
        singleplayer::Request long_game{singleplayer::Mode::skirmish, singleplayer::Faction::gla,
            "long-game-gla", {}, 2026, simulation::maximum_scenario_ticks, 100000};
        singleplayer::Session session(long_game, nullptr, environment(root / "long"));
        session.start(); session.finish(singleplayer::Outcome::victory);
        if (session.snapshot().tick != simulation::maximum_scenario_ticks || session.checkpoints().size() != 10)
            throw std::runtime_error("long game did not reach all bounded checkpoints");
        const auto expected_crc = persistence::snapshot_crc32(session.snapshot());
        session.shutdown();

        for (unsigned cycle = 0; cycle < 32; ++cycle) {
            singleplayer::Request request{singleplayer::Mode::skirmish,
                static_cast<singleplayer::Faction>(cycle % 3), "repeat-load-unload", {}, 2026, 400, 100};
            singleplayer::Session repeated(request, nullptr, environment(root / ("cycle-" + std::to_string(cycle))));
            repeated.start(); repeated.advance(200); repeated.save("cycle"); repeated.advance(100);
            repeated.load("cycle"); repeated.write_replay("cycle"); repeated.verify_replay("cycle");
            repeated.finish(singleplayer::Outcome::victory); repeated.shutdown();
            if (repeated.state() != singleplayer::SessionState::stopped)
                throw std::runtime_error("repeated session did not shut down");
        }
        singleplayer::Session repeat_long(long_game, nullptr, environment(root / "long-repeat"));
        repeat_long.start(); repeat_long.finish(singleplayer::Outcome::victory);
        if (persistence::snapshot_crc32(repeat_long.snapshot()) != expected_crc)
            throw std::runtime_error("long-game CRC changed between runs");
        repeat_long.shutdown();
        std::filesystem::remove_all(root, ignored);
        std::cout << "single-player stress acceptance: cycles=32 long_ticks=1000000 crc=" << expected_crc << '\n';
        return 0;
    } catch (const std::exception& error) {
        std::cerr << error.what() << '\n'; std::filesystem::remove_all(root, ignored); return 1;
    }
}
