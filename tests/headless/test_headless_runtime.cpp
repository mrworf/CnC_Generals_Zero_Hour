#include "zh/headless/runtime.h"

#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

namespace {

int failures = 0;

void check(bool condition, const char* message)
{
    if (!condition) {
        std::cerr << "FAIL: " << message << '\n';
        ++failures;
    }
}

template <typename Function>
void expect_usage(Function&& function, std::string_view expected)
{
    try {
        function();
        check(false, "expected usage error");
    } catch (const zh::headless::UsageError& error) {
        check(std::string_view(error.what()).find(expected) != std::string_view::npos, "usage error diagnostic");
    }
}

std::string read_text(const std::filesystem::path& path)
{
    std::ifstream stream(path);
    return {std::istreambuf_iterator<char>(stream), std::istreambuf_iterator<char>()};
}

zh::headless::BuildCapabilities capabilities()
{
    return {"test", "revision", "x86_64", "test compiler", "test SDL", "test FFmpeg", true};
}

} // namespace

int main()
{
    using zh::headless::parse_arguments;
    const auto parsed = parse_arguments({"--ticks", "0", "--state-dir", "/tmp/zh-headless-plan-test"});
    check(parsed.ticks == 0, "zero ticks accepted");
    check(parsed.state_directory == "/tmp/zh-headless-plan-test", "absolute state path accepted");
    check(parse_arguments({"--ticks", "1000000"}).ticks == 1'000'000, "maximum ticks accepted");

    expect_usage([] { parse_arguments({"--ticks"}); }, "requires a value");
    expect_usage([] { parse_arguments({"--ticks", "1000001"}); }, "0 through 1000000");
    expect_usage([] { parse_arguments({"--ticks", "-1"}); }, "0 through 1000000");
    expect_usage([] { parse_arguments({"--state-dir", "relative"}); }, "absolute path");
    expect_usage([] { parse_arguments({"--ticks", "1", "--ticks", "2"}); }, "only once");
    expect_usage([] { parse_arguments({"--unknown"}); }, "unknown headless option");

    const auto root = std::filesystem::temp_directory_path() / "zh-headless-runtime-test";
    std::error_code ignored;
    std::filesystem::remove_all(root, ignored);
    zh::headless::Options options;
    options.ticks = 3;
    options.state_directory = root;
    std::ostringstream output;
    std::ostringstream errors;
    check(zh::headless::run(options, capabilities(), output, errors) == zh::headless::ExitCode::success,
        "explicit-state run succeeds");
    check(errors.str().empty(), "successful run has no error output");
    check(output.str().find("headless: completed ticks=3") != std::string::npos, "tick result reported");
    check(output.str().find("SDL=test SDL (not initialized)") != std::string::npos, "SDL skip reported");
    check(output.str().find("GPU skipped") != std::string::npos, "GPU skip reported");
    check(read_text(root / "last-headless-run.txt").find("ticks=3") != std::string::npos,
        "completion record written below state root");
    check(read_text(root / "logs/headless.log").find("shutdown: engine") != std::string::npos,
        "lifecycle log written");

    const auto xdg = root / "xdg-state";
    zh::headless::Options xdg_options;
    xdg_options.ticks = 1;
    const zh::foundation::EnvironmentLookup environment = [xdg](std::string_view name) -> std::optional<std::string> {
        if (name == "XDG_STATE_HOME") return xdg.string();
        if (name == "HOME") return "/not-used";
        if (name == "XDG_CONFIG_HOME") return "/tmp/config";
        if (name == "XDG_DATA_HOME") return "/tmp/data";
        if (name == "XDG_CACHE_HOME") return "/tmp/cache";
        return std::nullopt;
    };
    std::ostringstream xdg_output;
    std::ostringstream xdg_errors;
    check(zh::headless::run(xdg_options, capabilities(), xdg_output, xdg_errors, environment) ==
            zh::headless::ExitCode::success,
        "XDG-state run succeeds");
    check(std::filesystem::exists(xdg / "generals-zero-hour/last-headless-run.txt"), "XDG state root used");

    const auto blocking_file = root / "blocking-file";
    { std::ofstream stream(blocking_file); stream << "not a directory"; }
    zh::headless::Options bad_options;
    bad_options.state_directory = blocking_file / "child";
    std::ostringstream bad_output;
    std::ostringstream bad_errors;
    check(zh::headless::run(bad_options, capabilities(), bad_output, bad_errors) == zh::headless::ExitCode::path,
        "unwritable state path has path exit code");
    check(bad_errors.str().find("cannot create headless state directory") != std::string::npos,
        "path error is actionable");

    std::filesystem::remove_all(root, ignored);
    return failures == 0 ? 0 : 1;
}
