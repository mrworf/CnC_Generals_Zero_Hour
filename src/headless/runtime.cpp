#include "zh/headless/runtime.h"

#include "zh/foundation/types.h"
#include "zh/headless/device.h"

#include <charconv>
#include <cstdlib>
#include <fstream>
#include <limits>
#include <memory>
#include <sstream>
#include <system_error>

namespace zh::headless {
namespace {

constexpr std::uint32_t maximum_ticks = 1'000'000;

foundation::EnvironmentLookup process_environment()
{
    return [](std::string_view name) -> std::optional<std::string> {
        const std::string key(name);
        const char* value = std::getenv(key.c_str());
        if (value == nullptr) return std::nullopt;
        return std::string(value);
    };
}

std::uint32_t parse_ticks(std::string_view text)
{
    std::uint32_t value = 0;
    const auto result = std::from_chars(text.data(), text.data() + text.size(), value);
    if (text.empty() || result.ec != std::errc{} || result.ptr != text.data() + text.size() || value > maximum_ticks) {
        throw UsageError("--ticks must be an integer from 0 through 1000000");
    }
    return value;
}

class EventLog {
public:
    EventLog(std::ostream& output, const std::filesystem::path& path) : output_(output), file_(path)
    {
        if (!file_) throw foundation::PlatformError("cannot open headless log: " + path.string());
    }

    void write(std::string_view message)
    {
        output_ << message << '\n';
        file_ << message << '\n';
        file_.flush();
        if (!file_) throw foundation::PlatformError("failed writing headless log");
    }

private:
    std::ostream& output_;
    std::ofstream file_;
};

void output_capabilities(
    EventLog& log,
    const BuildCapabilities& capabilities,
    const std::filesystem::path& state_directory,
    const std::vector<std::unique_ptr<NullDevice>>& devices)
{
    log.write("capability: Zero Hour " + capabilities.project_version + " revision " + capabilities.revision);
    log.write("capability: architecture=" + capabilities.architecture + " compiler=" + capabilities.compiler);
    log.write("capability: SDL=" + capabilities.sdl_version + " (not initialized)");
    log.write("capability: FFmpeg=" + capabilities.ffmpeg_version + " Bink-video-decoder=" +
        std::string(capabilities.bink_video_decoder_available ? "available" : "unavailable"));
    log.write("capability: state-root=" + state_directory.string());
    log.write("capability: zh-data-root=not-configured generals-data-root=not-configured locale=not-selected");
    for (const auto& device : devices) log.write("capability: " + std::string(device->capability()));
    log.write("capability: network skipped (headless asset-free mode)");
}

} // namespace

Options parse_arguments(const std::vector<std::string_view>& arguments)
{
    Options options;
    bool saw_ticks = false;
    bool saw_state = false;
    for (std::size_t index = 0; index < arguments.size(); ++index) {
        const auto argument = arguments[index];
        if (argument == "--ticks") {
            if (saw_ticks) throw UsageError("--ticks may be specified only once");
            if (++index == arguments.size()) throw UsageError("--ticks requires a value");
            options.ticks = parse_ticks(arguments[index]);
            saw_ticks = true;
        } else if (argument == "--state-dir") {
            if (saw_state) throw UsageError("--state-dir may be specified only once");
            if (++index == arguments.size()) throw UsageError("--state-dir requires a value");
            const std::filesystem::path path(arguments[index]);
            if (path.empty() || !path.is_absolute()) throw UsageError("--state-dir must be an absolute path");
            options.state_directory = path.lexically_normal();
            saw_state = true;
        } else {
            throw UsageError("unknown headless option: " + std::string(argument));
        }
    }
    return options;
}

ExitCode run(
    const Options& options,
    const BuildCapabilities& capabilities,
    std::ostream& output,
    std::ostream& errors,
    const foundation::EnvironmentLookup& environment)
{
    std::filesystem::path state_directory;
    try {
        output << "init: paths\n";
        const auto lookup = environment ? environment : process_environment();
        state_directory = options.state_directory.value_or(foundation::resolve_xdg_paths(lookup).state);
        std::error_code directory_error;
        std::filesystem::create_directories(state_directory / "logs", directory_error);
        if (directory_error) {
            throw foundation::PlatformError("cannot create headless state directory '" + state_directory.string() +
                "': " + directory_error.message());
        }

        output << "init: logging\n";
        EventLog log(output, state_directory / "logs/headless.log");
        std::vector<std::unique_ptr<NullDevice>> devices;
        devices.push_back(make_null_platform());
        devices.push_back(make_null_renderer());
        devices.push_back(make_null_audio());
        devices.push_back(make_null_video());

        std::size_t initialized = 0;
        try {
            for (auto& device : devices) {
                log.write("init: " + std::string(device->name()));
                device->initialize();
                ++initialized;
            }
            log.write("init: engine");
            output_capabilities(log, capabilities, state_directory, devices);

            std::uint64_t tick_digest = 0xcbf29ce484222325ULL;
            for (std::uint32_t tick = 0; tick < options.ticks; ++tick) {
                tick_digest ^= static_cast<std::uint64_t>(tick) + 1U;
                tick_digest *= 0x100000001b3ULL;
            }
            log.write("headless: completed ticks=" + std::to_string(options.ticks) +
                " digest=" + std::to_string(tick_digest));

            const std::string completion = "ticks=" + std::to_string(options.ticks) + "\ndigest=" +
                std::to_string(tick_digest) + "\n";
            foundation::write_binary_file_atomic(
                state_directory / "last-headless-run.txt",
                {reinterpret_cast<const foundation::UInt8*>(completion.data()), completion.size()});

            log.write("shutdown: engine");
            while (initialized != 0) {
                --initialized;
                log.write("shutdown: " + std::string(devices[initialized]->name()));
                devices[initialized]->shutdown();
            }
            log.write("shutdown: logging");
        } catch (...) {
            while (initialized != 0) {
                --initialized;
                devices[initialized]->shutdown();
            }
            throw;
        }
        output << "shutdown: paths\n";
        return ExitCode::success;
    } catch (const foundation::PlatformError& error) {
        errors << "headless path/log error: " << error.what() << '\n';
        return ExitCode::path;
    } catch (const std::exception& error) {
        errors << "headless runtime error: " << error.what() << '\n';
        return ExitCode::runtime;
    }
}

} // namespace zh::headless
