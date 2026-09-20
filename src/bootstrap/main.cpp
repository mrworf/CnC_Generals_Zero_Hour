#include "build_metadata.h"
#include "corpus_manifest.h"
#include "zh/data/config.h"
#include "zh/data/inventory.h"
#include "zh/data/vfs.h"
#include "zh/headless/runtime.h"
#include "zh/renderer/recording_device.h"
#include "zh/singleplayer/game_flow.h"

extern "C" {
#include <libavcodec/avcodec.h>
}

#include <algorithm>
#include <array>
#include <charconv>
#include <iostream>
#include <string>
#include <string_view>
#include <vector>

const char* zh_compression_bootstrap_component() noexcept;
const char* zh_wwsupport_bootstrap_component() noexcept;
const char* zh_w3d_bootstrap_component() noexcept;
const char* zh_wwshade_bootstrap_component() noexcept;
const char* zh_game_engine_bootstrap_component() noexcept;
const char* zh_platform_sdl_bootstrap_component() noexcept;
const char* zh_renderer_sdl_gpu_bootstrap_component() noexcept;
const char* zh_audio_miniaudio_bootstrap_component() noexcept;
const char* zh_video_ffmpeg_bootstrap_component() noexcept;
const char* zh_os_posix_bootstrap_component() noexcept;
const char* zh_game_device_bootstrap_component() noexcept;
const char* zh_null_platform_bootstrap_component() noexcept;
const char* zh_null_renderer_bootstrap_component() noexcept;
const char* zh_null_audio_bootstrap_component() noexcept;
const char* zh_null_video_bootstrap_component() noexcept;
const char* zh_test_support_bootstrap_component() noexcept;

