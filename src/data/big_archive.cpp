#include "zh/data/big_archive.h"

#include "zh/foundation/platform.h"

#include <algorithm>
#include <array>
#include <fstream>
#include <limits>
#include <map>
#include <set>

namespace zh::data {
namespace {

std::uint32_t read_be32(const foundation::UInt8* bytes) noexcept
{
    return (static_cast<std::uint32_t>(bytes[0]) << 24U) |
        (static_cast<std::uint32_t>(bytes[1]) << 16U) |
        (static_cast<std::uint32_t>(bytes[2]) << 8U) | static_cast<std::uint32_t>(bytes[3]);
}

std::uint32_t read_le32(const foundation::UInt8* bytes) noexcept
{
    return static_cast<std::uint32_t>(bytes[0]) |
        (static_cast<std::uint32_t>(bytes[1]) << 8U) |
        (static_cast<std::uint32_t>(bytes[2]) << 16U) | (static_cast<std::uint32_t>(bytes[3]) << 24U);
}

std::vector<foundation::UInt8> read_range(
    const std::filesystem::path& path,
    std::uint64_t offset,
    std::size_t size)
{
    std::ifstream stream(path, std::ios::binary);
    if (!stream) throw DataError("cannot open BIG archive '" + path.filename().string() + "'");
    stream.seekg(static_cast<std::streamoff>(offset));
    if (!stream) throw DataError("cannot seek BIG archive '" + path.filename().string() + "'");
    std::vector<foundation::UInt8> result(size);
    if (size != 0) {
        stream.read(reinterpret_cast<char*>(result.data()), static_cast<std::streamsize>(size));
        if (!stream || stream.gcount() != static_cast<std::streamsize>(size)) {
            throw DataError("short read from BIG archive '" + path.filename().string() + "'");
        }
    }
    return result;
}

} // namespace

BigArchive BigArchive::open(const std::filesystem::path& path, const BigLimits& limits)
{
    std::error_code error;
    const auto file_size = std::filesystem::file_size(path, error);
    if (error) throw DataError("cannot determine BIG size for '" + path.filename().string() + "'");
    if (file_size < 16) throw DataError("BIG header is truncated in '" + path.filename().string() + "'");
    if (file_size > limits.maximum_archive_bytes) {
        throw DataError("BIG archive exceeds configured size limit: '" + path.filename().string() + "'");
    }
    const auto header = read_range(path, 0, 16);
    const std::string identifier(reinterpret_cast<const char*>(header.data()), 4);
    if (identifier != "BIGF" && identifier != "BIG4") {
        throw DataError("unsupported BIG identifier in '" + path.filename().string() + "'");
    }
    const auto declared_size = static_cast<std::uint64_t>(read_le32(header.data() + 4));
    const auto count = static_cast<std::uint64_t>(read_be32(header.data() + 8));
    const auto table_end = static_cast<std::uint64_t>(read_be32(header.data() + 12));
    if (declared_size != file_size) throw DataError("BIG declared size does not match file size in '" + path.filename().string() + "'");
    if (count > limits.maximum_entries) throw DataError("BIG entry count exceeds configured limit in '" + path.filename().string() + "'");
    if (table_end < 16 || table_end > file_size || table_end - 16 > limits.maximum_table_bytes) {
        throw DataError("BIG table boundary is invalid in '" + path.filename().string() + "'");
    }
    const auto table = read_range(path, 16, static_cast<std::size_t>(table_end - 16));
    std::size_t cursor = 0;
    std::vector<BigEntry> entries;
    entries.reserve(static_cast<std::size_t>(count));
    std::map<std::string, std::string> original_names;
    for (std::uint64_t index = 0; index < count; ++index) {
        if (table.size() - cursor < 9) throw DataError("BIG entry table is truncated in '" + path.filename().string() + "'");
        const std::uint64_t offset = read_be32(table.data() + cursor);
        const std::uint64_t size = read_be32(table.data() + cursor + 4);
        cursor += 8;
        const auto end = std::find(table.begin() + static_cast<std::ptrdiff_t>(cursor), table.end(), 0);
        if (end == table.end()) throw DataError("BIG entry name is not NUL terminated in '" + path.filename().string() + "'");
        const auto name_size = static_cast<std::size_t>(end - (table.begin() + static_cast<std::ptrdiff_t>(cursor)));
        if (name_size == 0 || name_size > limits.maximum_name_bytes) {
            throw DataError("BIG entry name length is invalid in '" + path.filename().string() + "'");
        }
        const std::string logical(reinterpret_cast<const char*>(table.data() + cursor), name_size);
        cursor += name_size + 1;
        std::string normalized;
        try { normalized = foundation::normalize_logical_path(logical); }
        catch (const foundation::PlatformError& invalid) {
            throw DataError("invalid BIG logical path '" + logical + "' in '" + path.filename().string() + "': " + invalid.what());
        }
        if (size > limits.maximum_entry_bytes || offset > file_size || size > file_size - offset ||
            (size != 0 && offset < table_end)) {
            throw DataError("BIG entry range is invalid for '" + logical + "' in '" + path.filename().string() + "'");
        }
        const auto [known, inserted] = original_names.emplace(normalized, logical);
        if (!inserted && known->second != logical) {
            throw DataError("case-fold collision in BIG '" + path.filename().string() + "': '" + known->second + "' and '" + logical + "'");
        }
        entries.push_back({logical, normalized, offset, size});
    }
    // Retail archives may align the first payload beyond the final NUL-terminated name.
    // The advertised boundary remains authoritative, and entry ranges cannot overlap it.
    BigArchive archive;
    archive.path_ = path;
    archive.entries_ = std::move(entries);
    return archive;
}

std::vector<foundation::UInt8> BigArchive::read_prefix(const BigEntry& entry, std::size_t maximum_bytes) const
{
    const auto amount = static_cast<std::size_t>(std::min<std::uint64_t>(entry.size, maximum_bytes));
    return read_range(path_, entry.offset, amount);
}

} // namespace zh::data
