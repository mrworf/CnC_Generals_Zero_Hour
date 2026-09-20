#include "zh/data/config.h"

#include <algorithm>
#include <cctype>
#include <cstdlib>
#include <fstream>
#include <map>
#include <set>
#include <sstream>

namespace zh::data {
namespace {

foundation::EnvironmentLookup process_environment()
{
    return [](std::string_view name) -> std::optional<std::string> {
        const std::string key(name);
        const char* value = std::getenv(key.c_str());
        return value == nullptr ? std::nullopt : std::optional<std::string>(value);
    };
}

std::string trim(std::string value)
{
    const auto whitespace = [](unsigned char character) { return std::isspace(character) != 0; };
    value.erase(value.begin(), std::find_if_not(value.begin(), value.end(), whitespace));
    value.erase(std::find_if_not(value.rbegin(), value.rend(), whitespace).base(), value.end());
    return value;
}

void set_once(
    std::optional<std::filesystem::path>& field,
    std::string_view option,
    std::string_view value)
{
    if (field) throw DataUsageError(std::string(option) + " may be specified only once");
    if (value.empty()) throw DataUsageError(std::string(option) + " requires a non-empty value");
    field = std::filesystem::path(value);
}

void set_once(std::optional<std::string>& field, std::string_view option, std::string_view value)
{
    if (field) throw DataUsageError(std::string(option) + " may be specified only once");
    if (value.empty()) throw DataUsageError(std::string(option) + " requires a non-empty value");
    field = std::string(value);
}

DataArguments read_configuration(const std::filesystem::path& path)
{
    DataArguments configured;
    std::error_code error;
    if (!std::filesystem::exists(path, error)) return configured;
    if (error) throw DataError("cannot inspect configuration file: " + error.message());

    std::ifstream stream(path);
    if (!stream) throw DataError("cannot read configuration file");
    std::string line;
    std::size_t line_number = 0;
    std::set<std::string> keys;
    while (std::getline(stream, line)) {
        ++line_number;
        line = trim(std::move(line));
        if (line.empty() || line.front() == '#' || line.front() == ';') continue;
        const auto equals = line.find('=');
        if (equals == std::string::npos) {
            throw DataError("configuration line " + std::to_string(line_number) + " must be Key=Value");
        }
        const auto key = trim(line.substr(0, equals));
        const auto value = trim(line.substr(equals + 1));
        if (value.empty()) throw DataError("configuration key '" + key + "' has an empty value");
        if (!keys.insert(key).second) throw DataError("duplicate configuration key '" + key + "'");
        if (key == "ZeroHourDataPath") {
            configured.zero_hour_root = value;
        } else if (key == "GeneralsDataPath") {
            configured.generals_root = value;
        } else if (key == "Language") {
            configured.language = value;
        } else {
            throw DataError("unknown configuration key '" + key + "'");
        }
    }
    if (!stream.eof()) throw DataError("failed while reading configuration file");
    return configured;
}

std::filesystem::path require_root(
    const std::optional<std::filesystem::path>& candidate,
    std::string_view option,
    std::string_view key)
{
    if (!candidate) {
        throw DataError("unresolved retail root: provide " + std::string(option) + " or configuration key " +
            std::string(key));
    }
    if (!candidate->is_absolute()) {
        throw DataError(std::string(option) + "/" + std::string(key) + " must be an absolute path");
    }
    std::error_code error;
    if (!std::filesystem::is_directory(*candidate, error) || error) {
        throw DataError(std::string(option) + "/" + std::string(key) + " is not a readable directory");
    }
    return candidate->lexically_normal();
}

std::vector<std::string> discover_locales(const std::filesystem::path& root)
{
    std::vector<std::string> locales;
    const auto add_locale = [&](std::string name) {
        const auto same = [&](const std::string& existing) {
            return existing.size() == name.size() && std::equal(existing.begin(), existing.end(), name.begin(),
                [](char a, char b) { return std::tolower(static_cast<unsigned char>(a)) ==
                    std::tolower(static_cast<unsigned char>(b)); });
        };
        if (std::find_if(locales.begin(), locales.end(), same) == locales.end()) locales.push_back(std::move(name));
    };
    const auto data = root / "Data";
    std::error_code error;
    std::filesystem::directory_iterator iterator(data, error);
    if (!error) {
        for (const auto& entry : iterator) {
            std::error_code type_error;
            if (!entry.is_directory(type_error) || type_error) continue;
            if (std::filesystem::is_regular_file(entry.path() / "generals.csf", type_error) ||
                std::filesystem::is_regular_file(entry.path() / "Language.ini", type_error)) {
                add_locale(entry.path().filename().string());
            }
        }
    }

    std::map<std::string, std::string> archive_names;
    error.clear();
    for (std::filesystem::directory_iterator archives(root, error), end; !error && archives != end; archives.increment(error)) {
        std::error_code type_error;
        if (!archives->is_regular_file(type_error) || type_error) continue;
        const auto original = archives->path().filename().string();
        std::string folded = original;
        std::transform(folded.begin(), folded.end(), folded.begin(), [](unsigned char value) {
            return static_cast<char>(std::tolower(value));
        });
        archive_names.emplace(folded, original);
    }
    for (const auto& [folded, original] : archive_names) {
        constexpr std::string_view prefix = "audio";
        constexpr std::string_view suffix = "zh.big";
        if (folded.size() <= prefix.size() + suffix.size() || folded.compare(0, prefix.size(), prefix) != 0 ||
            folded.compare(folded.size() - suffix.size(), suffix.size(), suffix) != 0) continue;
        const auto language_folded = folded.substr(prefix.size(), folded.size() - prefix.size() - suffix.size());
        const auto language = original.substr(prefix.size(), original.size() - prefix.size() - suffix.size());
        if (archive_names.count(language_folded + std::string(suffix)) != 0 &&
            archive_names.count("speech" + language_folded + std::string(suffix)) != 0) {
            add_locale(language);
        }
    }
    const auto folded_less = [](const std::string& left, const std::string& right) {
        auto fold = [](unsigned char value) { return static_cast<unsigned char>(std::tolower(value)); };
        return std::lexicographical_compare(left.begin(), left.end(), right.begin(), right.end(),
            [&](char a, char b) { return fold(a) < fold(b); });
    };
    std::sort(locales.begin(), locales.end(), folded_less);
    return locales;
}

std::string select_language(
    const std::optional<std::string>& requested,
    const std::filesystem::path& zero_hour_root)
{
    const auto locales = discover_locales(zero_hour_root);
    if (requested) {
        const auto same = [&](const std::string& available) {
            return available.size() == requested->size() &&
                std::equal(available.begin(), available.end(), requested->begin(), [](char a, char b) {
                    return std::tolower(static_cast<unsigned char>(a)) ==
                        std::tolower(static_cast<unsigned char>(b));
                });
        };
        const auto found = std::find_if(locales.begin(), locales.end(), same);
        if (found == locales.end()) {
            throw DataError("selected language '" + *requested + "' has no viable Data/<language> directory");
        }
        return *found;
    }
    if (locales.size() == 1) return locales.front();
    std::ostringstream message;
    if (locales.empty()) {
        message << "no viable locale found; provide --language or configuration key Language after installing locale data";
    } else {
        message << "multiple viable locales found; provide --language or configuration key Language (available:";
        for (const auto& locale : locales) message << ' ' << locale;
        message << ')';
    }
    throw DataError(message.str());
}

} // namespace

DataArguments parse_data_arguments(const std::vector<std::string_view>& arguments)
{
    DataArguments parsed;
    for (std::size_t index = 0; index < arguments.size(); ++index) {
        const auto option = arguments[index];
        if (option != "--zh-data" && option != "--generals-data" && option != "--language" && option != "--mod") {
            throw DataUsageError("unknown verification option '" + std::string(option) + "'");
        }
        if (++index == arguments.size()) throw DataUsageError(std::string(option) + " requires a value");
        if (option == "--mod") {
            if (arguments[index].empty()) throw DataUsageError("--mod requires a non-empty value");
            parsed.mods.emplace_back(arguments[index]);
        } else if (option == "--zh-data") {
            set_once(parsed.zero_hour_root, option, arguments[index]);
        } else if (option == "--generals-data") {
            set_once(parsed.generals_root, option, arguments[index]);
        } else {
            set_once(parsed.language, option, arguments[index]);
        }
    }
    return parsed;
}

DataSelection resolve_data_selection(
    const DataArguments& arguments,
    const foundation::EnvironmentLookup& environment)
{
    const auto lookup = environment ? environment : process_environment();
    const auto xdg = foundation::resolve_xdg_paths(lookup);
    const auto configured = read_configuration(xdg.config / "options.ini");
    const auto zero_hour = require_root(arguments.zero_hour_root ? arguments.zero_hour_root : configured.zero_hour_root,
        "--zh-data", "ZeroHourDataPath");
    const auto generals = require_root(arguments.generals_root ? arguments.generals_root : configured.generals_root,
        "--generals-data", "GeneralsDataPath");
    const auto language = select_language(arguments.language ? arguments.language : configured.language, zero_hour);
    return {zero_hour, generals, language, arguments.mods};
}

void print_data_selection(const DataSelection& selection, std::ostream& output)
{
    output << "verification: Zero Hour root=" << selection.zero_hour_root.string() << '\n';
    output << "verification: Generals root=" << selection.generals_root.string() << '\n';
    output << "verification: locale=" << selection.language << '\n';
    output << "verification: data selection ok; devices skipped\n";
}

} // namespace zh::data
