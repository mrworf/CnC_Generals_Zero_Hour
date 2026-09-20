#include "zh/headless/runtime.h"

#include "zh/foundation/types.h"
#include "zh/headless/device.h"
#include "zh/lan/harness.h"

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
constexpr std::uint32_t maximum_lan_timeout = 30'000;

class InitFailure : public std::runtime_error {
public:
    explicit InitFailure(InitStage stage)
        : std::runtime_error("injected failure"), stage_(stage)
    {
    }

    InitStage stage() const noexcept { return stage_; }

private:
    InitStage stage_;
};

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

std::uint32_t parse_u32(std::string_view option, std::string_view text, std::uint32_t maximum)
{
    std::uint32_t value = 0;
    const auto result = std::from_chars(text.data(), text.data() + text.size(), value);
    if (text.empty() || result.ec != std::errc{} || result.ptr != text.data() + text.size() || value > maximum) {
        throw UsageError(std::string(option) + " must be an integer from 0 through " + std::to_string(maximum));
    }
    return value;
}

bool numeric_ipv4(std::string_view text)
{
    if (text.empty()) return false;
    std::size_t begin = 0;
    unsigned fields = 0;
    while (begin <= text.size()) {
        const auto end = text.find('.', begin);
        const auto field = text.substr(begin, end == std::string_view::npos ? text.size() - begin : end - begin);
        unsigned value = 0;
        const auto parsed = std::from_chars(field.data(), field.data() + field.size(), value);
        if (field.empty() || parsed.ec != std::errc{} || parsed.ptr != field.data() + field.size() || value > 255) return false;
        ++fields;
        if (end == std::string_view::npos) break;
        begin = end + 1;
    }
    return fields == 4;
}

InitStage parse_stage(std::string_view text)
{
    constexpr InitStage stages[]{InitStage::paths, InitStage::logging, InitStage::platform,
        InitStage::renderer, InitStage::audio, InitStage::video, InitStage::engine};
    for (const auto stage : stages) {
        if (text == stage_name(stage)) return stage;
    }
    throw UsageError("--fail-init stage must be one of: paths, logging, platform, renderer, audio, video, engine");
}

void inject_if_requested(const Options& options, InitStage stage)
{
    if (options.fail_initialization == stage) throw InitFailure(stage);
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
    const std::vector<std::unique_ptr<NullDevice>>& devices,
    bool lan_enabled)
{
    log.write("capability: Zero Hour " + capabilities.project_version + " revision " + capabilities.revision);
    log.write("capability: architecture=" + capabilities.architecture + " compiler=" + capabilities.compiler);
    log.write("capability: SDL=" + capabilities.sdl_version + " (not initialized)");
    log.write("capability: FFmpeg=" + capabilities.ffmpeg_version + " Bink-video-decoder=" +
        std::string(capabilities.bink_video_decoder_available ? "available" : "unavailable"));
    log.write("capability: state-root=" + state_directory.string());
    log.write("capability: zh-data-root=not-configured generals-data-root=not-configured locale=not-selected");
    for (const auto& device : devices) log.write("capability: " + std::string(device->capability()));
    log.write(lan_enabled ? "capability: network=POSIX IPv4 UDP (headless LAN harness)" :
                            "capability: network skipped (headless asset-free mode)");
}

void write_lines(EventLog& log, std::string_view text)
{
    std::size_t begin = 0;
    while (begin < text.size()) {
        const auto end = text.find('\n', begin);
        log.write(text.substr(begin, end == std::string_view::npos ? text.size() - begin : end - begin));
        if (end == std::string_view::npos) break;
        begin = end + 1;
    }
}

} // namespace

std::string_view stage_name(InitStage stage) noexcept
{
    switch (stage) {
    case InitStage::paths: return "paths";
    case InitStage::logging: return "logging";
    case InitStage::platform: return "platform";
    case InitStage::renderer: return "renderer";
    case InitStage::audio: return "audio";
    case InitStage::video: return "video";
    case InitStage::engine: return "engine";
    }
    return "unknown";
}

