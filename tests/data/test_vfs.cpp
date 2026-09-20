#include "zh/data/vfs.h"

#include <array>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>
#include <utility>
#include <vector>

namespace {
int failures = 0;
void check(bool condition, const char* message)
{
    if (!condition) { std::cerr << "FAIL: " << message << '\n'; ++failures; }
}

template <typename Function>
void expect_error(Function&& function, std::string_view expected)
{
    try { function(); check(false, "expected data error"); }
    catch (const std::exception& error) {
        check(std::string_view(error.what()).find(expected) != std::string_view::npos, "data error diagnostic");
    }
}

void append_be32(std::vector<unsigned char>& bytes, std::uint32_t value)
{
    bytes.push_back(static_cast<unsigned char>(value >> 24U));
    bytes.push_back(static_cast<unsigned char>(value >> 16U));
    bytes.push_back(static_cast<unsigned char>(value >> 8U));
    bytes.push_back(static_cast<unsigned char>(value));
}

void set_be32(std::vector<unsigned char>& bytes, std::size_t offset, std::uint32_t value)
{
    bytes[offset] = static_cast<unsigned char>(value >> 24U);
    bytes[offset + 1] = static_cast<unsigned char>(value >> 16U);
    bytes[offset + 2] = static_cast<unsigned char>(value >> 8U);
    bytes[offset + 3] = static_cast<unsigned char>(value);
}

void set_le32(std::vector<unsigned char>& bytes, std::size_t offset, std::uint32_t value)
{
    bytes[offset] = static_cast<unsigned char>(value);
    bytes[offset + 1] = static_cast<unsigned char>(value >> 8U);
    bytes[offset + 2] = static_cast<unsigned char>(value >> 16U);
    bytes[offset + 3] = static_cast<unsigned char>(value >> 24U);
}

void write_big(
    const std::filesystem::path& path,
    const std::vector<std::pair<std::string, std::string>>& entries,
    std::string_view identifier = "BIGF")
{
    std::vector<unsigned char> bytes(16, 0);
    std::copy(identifier.begin(), identifier.end(), bytes.begin());
    std::uint32_t table_end = 16;
    for (const auto& entry : entries) table_end += 8 + static_cast<std::uint32_t>(entry.first.size()) + 1;
    std::uint32_t data_offset = table_end;
    for (const auto& [name, contents] : entries) {
        append_be32(bytes, contents.empty() ? 0 : data_offset);
        append_be32(bytes, static_cast<std::uint32_t>(contents.size()));
        bytes.insert(bytes.end(), name.begin(), name.end());
        bytes.push_back(0);
        data_offset += static_cast<std::uint32_t>(contents.size());
    }
    for (const auto& entry : entries) bytes.insert(bytes.end(), entry.second.begin(), entry.second.end());
    set_le32(bytes, 4, static_cast<std::uint32_t>(bytes.size()));
    set_be32(bytes, 8, static_cast<std::uint32_t>(entries.size()));
    set_be32(bytes, 12, table_end);
    std::filesystem::create_directories(path.parent_path());
    std::ofstream stream(path, std::ios::binary);
    stream.write(reinterpret_cast<const char*>(bytes.data()), static_cast<std::streamsize>(bytes.size()));
}

void write_text(const std::filesystem::path& path, std::string_view contents)
{
    std::filesystem::create_directories(path.parent_path());
    std::ofstream(path) << contents;
}

std::string text(const std::vector<zh::foundation::UInt8>& bytes)
{
    return {bytes.begin(), bytes.end()};
}
}

