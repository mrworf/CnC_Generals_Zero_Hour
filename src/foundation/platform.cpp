#include "zh/foundation/platform.h"

#include <fstream>
#include <limits>
#include <system_error>

namespace zh::foundation {
namespace {

std::filesystem::path absolute_environment_path(
    const EnvironmentLookup& lookup,
    std::string_view variable,
    const std::filesystem::path& fallback)
{
    const auto value = lookup(variable);
    if (value && !value->empty()) {
        const std::filesystem::path path(*value);
        if (!path.is_absolute()) {
            throw PlatformError(std::string(variable) + " must be an absolute path");
        }
        return path;
    }
    return fallback;
}

std::filesystem::path require_home(const EnvironmentLookup& lookup)
{
    const auto home = lookup("HOME");
    if (!home || home->empty()) {
        throw PlatformError("HOME is required when an XDG path is unset");
    }
    const std::filesystem::path path(*home);
    if (!path.is_absolute()) {
        throw PlatformError("HOME must be an absolute path");
    }
    return path;
}

char ascii_fold(char value) noexcept
{
    if (value >= 'A' && value <= 'Z') return static_cast<char>(value + ('a' - 'A'));
    return value;
}

} // namespace

XdgPaths resolve_xdg_paths(const EnvironmentLookup& lookup)
{
    const auto is_unset = [&](std::string_view name) {
        const auto value = lookup(name);
        return !value || value->empty();
    };
    const bool needs_home = is_unset("XDG_CONFIG_HOME") || is_unset("XDG_DATA_HOME") ||
        is_unset("XDG_STATE_HOME") || is_unset("XDG_CACHE_HOME");
    const auto home = needs_home ? require_home(lookup) : std::filesystem::path("/");
    constexpr const char* app = "generals-zero-hour";
    return {
        absolute_environment_path(lookup, "XDG_CONFIG_HOME", home / ".config") / app,
        absolute_environment_path(lookup, "XDG_DATA_HOME", home / ".local/share") / app,
        absolute_environment_path(lookup, "XDG_STATE_HOME", home / ".local/state") / app,
        absolute_environment_path(lookup, "XDG_CACHE_HOME", home / ".cache") / app,
    };
}

std::string normalize_logical_path(std::string_view input)
{
    if (input.empty() || input.front() == '/' || input.front() == '\\' ||
        (input.size() >= 2 && input[1] == ':')) {
        throw PlatformError("logical path must be non-empty and relative");
    }
    std::string output;
    std::string segment;
    const auto flush = [&] {
        if (segment.empty()) return;
        if (segment == "." || segment == "..") {
            throw PlatformError("logical path contains traversal segment");
        }
        if (!output.empty()) output.push_back('/');
        output += segment;
        segment.clear();
    };
    for (char value : input) {
        if (value == '/' || value == '\\') {
            flush();
        } else if (value == '\0') {
            throw PlatformError("logical path contains NUL");
        } else {
            segment.push_back(ascii_fold(value));
        }
    }
    flush();
    if (output.empty()) throw PlatformError("logical path has no components");
    return output;
}

std::vector<UInt8> read_binary_file(const std::filesystem::path& path, std::size_t maximum_size)
{
    std::error_code error;
    const auto size = std::filesystem::file_size(path, error);
    if (error) throw PlatformError("cannot determine file size for '" + path.string() + "': " + error.message());
    if (size > maximum_size || size > std::numeric_limits<std::size_t>::max()) {
        throw PlatformError("file exceeds configured size limit: " + path.string());
    }
    std::vector<UInt8> bytes(static_cast<std::size_t>(size));
    std::ifstream stream(path, std::ios::binary);
    if (!stream) throw PlatformError("cannot open file for reading: " + path.string());
    if (!bytes.empty()) {
        stream.read(reinterpret_cast<char*>(bytes.data()), static_cast<std::streamsize>(bytes.size()));
        if (!stream || stream.gcount() != static_cast<std::streamsize>(bytes.size())) {
            throw PlatformError("short read from file: " + path.string());
        }
    }
    return bytes;
}

void write_binary_file_atomic(const std::filesystem::path& path, ByteView bytes)
{
    if (path.empty() || !path.has_filename()) throw PlatformError("atomic write requires a destination file");
    if (bytes.size != 0 && bytes.data == nullptr) throw PlatformError("atomic write received null bytes");
    const auto temporary = path.parent_path() / (path.filename().string() + ".tmp");
    try {
        std::ofstream stream(temporary, std::ios::binary | std::ios::trunc);
        if (!stream) throw PlatformError("cannot open temporary file for writing: " + temporary.string());
        if (bytes.size != 0) {
            stream.write(reinterpret_cast<const char*>(bytes.data), static_cast<std::streamsize>(bytes.size));
        }
        stream.close();
        if (!stream) throw PlatformError("failed writing temporary file: " + temporary.string());
        std::error_code error;
        std::filesystem::rename(temporary, path, error);
        if (error) throw PlatformError("cannot replace destination file '" + path.string() + "': " + error.message());
    } catch (...) {
        std::error_code ignored;
        std::filesystem::remove(temporary, ignored);
        throw;
    }
}

UInt32 wrapping_milliseconds(SteadyClock::time_point point) noexcept
{
    const auto count = std::chrono::duration_cast<std::chrono::milliseconds>(point.time_since_epoch()).count();
    return static_cast<UInt32>(static_cast<UInt64>(count));
}

bool wrapping_deadline_reached(UInt32 now, UInt32 deadline) noexcept
{
    return static_cast<Int32>(now - deadline) >= 0;
}

JoiningThread::~JoiningThread()
{
    if (thread_.joinable()) thread_.join();
}

JoiningThread& JoiningThread::operator=(JoiningThread&& other) noexcept
{
    if (this != &other) {
        if (thread_.joinable()) thread_.join();
        thread_ = std::move(other.thread_);
    }
    return *this;
}

void JoiningThread::join()
{
    if (!thread_.joinable()) throw PlatformError("thread is not joinable");
    thread_.join();
}

Ipv4Endpoint make_ipv4_endpoint(UInt32 address_host_order, UInt32 port_host_order)
{
    if (port_host_order > std::numeric_limits<UInt16>::max()) {
        throw PlatformError("IPv4 port is outside the 16-bit range");
    }
    return {address_host_order, static_cast<UInt16>(port_host_order)};
}

} // namespace zh::foundation
