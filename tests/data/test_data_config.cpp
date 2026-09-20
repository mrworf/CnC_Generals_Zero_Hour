#include "zh/data/config.h"

#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>

namespace {
int failures = 0;
void check(bool condition, const char* message)
{
    if (!condition) { std::cerr << "FAIL: " << message << '\n'; ++failures; }
}

template <typename Error, typename Function>
void expect_error(Function&& function, std::string_view expected)
{
    try { function(); check(false, "expected error"); }
    catch (const Error& error) {
        check(std::string_view(error.what()).find(expected) != std::string_view::npos, "error diagnostic");
    }
}

void touch(const std::filesystem::path& path)
{
    std::filesystem::create_directories(path.parent_path());
    std::ofstream(path) << "fixture";
}
}

int main()
{
    using namespace zh::data;
    const auto root = std::filesystem::temp_directory_path() / "zh-data-config-test";
    std::error_code ignored;
    std::filesystem::remove_all(root, ignored);
    const auto zh = root / "retail-zh";
    const auto generals = root / "retail-generals";
    touch(zh / "Data/English/marker");
    std::filesystem::create_directories(generals);
    const auto config = root / "config/generals-zero-hour";
    std::filesystem::create_directories(config);
    const zh::foundation::EnvironmentLookup environment = [&](std::string_view name) -> std::optional<std::string> {
        if (name == "XDG_CONFIG_HOME") return (root / "config").string();
        if (name == "XDG_DATA_HOME") return (root / "data").string();
        if (name == "XDG_STATE_HOME") return (root / "state").string();
        if (name == "XDG_CACHE_HOME") return (root / "cache").string();
        return std::nullopt;
    };

    auto explicit_args = parse_data_arguments({"--zh-data", zh.string(), "--generals-data", generals.string(),
        "--language", "english"});
    auto explicit_selection = resolve_data_selection(explicit_args, environment);
    check(explicit_selection.zero_hour_root == zh, "explicit Zero Hour root");
    check(explicit_selection.generals_root == generals, "explicit Generals root");
    check(explicit_selection.language == "English", "language canonicalized");

    auto automatic = explicit_args;
    automatic.language.reset();
    check(resolve_data_selection(automatic, environment).language == "English", "single locale auto-selected");

    { std::ofstream stream(config / "options.ini");
      stream << "ZeroHourDataPath=" << generals.string() << "\nGeneralsDataPath=" << generals.string()
             << "\nLanguage=Configured\n"; }
    touch(generals / "Data/Configured/marker");
    const auto configured = resolve_data_selection({}, environment);
    check(configured.zero_hour_root == generals && configured.generals_root == generals, "identical configured roots accepted");
    const auto overridden = resolve_data_selection(explicit_args, environment);
    check(overridden.zero_hour_root == zh && overridden.language == "English", "CLI overrides configuration");

    expect_error<DataUsageError>([] { parse_data_arguments({"--zh-data"}); }, "requires a value");
    expect_error<DataUsageError>([&] { parse_data_arguments({"--zh-data", zh.string(), "--zh-data", zh.string()}); }, "only once");
    expect_error<DataUsageError>([] { parse_data_arguments({"--scan"}); }, "unknown verification option");
    const auto mod_args = parse_data_arguments({"--mod", "/tmp/one.big", "--mod", "/tmp/two.big"});
    check(mod_args.mods.size() == 2, "multiple explicit mod archives accepted");
    auto relative = explicit_args; relative.zero_hour_root = "relative";
    expect_error<DataError>([&] { resolve_data_selection(relative, environment); }, "must be an absolute path");
    auto missing = explicit_args; missing.generals_root = root / "missing";
    expect_error<DataError>([&] { resolve_data_selection(missing, environment); }, "not a readable directory");
    auto bad_language = explicit_args; bad_language.language = "Missing";
    expect_error<DataError>([&] { resolve_data_selection(bad_language, environment); }, "no viable");

    std::filesystem::remove(config / "options.ini", ignored);
    touch(zh / "Data/French/marker");
    auto ambiguous = explicit_args; ambiguous.language.reset();
    expect_error<DataError>([&] { resolve_data_selection(ambiguous, environment); }, "multiple viable locales");
    std::filesystem::remove_all(zh / "Data", ignored);
    expect_error<DataError>([&] { resolve_data_selection(ambiguous, environment); }, "no viable locale");

    { std::ofstream(config / "options.ini") << "Mystery=thing\n"; }
    expect_error<DataError>([&] { resolve_data_selection(explicit_args, environment); }, "unknown configuration key");

    std::filesystem::remove_all(root, ignored);
    return failures == 0 ? 0 : 1;
}
