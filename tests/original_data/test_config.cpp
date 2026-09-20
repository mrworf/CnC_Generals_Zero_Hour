#include "zh/original_data.h"
#include "zh/original_process.h"

#include <algorithm>
#include <array>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>
#include <string_view>
#include <vector>
#include <unistd.h>

namespace {
int failures = 0;

void check(bool condition, std::string_view message)
{
    if (!condition) { std::cerr << "FAIL: " << message << '\n'; ++failures; }
}

template <typename Function>
void expect_error(Function&& function, std::string_view text)
{
    try { function(); check(false, "expected original-data error"); }
    catch (const zh::original_data::Error& error) {
        check(std::string_view(error.what()).find(text) != std::string_view::npos, "error diagnostic");
    }
}

void append_le32(std::vector<std::uint8_t>& bytes, std::uint32_t value)
{
    bytes.push_back(static_cast<std::uint8_t>(value));
    bytes.push_back(static_cast<std::uint8_t>(value >> 8U));
    bytes.push_back(static_cast<std::uint8_t>(value >> 16U));
    bytes.push_back(static_cast<std::uint8_t>(value >> 24U));
}

constexpr std::uint32_t tag(char a, char b, char c, char d)
{
    return (static_cast<std::uint32_t>(a) << 24U) | (static_cast<std::uint32_t>(b) << 16U) |
        (static_cast<std::uint32_t>(c) << 8U) | static_cast<std::uint32_t>(d);
}

std::vector<std::uint8_t> csf_fixture()
{
    std::vector<std::uint8_t> bytes;
    append_le32(bytes, tag('C', 'S', 'F', ' '));
    append_le32(bytes, 3); append_le32(bytes, 1); append_le32(bytes, 1);
    append_le32(bytes, 0); append_le32(bytes, 7);
    append_le32(bytes, tag('L', 'B', 'L', ' ')); append_le32(bytes, 1); append_le32(bytes, 5);
    bytes.insert(bytes.end(), {'H', 'E', 'L', 'L', 'O'});
    append_le32(bytes, tag('S', 'T', 'R', 'W')); append_le32(bytes, 4);
    for (char16_t value : std::u16string{u'A', char16_t{0xd83d}, char16_t{0xde80}, u'Z'}) {
        const auto encoded = static_cast<std::uint16_t>(~value);
        bytes.push_back(static_cast<std::uint8_t>(encoded));
        bytes.push_back(static_cast<std::uint8_t>(encoded >> 8U));
    }
    append_le32(bytes, 5); bytes.insert(bytes.end(), {'v', 'o', 'i', 'c', 'e'});
    return bytes;
}

void write_text(const std::filesystem::path& path, std::string_view contents)
{
    std::filesystem::create_directories(path.parent_path());
    std::ofstream(path, std::ios::binary).write(contents.data(), static_cast<std::streamsize>(contents.size()));
}

std::vector<std::pair<std::filesystem::path, std::uintmax_t>> snapshot(const std::filesystem::path& root)
{
    std::vector<std::pair<std::filesystem::path, std::uintmax_t>> result;
    for (const auto& entry : std::filesystem::recursive_directory_iterator(root)) {
        if (entry.is_regular_file()) result.emplace_back(entry.path().lexically_relative(root), entry.file_size());
    }
    std::sort(result.begin(), result.end());
    return result;
}
}

