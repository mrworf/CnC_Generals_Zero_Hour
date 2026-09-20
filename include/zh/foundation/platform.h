#pragma once

#include "zh/foundation/types.h"

#include <atomic>
#include <chrono>
#include <filesystem>
#include <functional>
#include <optional>
#include <stdexcept>
#include <string>
#include <string_view>
#include <thread>
#include <utility>
#include <vector>

namespace zh::foundation {

class PlatformError : public std::runtime_error {
public:
    using std::runtime_error::runtime_error;
};

using EnvironmentLookup = std::function<std::optional<std::string>(std::string_view)>;

struct XdgPaths {
    std::filesystem::path config;
    std::filesystem::path data;
    std::filesystem::path state;
    std::filesystem::path cache;
};

XdgPaths resolve_xdg_paths(const EnvironmentLookup& lookup);
std::string normalize_logical_path(std::string_view input);

std::vector<UInt8> read_binary_file(const std::filesystem::path& path, std::size_t maximum_size);
void write_binary_file_atomic(const std::filesystem::path& path, ByteView bytes);

using SteadyClock = std::chrono::steady_clock;
UInt32 wrapping_milliseconds(SteadyClock::time_point point) noexcept;
bool wrapping_deadline_reached(UInt32 now, UInt32 deadline) noexcept;

class JoiningThread {
public:
    JoiningThread() noexcept = default;

    template <typename Function, typename... Arguments>
    explicit JoiningThread(Function&& function, Arguments&&... arguments)
        : thread_(std::forward<Function>(function), std::forward<Arguments>(arguments)...)
    {
    }

    ~JoiningThread();
    JoiningThread(JoiningThread&& other) noexcept = default;
    JoiningThread& operator=(JoiningThread&& other) noexcept;
    JoiningThread(const JoiningThread&) = delete;
    JoiningThread& operator=(const JoiningThread&) = delete;

    bool joinable() const noexcept { return thread_.joinable(); }
    void join();

private:
    std::thread thread_;
};

using AtomicInt32 = std::atomic<Int32>;
using AtomicUInt32 = std::atomic<UInt32>;

struct Ipv4Endpoint {
    UInt32 address_host_order;
    UInt16 port_host_order;
};

Ipv4Endpoint make_ipv4_endpoint(UInt32 address_host_order, UInt32 port_host_order);

} // namespace zh::foundation
