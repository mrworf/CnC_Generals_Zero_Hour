#include "zh/original_data.h"
#include "zh/original_process.h"

#include <algorithm>
#include <filesystem>
#include <fstream>
#include <iostream>
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
void append_u32(std::vector<std::uint8_t>& output, std::uint32_t value)
{ for (unsigned shift = 0; shift < 32; shift += 8) output.push_back(static_cast<std::uint8_t>(value >> shift)); }

std::vector<std::uint8_t> map_bytes(std::string display, std::int32_t width = 100,
    std::int32_t height = 80, bool nonempty_objects = false)
{
    using namespace zh::original_data;
    XferRecord world; world.ascii = std::move(display);
    std::vector<std::uint8_t> heights;
    append_u32(heights, static_cast<std::uint32_t>(width)); append_u32(heights, static_cast<std::uint32_t>(height));
    append_u32(heights, 5); append_u32(heights, 2);
    append_u32(heights, 5); append_u32(heights, 5); append_u32(heights, 94); append_u32(heights, 74);
    return write_data_chunks({{"WorldInfo", 1, write_xfer_record(world)}, {"HeightMapData", 4, heights},
        {"ObjectsList", 2, nonempty_objects ? std::vector<std::uint8_t>{1} : std::vector<std::uint8_t>{}},
        {"UnknownFutureChunk", 9, {1, 2, 3}}});
}

void write_bytes(const std::filesystem::path& path, const std::vector<std::uint8_t>& bytes)
{
    std::filesystem::create_directories(path.parent_path());
    std::ofstream stream(path, std::ios::binary | std::ios::trunc);
    stream.write(reinterpret_cast<const char*>(bytes.data()), static_cast<std::streamsize>(bytes.size()));
}

std::vector<std::pair<std::filesystem::path, std::vector<std::uint8_t>>> snapshot(const std::filesystem::path& root)
{
    std::vector<std::pair<std::filesystem::path, std::vector<std::uint8_t>>> result;
    for (const auto& entry : std::filesystem::recursive_directory_iterator(root)) {
        if (!entry.is_regular_file()) continue;
        std::ifstream stream(entry.path(), std::ios::binary);
        result.emplace_back(entry.path().lexically_relative(root),
            std::vector<std::uint8_t>((std::istreambuf_iterator<char>(stream)), std::istreambuf_iterator<char>()));
    }
    std::sort(result.begin(), result.end()); return result;
}
}