int main()
{
    using namespace zh::original_data;
    const auto root = std::filesystem::temp_directory_path() /
        ("zh-original-data-config-" + std::to_string(static_cast<long long>(::getpid())));
    std::error_code ignored;
    std::filesystem::remove_all(root, ignored);
    const auto zero_hour = root / "zh";
    const auto generals = root / "generals";
    write_text(zero_hour / "Data/INI/B.ini", "Fixture\nName root-b\nEnabled Yes\nCount 0x2a\nScale 1.25\nWide Rocket-\xF0\x9F\x9A\x80\nKey Alpha\nEND\n");
    write_text(zero_hour / "Data/INI/a.INI", "Fixture\nName root-a\nEnabled No\nCount -7\nScale 2.5\nWide Omega\nKey Beta\nEND\n");
    write_text(zero_hour / "Data/INI/Sub/0.ini", "Fixture\nName nested\nEnabled Yes\nCount 3\nScale 4\nWide Child\nKey Gamma\nEND\n");
    write_text(generals / "Data/INI/base.ini", "Fixture\nName base\nEnabled Yes\nCount 1\nScale 1\nWide Base\nKey Base\nEND\n");
    const auto before_zh = snapshot(zero_hour);
    const auto before_generals = snapshot(generals);

    const auto vfs = zh::data::VirtualFileSystem::mount({zero_hour, generals, "English", {}});
    const LogicalFiles files(vfs);
    const IniSchema schema{{"Name", IniFieldType::ascii}, {"Enabled", IniFieldType::boolean},
        {"Count", IniFieldType::integer}, {"Scale", IniFieldType::real}, {"Wide", IniFieldType::utf16},
        {"Key", IniFieldType::name_key}};
    const auto blocks = load_ini_directory(files, "Data/INI", "Fixture", schema);
    check(blocks.size() == 4, "all effective root and nested INIs loaded");
    check(std::get<std::string>(blocks[0].fields.at("Name")) == "root-a", "root files sort first");
    check(std::get<std::string>(blocks[1].fields.at("Name")) == "root-b", "all root files globally sorted");
    check(std::get<std::string>(blocks[2].fields.at("Name")) == "base", "remaining root before subdirectories");
    check(std::get<std::string>(blocks[3].fields.at("Name")) == "nested", "subdirectories load second");
    check(std::get<std::int32_t>(blocks[1].fields.at("Count")) == 42, "hex Int scanner");
    check(std::get<bool>(blocks[1].fields.at("Enabled")), "Bool scanner");
    check(std::get<std::u16string>(blocks[1].fields.at("Wide")).size() == 9, "UTF-16 surrogate pair");
    check(std::get<std::uint32_t>(blocks[1].fields.at("Key")) != 0, "name key produced");
    check(std::string(provider_file_identity()) == "OriginalFileSystem.cpp", "file provider witness");
    check(std::string(provider_ini_identity()) == "OriginalINI.cpp", "INI provider witness");

    const auto csf = parse_csf(csf_fixture());
    check(csf.language == 7 && csf.entries.size() == 1, "CSF header and label parsed");
    check(csf.entries[0].text == std::u16string({u'A', char16_t{0xd83d}, char16_t{0xde80}, u'Z'}),
        "CSF inverted UTF-16 preserved");
    check(csf.entries[0].speech == "voice", "CSF speech parsed");
    check(std::string(provider_csf_identity()) == "OriginalCSF.cpp", "CSF provider witness");

    expect_error([&] { files.read("../escape.ini"); }, "logical");
    expect_error([&] { parse_ini(std::vector<std::uint8_t>{'F','i','x','t','u','r','e','\n','E','n','a','b','l','e','d',' ','M','a','y','b','e','\n','E','N','D','\n'}, "Fixture", schema); }, "Bool");
    expect_error([&] { auto bad = csf_fixture(); bad.pop_back(); parse_csf(bad); }, "truncated");
    expect_error([&] { auto bad = csf_fixture(); bad[0] = 0; parse_csf(bad); }, "identifier");
    Limits tiny; tiny.maximum_file_bytes = 3;
    expect_error([&] { parse_csf(csf_fixture(), tiny); }, "limit");
    check(snapshot(zero_hour) == before_zh && snapshot(generals) == before_generals, "read roots unchanged");
    const auto raw_before = zh::original_process::live_raw_allocations();
    { const auto ownership_probe = parse_csf(csf_fixture()); check(!ownership_probe.entries.empty(), "ownership probe parsed"); }
    check(zh::original_process::live_raw_allocations() == raw_before, "config consumer releases raw allocations");

    std::cout << "original-data config: ok providers=" << provider_file_identity() << ','
              << provider_ini_identity() << ',' << provider_csf_identity()
              << " blocks=" << blocks.size() << " labels=" << csf.entries.size()
              << " raw=" << zh::original_process::live_raw_allocations() << '\n';
    std::filesystem::remove_all(root, ignored);
    return failures == 0 ? 0 : 1;
}
