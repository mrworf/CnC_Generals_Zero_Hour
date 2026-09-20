#include "zh/foundation/platform.h"

#include <chrono>
#include <filesystem>
#include <iostream>
#include <map>
#include <stdexcept>
#include <string>

using namespace zh::foundation;

namespace {

void check(bool condition, const char* message)
{
    if (!condition) throw std::runtime_error(message);
}

template <typename Function>
void rejects(Function function, const char* message)
{
    try {
        function();
    } catch (const PlatformError&) {
        return;
    }
    throw std::runtime_error(message);
}

EnvironmentLookup environment(std::map<std::string, std::string> values)
{
    return [values = std::move(values)](std::string_view key) -> std::optional<std::string> {
        const auto found = values.find(std::string(key));
        if (found == values.end()) return std::nullopt;
        return found->second;
    };
}

void test_xdg_and_paths()
{
    const auto fallback = resolve_xdg_paths(environment({{"HOME", "/home/tester"}}));
    check(fallback.config == "/home/tester/.config/generals-zero-hour", "config fallback");
    check(fallback.data == "/home/tester/.local/share/generals-zero-hour", "data fallback");
    check(fallback.state == "/home/tester/.local/state/generals-zero-hour", "state fallback");
    check(fallback.cache == "/home/tester/.cache/generals-zero-hour", "cache fallback");
    const auto explicit_paths = resolve_xdg_paths(environment({
        {"HOME", "/home/tester"}, {"XDG_CONFIG_HOME", "/cfg"}, {"XDG_DATA_HOME", "/data"},
        {"XDG_STATE_HOME", "/state"}, {"XDG_CACHE_HOME", "/cache"}}));
    check(explicit_paths.config == "/cfg/generals-zero-hour", "explicit config");
    check(explicit_paths.cache == "/cache/generals-zero-hour", "explicit cache");
    const auto explicit_without_home = resolve_xdg_paths(environment({
        {"XDG_CONFIG_HOME", "/cfg"}, {"XDG_DATA_HOME", "/data"},
        {"XDG_STATE_HOME", "/state"}, {"XDG_CACHE_HOME", "/cache"}}));
    check(explicit_without_home.state == "/state/generals-zero-hour", "explicit XDG paths incorrectly require HOME");
    rejects([] { resolve_xdg_paths(environment({})); }, "missing HOME accepted");
    rejects([] { resolve_xdg_paths(environment({{"HOME", "/h"}, {"XDG_DATA_HOME", "relative"}})); }, "relative XDG accepted");

    check(normalize_logical_path("Data\\INI//Object/.Hidden") == "data/ini/object/.hidden", "logical normalization");
    rejects([] { normalize_logical_path(""); }, "empty logical path accepted");
    rejects([] { normalize_logical_path("/absolute"); }, "absolute logical path accepted");
    rejects([] { normalize_logical_path("C:\\absolute"); }, "drive logical path accepted");
    rejects([] { normalize_logical_path("data/../secret"); }, "parent traversal accepted");
}

void test_files()
{
    const auto root = std::filesystem::temp_directory_path() /
        ("zh-foundation-" + std::to_string(SteadyClock::now().time_since_epoch().count()));
    std::filesystem::create_directory(root);
    try {
        const UInt8 content[] = {0, 1, 2, 0xff};
        const auto file = root / "state.bin";
        write_binary_file_atomic(file, {content, sizeof(content)});
        check(read_binary_file(file, sizeof(content)) == std::vector<UInt8>(content, content + sizeof(content)), "file round trip");
        rejects([&] { read_binary_file(file, sizeof(content) - 1); }, "file size limit ignored");
        rejects([&] { read_binary_file(root / "missing", 8); }, "missing file accepted");
        rejects([&] { write_binary_file_atomic(root / "missing/child", {content, sizeof(content)}); }, "missing parent accepted");
    } catch (...) {
        std::filesystem::remove_all(root);
        throw;
    }
    std::filesystem::remove_all(root);
}

void test_clock_thread_and_socket_types()
{
    check(!wrapping_deadline_reached(0xfffffff0U, 0x00000010U), "pre-wrap deadline reached early");
    check(wrapping_deadline_reached(0x00000010U, 0xfffffff0U), "post-wrap deadline missed");
    const auto point = SteadyClock::time_point(std::chrono::milliseconds(0x100000005ULL));
    check(wrapping_milliseconds(point) == 5U, "wrapping millisecond conversion");

    AtomicInt32 result{0};
    {
        JoiningThread worker([&] { result.store(42); });
        check(worker.joinable(), "new worker is not joinable");
    }
    check(result.load() == 42, "worker was not joined");
    const auto endpoint = make_ipv4_endpoint(0x7f000001U, 8088);
    check(endpoint.address_host_order == 0x7f000001U && endpoint.port_host_order == 8088, "endpoint fields");
    rejects([] { make_ipv4_endpoint(0, 65536); }, "oversized port accepted");
}

} // namespace

int main()
{
    try {
        test_xdg_and_paths();
        test_files();
        test_clock_thread_and_socket_types();
        std::cout << "POSIX support tests: ok\n";
        return 0;
    } catch (const std::exception& error) {
        std::cerr << error.what() << '\n';
        return 1;
    }
}