Options parse_arguments(const std::vector<std::string_view>& arguments)
{
    Options options;
    bool saw_ticks = false;
    bool saw_state = false;
    bool saw_failure = false;
    bool saw_lan_role = false;
    bool saw_lan_bind = false;
    bool saw_lan_discovery = false;
    bool saw_lan_direct = false;
    bool saw_lan_data = false;
    bool saw_lan_map = false;
    bool saw_lan_timeout = false;
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
        } else if (argument == "--fail-init") {
            if (saw_failure) throw UsageError("--fail-init may be specified only once");
            if (++index == arguments.size()) throw UsageError("--fail-init requires a stage");
            options.fail_initialization = parse_stage(arguments[index]);
            saw_failure = true;
        } else if (argument == "--lan-role") {
            if (saw_lan_role) throw UsageError("--lan-role may be specified only once");
            if (++index == arguments.size()) throw UsageError("--lan-role requires host or joiner");
            if (arguments[index] == "host") options.lan_role = LanRole::host;
            else if (arguments[index] == "joiner") options.lan_role = LanRole::joiner;
            else throw UsageError("--lan-role must be host or joiner");
            saw_lan_role = true;
        } else if (argument == "--lan-bind-address" || argument == "--lan-discovery-address" ||
                   argument == "--lan-direct-connect") {
            bool* seen = argument == "--lan-bind-address" ? &saw_lan_bind :
                (argument == "--lan-discovery-address" ? &saw_lan_discovery : &saw_lan_direct);
            if (*seen) throw UsageError(std::string(argument) + " may be specified only once");
            if (++index == arguments.size()) throw UsageError(std::string(argument) + " requires an IPv4 address");
            if (!numeric_ipv4(arguments[index])) throw UsageError(std::string(argument) + " requires a numeric IPv4 address");
            if (argument == "--lan-bind-address") options.lan_bind_address = std::string(arguments[index]);
            else if (argument == "--lan-discovery-address") options.lan_discovery_address = std::string(arguments[index]);
            else options.lan_direct_connect = std::string(arguments[index]);
            *seen = true;
        } else if (argument == "--lan-data-identity" || argument == "--lan-map-identity" ||
                   argument == "--lan-timeout-ms") {
            bool* seen = argument == "--lan-data-identity" ? &saw_lan_data :
                (argument == "--lan-map-identity" ? &saw_lan_map : &saw_lan_timeout);
            if (*seen) throw UsageError(std::string(argument) + " may be specified only once");
            if (++index == arguments.size()) throw UsageError(std::string(argument) + " requires a value");
            const auto maximum = argument == "--lan-timeout-ms" ? maximum_lan_timeout :
                std::numeric_limits<std::uint32_t>::max();
            const auto value = parse_u32(argument, arguments[index], maximum);
            if (argument == "--lan-data-identity") options.lan_data_identity = value;
            else if (argument == "--lan-map-identity") options.lan_map_identity = value;
            else options.lan_timeout_milliseconds = value;
            *seen = true;
        } else {
            throw UsageError("unknown headless option: " + std::string(argument));
        }
    }
    const bool any_lan_option = saw_lan_role || saw_lan_bind || saw_lan_discovery || saw_lan_direct ||
        saw_lan_data || saw_lan_map || saw_lan_timeout;
    if (any_lan_option && !options.lan_role) throw UsageError("LAN options require --lan-role host or joiner");
    if (options.lan_role && !options.lan_bind_address) throw UsageError("--lan-role requires --lan-bind-address");
    if (options.lan_role == LanRole::joiner && !options.lan_direct_connect) {
        throw UsageError("--lan-role joiner requires --lan-direct-connect");
    }
    if (options.lan_role == LanRole::host && options.lan_direct_connect) {
        throw UsageError("--lan-direct-connect is valid only for --lan-role joiner");
    }
    if (options.lan_role && options.lan_timeout_milliseconds < 100) {
        throw UsageError("--lan-timeout-ms must be at least 100");
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
    bool paths_initialized = false;
    bool engine_initialized = false;
    std::unique_ptr<EventLog> log;
    std::vector<std::unique_ptr<NullDevice>> devices;
    std::size_t initialized_devices = 0;
    const auto trace = [&](std::string message) {
        if (log) log->write(message);
        else output << message << '\n';
    };
    const auto shutdown = [&] {
        if (engine_initialized) {
            trace("shutdown: engine");
            engine_initialized = false;
        }
        while (initialized_devices != 0) {
            --initialized_devices;
            trace("shutdown: " + std::string(devices[initialized_devices]->name()));
            devices[initialized_devices]->shutdown();
        }
        if (log) {
            log->write("shutdown: logging");
            log.reset();
        }
        if (paths_initialized) {
            output << "shutdown: paths\n";
            paths_initialized = false;
        }
    };
    try {
        output << "init: paths\n";
        inject_if_requested(options, InitStage::paths);
        const auto lookup = environment ? environment : process_environment();
        state_directory = options.state_directory.value_or(foundation::resolve_xdg_paths(lookup).state);
        std::error_code directory_error;
        std::filesystem::create_directories(state_directory / "logs", directory_error);
        if (directory_error) {
            throw foundation::PlatformError("cannot create headless state directory '" + state_directory.string() +
                "': " + directory_error.message());
        }
        paths_initialized = true;

        output << "init: logging\n";
        inject_if_requested(options, InitStage::logging);
        log = std::make_unique<EventLog>(output, state_directory / "logs/headless.log");
        devices.push_back(make_null_platform());
        devices.push_back(make_null_renderer());
        devices.push_back(make_null_audio());
        devices.push_back(make_null_video());

        constexpr InitStage device_stages[]{InitStage::platform, InitStage::renderer, InitStage::audio, InitStage::video};
        for (std::size_t index = 0; index < devices.size(); ++index) {
            trace("init: " + std::string(devices[index]->name()));
            inject_if_requested(options, device_stages[index]);
            devices[index]->initialize();
            ++initialized_devices;
        }
        trace("init: engine");
        inject_if_requested(options, InitStage::engine);
        engine_initialized = true;
        output_capabilities(*log, capabilities, state_directory, devices, options.lan_role.has_value());

        if (options.lan_role) {
            lan::HarnessConfig lan_config;
            lan_config.role = *options.lan_role == LanRole::host ? lan::HarnessRole::host : lan::HarnessRole::joiner;
            lan_config.bind_address = lan::EndpointAddress{*options.lan_bind_address, lan::lobby_port};
            lan_config.discovery_address = lan::EndpointAddress{options.lan_discovery_address, lan::lobby_port};
            if (options.lan_direct_connect) lan_config.direct_connect = lan::EndpointAddress{*options.lan_direct_connect, lan::lobby_port};
            lan_config.peer_id = *options.lan_role == LanRole::host ? 1U : 2U;
            lan_config.data_identity = options.lan_data_identity;
            lan_config.map_identity = options.lan_map_identity;
            lan_config.timeout_milliseconds = options.lan_timeout_milliseconds;
            std::ostringstream lan_output;
            std::ostringstream lan_errors;
            const int lan_result = lan::run_headless_peer(lan_config, lan_output, lan_errors);
            write_lines(*log, lan_output.str());
            if (lan_result != 0) {
                write_lines(*log, lan_errors.str());
                throw std::runtime_error(lan_errors.str());
            }
        }

        std::uint64_t tick_digest = 0xcbf29ce484222325ULL;
        for (std::uint32_t tick = 0; tick < options.ticks; ++tick) {
            tick_digest ^= static_cast<std::uint64_t>(tick) + 1U;
            tick_digest *= 0x100000001b3ULL;
        }
        log->write("headless: completed ticks=" + std::to_string(options.ticks) +
            " digest=" + std::to_string(tick_digest));

        const std::string completion = "ticks=" + std::to_string(options.ticks) + "\ndigest=" +
            std::to_string(tick_digest) + "\n";
        foundation::write_binary_file_atomic(
            state_directory / "last-headless-run.txt",
            {reinterpret_cast<const foundation::UInt8*>(completion.data()), completion.size()});

        shutdown();
        return ExitCode::success;
    } catch (const InitFailure& failure) {
        trace("initialization failed at " + std::string(stage_name(failure.stage())) + ": " + failure.what());
        shutdown();
        errors << "initialization failed at " << stage_name(failure.stage()) << ": " << failure.what() << '\n';
        return ExitCode::initialization;
    } catch (const foundation::PlatformError& error) {
        shutdown();
        errors << "headless path/log error: " << error.what() << '\n';
        return ExitCode::path;
    } catch (const std::exception& error) {
        shutdown();
        errors << "headless runtime error: " << error.what() << '\n';
        return ExitCode::runtime;
    }
}

} // namespace zh::headless
