#include "build_metadata.h"

#include <array>
#include <iostream>
#include <string_view>

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

int main(int argc, char** argv)
{
    if (argc != 2 || std::string_view(argv[1]) != "--bootstrap-smoke") {
        std::cerr << "M0 bootstrap only: pass --bootstrap-smoke; gameplay arrives in later milestones.\n";
        return 2;
    }

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
