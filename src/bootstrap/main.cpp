#include "build_metadata.h"
#include "zh/data/config.h"
#include "zh/data/vfs.h"
#include "zh/headless/runtime.h"

extern "C" {
#include <libavcodec/avcodec.h>
}

#include <array>
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
    if (argc < 2 || std::string_view(argv[1]) != "--headless") {
        std::cerr << "usage: zh_main --verify-data [--zh-data PATH] [--generals-data PATH] [--language NAME] [--mod BIG_OR_DIR]\n"
                     "       zh_main --headless [--ticks N] [--state-dir /absolute/path] [--fail-init stage]\n";
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