int main()
{
    using namespace zh::original_data;
    const auto root = std::filesystem::temp_directory_path() /
        ("zh-original-map-" + std::to_string(static_cast<long long>(::getpid())));
    std::error_code ignored; std::filesystem::remove_all(root, ignored);
    const auto zh = root / "zh"; const auto generals = root / "generals"; const auto xdg = root / "xdg-cache";
    write_bytes(zh / "Maps/Alpha.map", map_bytes("standard alpha"));
    write_bytes(zh / "Maps/Beta.map", map_bytes("standard beta", 128, 96));
    write_bytes(zh / "UserMaps/alpha.MAP", map_bytes("user alpha", 120, 90));
    std::filesystem::create_directories(generals);
    const auto roots_before = std::make_pair(snapshot(zh), snapshot(generals));
    const auto old_cwd = std::filesystem::current_path(); std::filesystem::current_path(root);

    const auto vfs = zh::data::VirtualFileSystem::mount({zh, generals, "English", {}});
    const LogicalFiles files(vfs);
    const auto cold = load_map_catalog(files, "Maps", "UserMaps", {xdg, {}});
    check(!cold.warm_cache && cold.maps.size() == 2, "cold cache enumerates effective maps");
    check(cold.maps[0].identity == "alpha" && cold.maps[0].user_map && cold.maps[0].display_name == "user alpha",
        "user map overrides standard map identity");
    check(cold.maps[1].identity == "beta" && !cold.maps[1].user_map, "standard map retained");
    check(cold.maps[0].width == 120 && cold.maps[0].height == 90 && cold.maps[0].border == 5 &&
        cold.maps[0].boundaries == std::vector<MapBoundary>{{5, 5}, {94, 74}}, "height and boundaries parsed");
    check(cold.cache_path == xdg / "generals-zero-hour/map-metadata.cache", "cache path is XDG-confined");
    const auto warm = load_map_catalog(files, "Maps", "UserMaps", {xdg, {}});
    check(warm.warm_cache && warm.maps == cold.maps, "warm cache equals cold metadata");
    check(cold.maps[0].crc == 0x17abf264U && cold.maps[1].crc == 0xdda2c265U, "source CRCs characterized");
    check(snapshot(zh) == roots_before.first && snapshot(generals) == roots_before.second, "metadata reads leave read roots unchanged");

    write_bytes(cold.cache_path, {1, 2, 3});
    const auto corrupt = load_map_catalog(files, "Maps", "UserMaps", {xdg, {}});
    check(!corrupt.warm_cache && corrupt.maps == cold.maps, "malformed cache rebuilds from authoritative maps");
    check(load_map_catalog(files, "Maps", "UserMaps", {xdg, {}}).warm_cache, "malformed-cache rebuild becomes warm");

    const auto original_cache = snapshot(xdg);
    write_bytes(zh / "UserMaps/alpha.MAP", map_bytes("changed alpha", 122, 92));
    const auto changed_vfs = zh::data::VirtualFileSystem::mount({zh, generals, "English", {}});
    const LogicalFiles changed_files(changed_vfs);
    expect_error([&] {
        load_map_catalog(changed_files, "Maps", "UserMaps", {xdg,
            [](const auto&, const auto&) { throw std::runtime_error("injected denial"); }});
    }, "cannot write");
    check(snapshot(xdg) == original_cache, "denied rebuild preserves prior cache atomically");
    const auto stale = load_map_catalog(changed_files, "Maps", "UserMaps", {xdg, {}});
    check(!stale.warm_cache && stale.maps[0].display_name == "changed alpha", "stale cache rebuilds from map");
    check(load_map_catalog(changed_files, "Maps", "UserMaps", {xdg, {}}).warm_cache, "rebuilt cache becomes warm");

    write_bytes(zh / "UserMaps/Bad.map", map_bytes("bad", 100, 80, true));
    const auto bad_vfs = zh::data::VirtualFileSystem::mount({zh, generals, "English", {}});
    const LogicalFiles bad_files(bad_vfs);
    expect_error([&] { load_map_catalog(bad_files, "Maps", "UserMaps", {root / "bad-cache", {}}); }, "M20");
    write_bytes(zh / "UserMaps/Bad.map", {1, 2, 3});
    const auto truncated_vfs = zh::data::VirtualFileSystem::mount({zh, generals, "English", {}});
    const LogicalFiles truncated_files(truncated_vfs);
    expect_error([&] { load_map_catalog(truncated_files, "Maps", "UserMaps", {root / "bad-cache", {}}); }, "truncated");
    expect_error([&] { load_map_catalog(files, "Maps", "UserMaps", {"relative-cache", {}}); }, "absolute");
    const auto raw_before = zh::original_process::live_raw_allocations();
    { const auto ownership_probe = load_map_catalog(changed_files, "Maps", "UserMaps", {xdg, {}});
      check(ownership_probe.warm_cache, "ownership probe used warm cache"); }
    check(zh::original_process::live_raw_allocations() == raw_before, "map consumer releases raw allocations");

    std::filesystem::current_path(old_cwd);
    check(snapshot(zh).size() == roots_before.first.size() + 1, "only explicit owned malformed fixture changed read root");
    check(snapshot(generals) == roots_before.second, "base read root unchanged");
    check(std::string_view(provider_map_identity()) == "OriginalMapMetadata.cpp", "map provider witness");
    std::cout << "original-data map: ok provider=" << provider_map_identity()
              << " maps=" << cold.maps.size() << " cache=xdg crcs=" << std::hex
              << cold.maps[0].crc << ',' << cold.maps[1].crc << std::dec
              << " cache-files=1 raw=" << zh::original_process::live_raw_allocations() << '\n';
    std::filesystem::remove_all(root, ignored);
    return failures == 0 ? 0 : 1;
}
