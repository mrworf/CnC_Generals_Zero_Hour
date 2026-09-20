#pragma once

#include "zh/data/vfs.h"

#include <cstdint>
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

const char* provider_file_identity() noexcept;
const char* provider_ini_identity() noexcept;
const char* provider_csf_identity() noexcept;

} // namespace zh::original_data