namespace {

struct SinglePlayerArguments {
    zh::singleplayer::Request request;
    zh::singleplayer::Outcome outcome = zh::singleplayer::Outcome::victory;
    std::vector<std::string_view> data_arguments;
};

SinglePlayerArguments parse_single_player_arguments(const std::vector<std::string_view>& arguments)
{
    SinglePlayerArguments parsed;
    bool saw_mode = false, saw_faction = false, saw_scenario = false, saw_map = false;
    bool saw_ticks = false, saw_outcome = false;
    for (std::size_t index = 0; index < arguments.size(); ++index) {
        const auto option = arguments[index];
        if (option == "--zh-data" || option == "--generals-data" || option == "--language" || option == "--mod") {
            if (index + 1 == arguments.size()) throw zh::data::DataUsageError(std::string(option) + " requires a value");
            parsed.data_arguments.push_back(option);
            parsed.data_arguments.push_back(arguments[++index]);
            continue;
        }
        if (index + 1 == arguments.size()) throw zh::data::DataUsageError(std::string(option) + " requires a value");
        const auto value = arguments[++index];
        if (option == "--mode") {
            if (saw_mode) throw zh::data::DataUsageError("--mode may be specified only once");
            if (value == "campaign") parsed.request.mode = zh::singleplayer::Mode::campaign;
            else if (value == "skirmish") parsed.request.mode = zh::singleplayer::Mode::skirmish;
            else if (value == "user-map") parsed.request.mode = zh::singleplayer::Mode::user_map;
            else throw zh::data::DataUsageError("--mode must be campaign, skirmish, or user-map");
            saw_mode = true;
        } else if (option == "--faction") {
            if (saw_faction) throw zh::data::DataUsageError("--faction may be specified only once");
            if (value == "usa") parsed.request.faction = zh::singleplayer::Faction::usa;
            else if (value == "china") parsed.request.faction = zh::singleplayer::Faction::china;
            else if (value == "gla") parsed.request.faction = zh::singleplayer::Faction::gla;
            else throw zh::data::DataUsageError("--faction must be usa, china, or gla");
            saw_faction = true;
        } else if (option == "--scenario") {
            if (saw_scenario || value.empty()) throw zh::data::DataUsageError("--scenario requires one non-empty value");
            parsed.request.scenario = std::string(value); saw_scenario = true;
        } else if (option == "--map") {
            if (saw_map || value.empty()) throw zh::data::DataUsageError("--map requires one non-empty value");
            parsed.request.user_map = std::string(value); saw_map = true;
        } else if (option == "--ticks") {
            if (saw_ticks) throw zh::data::DataUsageError("--ticks may be specified only once");
            std::uint32_t ticks = 0;
            const auto result = std::from_chars(value.data(), value.data() + value.size(), ticks);
            if (value.empty() || result.ec != std::errc{} || result.ptr != value.data() + value.size()
                || ticks < 4 || ticks > zh::simulation::maximum_scenario_ticks)
                throw zh::data::DataUsageError("--ticks must be an integer from 4 through 1000000");
            parsed.request.final_tick = ticks;
            parsed.request.checkpoint_interval = std::max<std::uint32_t>(1, ticks / 4);
            saw_ticks = true;
        } else if (option == "--outcome") {
            if (saw_outcome) throw zh::data::DataUsageError("--outcome may be specified only once");
            if (value == "victory") parsed.outcome = zh::singleplayer::Outcome::victory;
            else if (value == "defeat") parsed.outcome = zh::singleplayer::Outcome::defeat;
            else throw zh::data::DataUsageError("--outcome must be victory or defeat");
            saw_outcome = true;
        } else {
            throw zh::data::DataUsageError("unknown single-player option '" + std::string(option) + "'");
        }
    }
    if (parsed.request.mode == zh::singleplayer::Mode::user_map && !saw_map)
        throw zh::data::DataUsageError("--mode user-map requires --map");
    if (parsed.request.mode != zh::singleplayer::Mode::user_map && saw_map)
        throw zh::data::DataUsageError("--map is valid only with --mode user-map");
    return parsed;
}

int bootstrap_smoke()
{
    const std::array components{
        zh_compression_bootstrap_component(), zh_wwsupport_bootstrap_component(),
        zh_w3d_bootstrap_component(), zh_wwshade_bootstrap_component(),
        zh_game_engine_bootstrap_component(), zh_platform_sdl_bootstrap_component(),
        zh_renderer_sdl_gpu_bootstrap_component(), zh_audio_miniaudio_bootstrap_component(),
        zh_video_ffmpeg_bootstrap_component(), zh_os_posix_bootstrap_component(),
        zh_game_device_bootstrap_component(), zh_null_platform_bootstrap_component(),
        zh_null_renderer_bootstrap_component(), zh_null_audio_bootstrap_component(),
        zh_null_video_bootstrap_component(), zh_test_support_bootstrap_component(),
    };
    for (const char* component : components) {
        if (component == nullptr || *component == '\0') {
            std::cerr << "bootstrap component registration is invalid\n";
            return 3;
        }
    }

    std::cout << "Zero Hour " << ZH_BUILD_PROJECT_VERSION << " revision " << ZH_BUILD_GIT_REVISION
              << " architecture " << ZH_BUILD_ARCHITECTURE << " compiler " << ZH_BUILD_COMPILER
              << " SDL " << ZH_BUILD_SDL_VERSION << " FFmpeg " << ZH_BUILD_FFMPEG_VERSION << '\n';
    std::cout << "asset-free bootstrap graph: ok (" << components.size() << " components)\n";
    return 0;
}

} // namespace