int main()
{
    using namespace zh::data;
    const auto root = std::filesystem::temp_directory_path() / "zh-vfs-test";
    std::error_code ignored;
    std::filesystem::remove_all(root, ignored);
    const auto zh = root / "zh";
    const auto generals = root / "generals";
    const auto mods = root / "mods";
    write_text(zh / "Same.TXT", "zh-loose");
    write_text(zh / "zh-loose.txt", "zh");
    write_text(generals / "same.txt", "generals-loose");
    write_text(generals / "base-loose.txt", "base");
    write_big(mods / "mod.big", {{"same.txt", "mod"}, {"mod-only.txt", "mod-only"}});
    write_big(zh / "B.big", {{"Archive-Only.txt", "B"}});
    write_big(zh / "a.BIG", {{"archive-only.txt", "A"}, {"zh-big.txt", "zh-big"}});
    write_big(generals / "base.big", {{"same.txt", "base-big"}, {"base-big.txt", "base-big"}});

    DataSelection selection{zh, generals, "English", {mods}};
    const auto vfs = VirtualFileSystem::mount(selection);
    check(text(vfs.read_prefix("same.txt", 100)) == "zh-loose", "Zero Hour loose wins");
    check(text(vfs.read_prefix("MOD-ONLY.TXT", 100)) == "mod-only", "mod archive mounted case-insensitively");
    check(text(vfs.read_prefix("archive-only.txt", 100)) == "A", "case-insensitive archive order is first-loaded wins");
    check(text(vfs.read_prefix("base-loose.txt", 100)) == "base", "Generals loose mounted");
    check(text(vfs.read_prefix("base-big.txt", 100)) == "base-big", "Generals BIG mounted");
    check(vfs.archive_mount_order().size() == 4, "all archive layers reported");
    check(vfs.archive_mount_order()[1].find("a.BIG") != std::string::npos, "stable archive order reported");
    expect_error([] { zh::foundation::normalize_logical_path("../escape"); }, "traversal");
    expect_error([] { zh::foundation::normalize_logical_path("/absolute"); }, "relative");
    expect_error([&] { vfs.read_prefix("missing.txt", 4); }, "missing logical resource");

    const auto malformed = root / "malformed.big";
    write_big(malformed, {{"ok", "x"}}, "NOPE");
    expect_error([&] { BigArchive::open(malformed); }, "identifier");
    write_big(malformed, {{"../escape", "x"}});
    expect_error([&] { BigArchive::open(malformed); }, "invalid BIG logical path");
    write_big(malformed, {{"Case", "x"}, {"case", "y"}});
    expect_error([&] { BigArchive::open(malformed); }, "case-fold collision");

    write_big(malformed, {{"ok", "x"}});
    { std::fstream stream(malformed, std::ios::binary | std::ios::in | std::ios::out);
      const char non_nul = 'x'; stream.seekp(26); stream.write(&non_nul, 1); }
    expect_error([&] { BigArchive::open(malformed); }, "not NUL terminated");

    write_big(malformed, {{"ok", "x"}});
    { std::fstream stream(malformed, std::ios::binary | std::ios::in | std::ios::out);
      std::array<char, 4> bad_offset{char(0xff), char(0xff), char(0xff), char(0xff)};
      stream.seekp(16); stream.write(bad_offset.data(), 4); }
    expect_error([&] { BigArchive::open(malformed); }, "entry range");

    write_big(malformed, {{"ok", "x"}});
    BigLimits entry_limit;
    entry_limit.maximum_entry_bytes = 0;
    expect_error([&] { BigArchive::open(malformed, entry_limit); }, "entry range");
    BigLimits archive_limit;
    archive_limit.maximum_archive_bytes = 10;
    expect_error([&] { BigArchive::open(malformed, archive_limit); }, "archive exceeds");

    write_big(malformed, {{"ok", "x"}});
    { std::fstream stream(malformed, std::ios::binary | std::ios::in | std::ios::out);
      std::array<char, 4> bad_count{char(0x7f), char(0xff), char(0xff), char(0xff)};
      stream.seekp(8); stream.write(bad_count.data(), 4); }
    expect_error([&] { BigArchive::open(malformed, BigLimits{1}); }, "entry count");

    write_big(malformed, {{"ok", "x"}});
    { std::fstream stream(malformed, std::ios::binary | std::ios::in | std::ios::out);
      std::array<char, 4> bad_end{char(0xff), char(0xff), char(0xff), char(0xff)};
      stream.seekp(12); stream.write(bad_end.data(), 4); }
    expect_error([&] { BigArchive::open(malformed); }, "table boundary");

    write_big(malformed, {{"ok", "x"}});
    std::filesystem::resize_file(malformed, 20);
    expect_error([&] { BigArchive::open(malformed); }, "declared size");

    const auto collision_root = root / "collision";
    write_text(collision_root / "Name.ini", "one");
    write_text(collision_root / "name.ini", "two");
    DataSelection collision{collision_root, collision_root, "English", {}};
    expect_error([&] { VirtualFileSystem::mount(collision); }, "loose collision");

    const auto archive_collision = root / "archive-collision";
    write_big(archive_collision / "Name.big", {{"one", "x"}});
    write_big(archive_collision / "name.BIG", {{"two", "x"}});
    DataSelection colliding_archives{archive_collision, archive_collision, "English", {}};
    expect_error([&] { VirtualFileSystem::mount(colliding_archives); }, "archive collision");

    const auto combined = root / "combined";
    write_text(combined / "unique.txt", "one");
    const auto combined_vfs = VirtualFileSystem::mount({combined, combined, "English", {}});
    check(combined_vfs.resources().size() == 1, "identical roots mounted once");

    std::filesystem::remove_all(root, ignored);
    return failures == 0 ? 0 : 1;
}
