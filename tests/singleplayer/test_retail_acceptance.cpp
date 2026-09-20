#include "zh/data/inventory.h"
#include "zh/renderer/recording_device.h"
#include "zh/singleplayer/game_flow.h"

#include <filesystem>
#include <iostream>
#include <optional>
#include <sstream>
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

int main(int argc, char** argv)
{
    if (argc != 4) { std::cerr << "usage: singleplayer_retail_integration ZH_ROOT GENERALS_ROOT LANGUAGE\n"; return 2; }
    const auto state = std::filesystem::temp_directory_path() / ("zh-singleplayer-retail-" + std::to_string(::getpid()));
    std::error_code ignored; std::filesystem::remove_all(state, ignored);
    try {
        const auto vfs = data::VirtualFileSystem::mount({argv[1], argv[2], argv[3], {}});
        const auto manifest = data::load_corpus_manifest(std::filesystem::path(ZH_SOURCE_DIR) / "data/corpus/english-manifest.tsv");
        std::ostringstream corpus_log;
        data::verify_corpus(vfs, manifest, argv[3], corpus_log);
        for (const auto faction : {singleplayer::Faction::usa, singleplayer::Faction::china, singleplayer::Faction::gla}) {
            renderer::RecordingGpuDevice device(64);
            singleplayer::Request request{singleplayer::Mode::skirmish, faction,
                "retail-skirmish-" + std::string(singleplayer::faction_name(faction)), {}, 104, 240, 60};
            singleplayer::GameFlow flow(device, vfs, request,
                environment(state / std::string(singleplayer::faction_name(faction))));
            const auto report = flow.run(singleplayer::Outcome::victory);
            if (report.final_tick != 240 || report.final_crc == 0)
                throw std::runtime_error("retail-backed faction flow did not complete");
        }
        renderer::RecordingGpuDevice campaign_device(64);
        singleplayer::GameFlow campaign(campaign_device, vfs,
            {singleplayer::Mode::campaign, singleplayer::Faction::usa, "retail-campaign-representative",
                {}, 104, 240, 60}, environment(state / "campaign"));
        const auto report = campaign.run(singleplayer::Outcome::victory);
        if (report.final_tick != 240) throw std::runtime_error("retail-backed campaign flow did not complete");
        std::filesystem::remove_all(state, ignored);
        std::cout << "single-player retail integration ok: locale=" << argv[3]
                  << " modes=campaign,skirmish factions=usa,china,gla corpus=text,speech,music,effects,movies\n";
        return 0;
    } catch (const std::exception& error) {
        std::cerr << error.what() << '\n'; std::filesystem::remove_all(state, ignored); return 1;
    }
}