int main(int argc, char** argv)
{
    if (argc == 2 && std::string_view(argv[1]) == "--bootstrap-smoke") return bootstrap_smoke();
    if (argc >= 2 && std::string_view(argv[1]) == "--verify-data") {
        std::vector<std::string_view> arguments;
        for (int index = 2; index < argc; ++index) arguments.emplace_back(argv[index]);
        try {
            const auto selection = zh::data::resolve_data_selection(zh::data::parse_data_arguments(arguments));
            zh::data::print_data_selection(selection, std::cout);
            const auto vfs = zh::data::VirtualFileSystem::mount(selection);
            for (const auto& archive : vfs.archive_mount_order()) std::cout << "verification: mount=" << archive << '\n';
            std::cout << "verification: resources=" << vfs.resources().size() << "\nverification: VFS ok\n";
            const auto manifest = zh::data::parse_corpus_manifest(ZH_CORPUS_MANIFEST);
            zh::data::verify_corpus(vfs, manifest, selection.language, std::cout);
            return 0;
        } catch (const zh::data::DataUsageError& error) {
            std::cerr << "data argument error: " << error.what() << '\n';
            return 2;
        } catch (const zh::data::DataError& error) {
            std::cerr << "data verification error: " << error.what() << '\n';
            return 3;
        } catch (const zh::foundation::PlatformError& error) {
            std::cerr << "data path error: " << error.what() << '\n';
            return 3;
        }
    }
    if (argc >= 2 && std::string_view(argv[1]) == "--single-player-integration-smoke") {
        std::vector<std::string_view> arguments;
        for (int index = 2; index < argc; ++index) arguments.emplace_back(argv[index]);
        try {
            const auto parsed = parse_single_player_arguments(arguments);
            const auto selection = zh::data::resolve_data_selection(zh::data::parse_data_arguments(parsed.data_arguments));
            const auto vfs = zh::data::VirtualFileSystem::mount(selection);
            zh::renderer::RecordingGpuDevice device(64);
            zh::singleplayer::GameFlow flow(device, vfs, parsed.request);
            const auto report = flow.run(parsed.outcome);
            std::cout << "single-player: mode=" << zh::singleplayer::mode_name(report.mode)
                      << " faction=" << zh::singleplayer::faction_name(report.faction)
                      << " scenario=" << report.scenario << " tick=" << report.final_tick
                      << " crc=" << report.final_crc << " warnings=" << report.warnings.size() << '\n';
            std::cout << "single-player: completed "
                      << (report.outcome == zh::singleplayer::Outcome::victory ? "victory" : "defeat")
                      << " and exited cleanly\n";
            return 0;
        } catch (const zh::data::DataUsageError& error) {
            std::cerr << "single-player argument error: " << error.what() << '\n'; return 2;
        } catch (const zh::data::DataError& error) {
            std::cerr << "single-player data error: " << error.what() << '\n'; return 3;
        } catch (const std::exception& error) {
            std::cerr << "single-player runtime error: " << error.what() << '\n'; return 5;
        }
    }
    if (argc < 2 || std::string_view(argv[1]) != "--headless") {
        std::cerr << "usage: zh_main --verify-data [--zh-data PATH] [--generals-data PATH] [--language NAME] [--mod BIG_OR_DIR]\n"
                     "       zh_main --single-player-integration-smoke --mode campaign|skirmish|user-map --scenario NAME\n"
                     "         --faction usa|china|gla [--map LOGICAL_PATH] [--ticks N] [--outcome victory|defeat]\n"
                     "         [--zh-data PATH] [--generals-data PATH] [--language NAME] [--mod BIG_OR_DIR]\n"
                     "       zh_main --headless [--ticks N] [--state-dir /absolute/path] [--fail-init stage]\n"
                     "         [--lan-role host|joiner --lan-bind-address IPv4]\n"
                     "         [--lan-discovery-address IPv4] [--lan-direct-connect IPv4]\n";
        return static_cast<int>(zh::headless::ExitCode::usage);
    }

    std::vector<std::string_view> arguments;
    for (int index = 2; index < argc; ++index) arguments.emplace_back(argv[index]);
    try {
        const auto options = zh::headless::parse_arguments(arguments);
        const zh::headless::BuildCapabilities capabilities{
            ZH_BUILD_PROJECT_VERSION,
            ZH_BUILD_GIT_REVISION,
            ZH_BUILD_ARCHITECTURE,
            ZH_BUILD_COMPILER,
            ZH_BUILD_SDL_VERSION,
            ZH_BUILD_FFMPEG_VERSION,
            avcodec_find_decoder(AV_CODEC_ID_BINKVIDEO) != nullptr,
        };
        return static_cast<int>(zh::headless::run(options, capabilities, std::cout, std::cerr));
    } catch (const zh::headless::UsageError& error) {
        std::cerr << "headless argument error: " << error.what() << '\n';
        return static_cast<int>(zh::headless::ExitCode::usage);
    }
}
