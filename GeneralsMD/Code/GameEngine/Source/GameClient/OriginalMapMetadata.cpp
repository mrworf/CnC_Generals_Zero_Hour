/*
** Command & Conquer Generals Zero Hour(tm)
** Copyright 2025 Electronic Arts Inc.
** GPL-3.0-or-later
*/

// M27 extraction provenance: MapUtil.cpp calcCRC, ParseWorldDictDataChunk,
// ParseSizeOnly, loadMap and MapCache cache-path/write operations. The
// ThingFactory-dependent ParseObjectDataChunk, addMap, INIMapCache and map.str
// localization remain intact in MapUtil.cpp and are explicitly M20-owned.

#include "zh/original_data.h"

#include "GameLogic/FPUControl.h"

#include <algorithm>
#include <cctype>
#include <fstream>
#include <map>
#include <system_error>
#include <unistd.h>

namespace zh::original_data {
namespace {

constexpr std::string_view cache_magic{"ZHMC27\0", 7};

void append_u32(std::vector<std::uint8_t>& output, std::uint32_t value)
{ for (unsigned shift = 0; shift < 32; shift += 8) output.push_back(static_cast<std::uint8_t>(value >> shift)); }

class Reader {
public:
    explicit Reader(const std::vector<std::uint8_t>& bytes) : bytes_(bytes) {}
    std::uint32_t u32()
    {
        if (bytes_.size() - cursor_ < 4) throw Error("truncated map metadata");
        std::uint32_t result = 0;
        for (unsigned shift = 0; shift < 32; shift += 8) result |= static_cast<std::uint32_t>(bytes_[cursor_++]) << shift;
        return result;
    }
    std::string text(std::size_t maximum)
    {
        const auto size = u32();
        if (size > maximum || size > bytes_.size() - cursor_) throw Error("truncated or oversized map metadata string");
        std::string result(bytes_.begin() + static_cast<std::ptrdiff_t>(cursor_),
            bytes_.begin() + static_cast<std::ptrdiff_t>(cursor_ + size)); cursor_ += size; return result;
    }
    bool empty() const noexcept { return cursor_ == bytes_.size(); }
private:
    const std::vector<std::uint8_t>& bytes_;
    std::size_t cursor_ = 0;
};

void append_text(std::vector<std::uint8_t>& output, std::string_view value)
{
    if (value.size() > UINT32_MAX) throw Error("map metadata string cannot be represented");
    append_u32(output, static_cast<std::uint32_t>(value.size())); output.insert(output.end(), value.begin(), value.end());
}

std::uint32_t source_crc(const std::vector<std::uint8_t>& bytes) noexcept
{
    std::uint32_t crc = 0;
    for (const auto value : bytes) {
        const bool high = (crc & 0x80000000U) != 0;
        crc <<= 1U; crc += value; crc += high ? 1U : 0U;
    }
    return crc;
}

std::string folded(std::string value)
{
    std::transform(value.begin(), value.end(), value.begin(),
        [](unsigned char character) { return static_cast<char>(std::tolower(character)); });
    return value;
}

std::string identity_for(const std::string& logical)
{
    const auto slash = logical.find_last_of("/\\");
    auto name = logical.substr(slash == std::string::npos ? 0 : slash + 1);
    const auto dot = name.find_last_of('.');
    if (dot != std::string::npos) name.resize(dot);
    if (name.empty()) throw Error("map logical name has no identity");
    return folded(std::move(name));
}

MapMetadata parse_map(const LogicalFiles& files, const std::string& logical, bool user, const Limits& limits)
{
    setFPMode();
    const auto bytes = files.read(logical);
    const auto chunks = read_data_chunks(bytes, limits);
    MapMetadata result;
    result.identity = identity_for(logical); result.logical_name = logical;
    result.crc = source_crc(bytes); result.user_map = user;
    bool world = false, height = false, objects = false;
    for (const auto& chunk : chunks) {
        if (chunk.label == "WorldInfo") {
            if (world) throw Error("duplicate WorldInfo chunk in map '" + logical + "'");
            result.display_name = read_xfer_record(chunk.payload, limits).ascii; world = true;
        } else if (chunk.label == "HeightMapData") {
            if (height) throw Error("duplicate HeightMapData chunk in map '" + logical + "'");
            Reader reader(chunk.payload);
            result.width = static_cast<std::int32_t>(reader.u32()); result.height = static_cast<std::int32_t>(reader.u32());
            result.border = chunk.version >= 3 ? static_cast<std::int32_t>(reader.u32()) : 0;
            const auto count = chunk.version >= 4 ? reader.u32() : 0;
            if (count > limits.maximum_records) throw Error("map boundary count exceeds configured limit");
            for (std::uint32_t index = 0; index < count; ++index)
                result.boundaries.push_back({static_cast<std::int32_t>(reader.u32()), static_cast<std::int32_t>(reader.u32())});
            if (!reader.empty()) throw Error("HeightMapData chunk has trailing data");
            if (result.width <= 0 || result.height <= 0 || result.width > 32768 || result.height > 32768 ||
                result.border < 0 || result.border * 2 >= result.width || result.border * 2 >= result.height)
                throw Error("invalid map height dimensions");
            height = true;
        } else if (chunk.label == "ObjectsList") {
            if (objects) throw Error("duplicate ObjectsList chunk in map '" + logical + "'");
            if (!chunk.payload.empty()) throw Error("nonempty ObjectsList requires M20 ThingFactory integration");
            objects = true;
        }
    }
    if (!world || !height || !objects) throw Error("map is missing independent metadata chunks");
    return result;
}

std::uint32_t catalog_fingerprint(const std::vector<MapMetadata>& maps) noexcept
{
    std::uint32_t crc = 0;
    const auto add = [&crc](std::uint8_t value) { const bool high = crc & 0x80000000U; crc <<= 1U; crc += value; crc += high; };
    for (const auto& map : maps) {
        for (const auto character : map.identity) add(static_cast<std::uint8_t>(character));
        for (unsigned shift = 0; shift < 32; shift += 8) add(static_cast<std::uint8_t>(map.crc >> shift));
    }
    return crc;
}

std::vector<std::uint8_t> serialize_cache(const std::vector<MapMetadata>& maps, const Limits& limits)
{
    std::vector<std::uint8_t> output(cache_magic.begin(), cache_magic.end());
    append_u32(output, catalog_fingerprint(maps)); append_u32(output, static_cast<std::uint32_t>(maps.size()));
    for (const auto& map : maps) {
        append_text(output, map.identity); append_text(output, map.logical_name); append_text(output, map.display_name);
        append_u32(output, static_cast<std::uint32_t>(map.width)); append_u32(output, static_cast<std::uint32_t>(map.height));
        append_u32(output, static_cast<std::uint32_t>(map.border)); append_u32(output, map.crc); append_u32(output, map.user_map ? 1U : 0U);
        append_u32(output, static_cast<std::uint32_t>(map.boundaries.size()));
        for (const auto& boundary : map.boundaries) {
            append_u32(output, static_cast<std::uint32_t>(boundary.x)); append_u32(output, static_cast<std::uint32_t>(boundary.y));
        }
    }
    if (output.size() > limits.maximum_file_bytes) throw Error("map cache output exceeds configured limit");
    return output;
}

std::vector<MapMetadata> deserialize_cache(const std::vector<std::uint8_t>& bytes, std::uint32_t expected, const Limits& limits)
{
    if (bytes.size() < cache_magic.size() || !std::equal(cache_magic.begin(), cache_magic.end(), bytes.begin()))
        throw Error("invalid map cache identifier");
    std::vector<std::uint8_t> body(bytes.begin() + static_cast<std::ptrdiff_t>(cache_magic.size()), bytes.end());
    Reader reader(body);
    if (reader.u32() != expected) throw Error("stale map cache fingerprint");
    const auto count = reader.u32();
    if (count > limits.maximum_records) throw Error("map cache count exceeds configured limit");
    std::vector<MapMetadata> maps;
    for (std::uint32_t index = 0; index < count; ++index) {
        MapMetadata map;
        map.identity = reader.text(limits.maximum_line_bytes); map.logical_name = reader.text(limits.maximum_line_bytes);
        map.display_name = reader.text(limits.maximum_line_bytes); map.width = static_cast<std::int32_t>(reader.u32());
        map.height = static_cast<std::int32_t>(reader.u32()); map.border = static_cast<std::int32_t>(reader.u32());
        map.crc = reader.u32(); const auto user = reader.u32();
        if (user > 1) throw Error("invalid map cache Bool");
        map.user_map = user != 0;
        const auto boundaries = reader.u32();
        if (boundaries > limits.maximum_records) throw Error("map cache boundary count exceeds limit");
        for (std::uint32_t boundary = 0; boundary < boundaries; ++boundary)
            map.boundaries.push_back({static_cast<std::int32_t>(reader.u32()), static_cast<std::int32_t>(reader.u32())});
        maps.push_back(std::move(map));
    }
    if (!reader.empty()) throw Error("map cache has trailing data");
    return maps;
}

void atomic_write(const std::filesystem::path& path, const std::vector<std::uint8_t>& bytes)
{
    std::error_code error;
    std::filesystem::create_directories(path.parent_path(), error);
    if (error) throw Error("cannot create XDG map cache directory: " + error.message());
    const auto temporary = path.string() + ".tmp-" + std::to_string(static_cast<long long>(::getpid()));
    {
        std::ofstream stream(temporary, std::ios::binary | std::ios::trunc);
        if (!stream) throw Error("cannot open temporary XDG map cache");
        stream.write(reinterpret_cast<const char*>(bytes.data()), static_cast<std::streamsize>(bytes.size()));
        if (!stream) { std::filesystem::remove(temporary, error); throw Error("cannot write temporary XDG map cache"); }
    }
    std::filesystem::rename(temporary, path, error);
    if (error) { std::filesystem::remove(temporary, error); throw Error("cannot replace XDG map cache"); }
}

} // namespace

bool MapMetadata::operator==(const MapMetadata& other) const noexcept
{
    return identity == other.identity && logical_name == other.logical_name && display_name == other.display_name &&
        width == other.width && height == other.height && border == other.border && boundaries == other.boundaries &&
        crc == other.crc && user_map == other.user_map;
}

MapCatalog load_map_catalog(const LogicalFiles& files, std::string_view standard_directory,
    std::string_view user_directory, const MapCacheOptions& options, const Limits& limits)
{
    if (!options.xdg_cache_root.is_absolute()) throw Error("XDG cache root must be absolute");
    MapCatalog result;
    const auto normalized_root = options.xdg_cache_root.lexically_normal();
    result.cache_path = (normalized_root / "generals-zero-hour" / "map-metadata.cache").lexically_normal();
    const auto relative = result.cache_path.lexically_relative(normalized_root);
    if (relative.empty() || *relative.begin() == "..") throw Error("map cache path escapes XDG cache root");
    std::map<std::string, MapMetadata> selected;
    for (const auto& logical : files.enumerate_directory(standard_directory, ".map", true))
        selected.insert_or_assign(identity_for(logical), parse_map(files, logical, false, limits));
    for (const auto& logical : files.enumerate_directory(user_directory, ".map", true))
        selected.insert_or_assign(identity_for(logical), parse_map(files, logical, true, limits));
    for (auto& [identity, map] : selected) result.maps.push_back(std::move(map));
    const auto fingerprint = catalog_fingerprint(result.maps);
    std::error_code error;
    if (std::filesystem::is_regular_file(result.cache_path, error) && !error) {
        const auto cache_size = std::filesystem::file_size(result.cache_path, error);
        if (error || cache_size > limits.maximum_file_bytes) {
            error.clear();
        } else {
        std::ifstream stream(result.cache_path, std::ios::binary);
        const std::vector<std::uint8_t> cached((std::istreambuf_iterator<char>(stream)), std::istreambuf_iterator<char>());
        try {
            const auto decoded = deserialize_cache(cached, fingerprint, limits);
            if (decoded == result.maps) { result.maps = decoded; result.warm_cache = true; return result; }
        } catch (const Error&) { /* stale/malformed caches rebuild from authoritative map input */ }
        }
    }
    const auto encoded = serialize_cache(result.maps, limits);
    try { (options.writer ? options.writer : CacheWriter{atomic_write})(result.cache_path, encoded); }
    catch (const Error&) { throw; }
    catch (const std::exception& failure) { throw Error("cannot write XDG map cache: " + std::string(failure.what())); }
    return result;
}

const char* provider_map_identity() noexcept { return "OriginalMapMetadata.cpp"; }

} // namespace zh::original_data
