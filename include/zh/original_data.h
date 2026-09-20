#pragma once

#include "zh/data/vfs.h"

#include <cstdint>
#include <filesystem>
#include <functional>
#include <map>
#include <stdexcept>
#include <string>
#include <string_view>
#include <variant>
#include <vector>

namespace zh::original_data {

class Error : public std::runtime_error {
public:
    using std::runtime_error::runtime_error;
};

struct Limits {
    std::size_t maximum_file_bytes = 8U * 1024U * 1024U;
    std::size_t maximum_line_bytes = 4096;
    std::size_t maximum_records = 100000;
    std::size_t maximum_string_code_units = 1024U * 1024U;
};

class LogicalFiles {
public:
    explicit LogicalFiles(const data::VirtualFileSystem& vfs, Limits limits = {});

    std::vector<std::uint8_t> read(std::string_view logical_name) const;
    std::vector<std::string> enumerate_directory(
        std::string_view directory,
        std::string_view extension,
        bool recursive) const;
    const data::VfsResource& describe(std::string_view logical_name) const;

private:
    const data::VirtualFileSystem& vfs_;
    Limits limits_;
};

enum class IniFieldType { boolean, integer, real, ascii, utf16, name_key };
using IniValue = std::variant<bool, std::int32_t, float, std::string, std::u16string, std::uint32_t>;
using IniSchema = std::map<std::string, IniFieldType>;

struct IniBlock {
    std::string type;
    std::map<std::string, IniValue> fields;
};

std::vector<IniBlock> parse_ini(
    const std::vector<std::uint8_t>& bytes,
    std::string_view block_type,
    const IniSchema& schema,
    const Limits& limits = {});

std::vector<IniBlock> load_ini_directory(
    const LogicalFiles& files,
    std::string_view directory,
    std::string_view block_type,
    const IniSchema& schema,
    bool recursive = true,
    const Limits& limits = {});

struct CsfEntry {
    std::string label;
    std::u16string text;
    std::string speech;
};

struct CsfDocument {
    std::int32_t language = 0;
    std::vector<CsfEntry> entries;
};

CsfDocument parse_csf(const std::vector<std::uint8_t>& bytes, const Limits& limits = {});

struct XferRecord {
    bool boolean = false;
    std::int32_t integer = 0;
    std::int64_t integer64 = 0;
    float real = 0.0F;
    std::string ascii;
    std::u16string unicode;
};

std::vector<std::uint8_t> write_xfer_record(const XferRecord& record, const Limits& limits = {});
XferRecord read_xfer_record(const std::vector<std::uint8_t>& bytes, const Limits& limits = {});

struct DataChunk {
    std::string label;
    std::uint16_t version = 0;
    std::vector<std::uint8_t> payload;
};

std::vector<std::uint8_t> write_data_chunks(const std::vector<DataChunk>& chunks, const Limits& limits = {});
std::vector<DataChunk> read_data_chunks(const std::vector<std::uint8_t>& bytes, const Limits& limits = {});

struct RandomCheckpoints {
    std::uint32_t initial_logic_crc = 0;
    std::uint32_t logic_crc_after_draws = 0;
    std::vector<std::int32_t> logic_values;
    std::vector<std::int32_t> client_values;
    std::vector<std::int32_t> audio_values;
};

RandomCheckpoints characterize_random_streams(
    std::uint32_t seed, std::size_t logic_draws, std::size_t client_draws, std::size_t audio_draws);

struct MapBoundary {
    std::int32_t x = 0;
    std::int32_t y = 0;
    bool operator==(const MapBoundary& other) const noexcept { return x == other.x && y == other.y; }
};

struct MapMetadata {
    std::string identity;
    std::string logical_name;
    std::string display_name;
    std::int32_t width = 0;
    std::int32_t height = 0;
    std::int32_t border = 0;
    std::vector<MapBoundary> boundaries;
    std::uint32_t crc = 0;
    bool user_map = false;
    bool operator==(const MapMetadata& other) const noexcept;
};

using CacheWriter = std::function<void(const std::filesystem::path&, const std::vector<std::uint8_t>&)>;

struct MapCacheOptions {
    std::filesystem::path xdg_cache_root;
    CacheWriter writer;
};

struct MapCatalog {
    std::vector<MapMetadata> maps;
    std::filesystem::path cache_path;
    bool warm_cache = false;
};

MapCatalog load_map_catalog(
    const LogicalFiles& files,
    std::string_view standard_directory,
    std::string_view user_directory,
    const MapCacheOptions& options,
    const Limits& limits = {});

const char* provider_file_identity() noexcept;
const char* provider_ini_identity() noexcept;
const char* provider_csf_identity() noexcept;
const char* provider_xfer_identity() noexcept;
const char* provider_chunk_identity() noexcept;
const char* provider_random_identity() noexcept;
const char* provider_map_identity() noexcept;

} // namespace zh::original_data
