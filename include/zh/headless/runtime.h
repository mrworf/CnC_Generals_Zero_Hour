#pragma once

#include "zh/foundation/platform.h"

#include <filesystem>
#include <iosfwd>
#include <optional>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

namespace zh::headless {

enum class ExitCode : int {
    success = 0,
    usage = 2,
    path = 3,
    initialization = 4,
    runtime = 5,
};

class UsageError : public std::runtime_error {
public:
    using std::runtime_error::runtime_error;
};

enum class InitStage {
    paths,
    logging,
    platform,
    renderer,
    audio,
    video,
    engine,
};

enum class LanRole { host, joiner };

std::string_view stage_name(InitStage stage) noexcept;

struct Options {
    std::uint32_t ticks = 1;
    std::optional<std::filesystem::path> state_directory;
    std::optional<InitStage> fail_initialization;
    std::optional<LanRole> lan_role;
    std::optional<std::string> lan_bind_address;
    std::string lan_discovery_address = "255.255.255.255";
    std::optional<std::string> lan_direct_connect;
    std::uint32_t lan_data_identity = 0x5a484441U;
    std::uint32_t lan_map_identity = 0x5a484d41U;
    std::uint32_t lan_timeout_milliseconds = 3000;
};

struct BuildCapabilities {
    std::string project_version;
    std::string revision;
    std::string architecture;
    std::string compiler;
    std::string sdl_version;
    std::string ffmpeg_version;
    bool bink_video_decoder_available = false;
};

Options parse_arguments(const std::vector<std::string_view>& arguments);

ExitCode run(
    const Options& options,
    const BuildCapabilities& capabilities,
    std::ostream& output,
    std::ostream& errors,
    const foundation::EnvironmentLookup& environment = {});

} // namespace zh::headless
