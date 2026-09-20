#pragma once

#include "zh/data/config.h"
#include "zh/foundation/types.h"

#include <filesystem>
#include <string>
#include <vector>

namespace zh::data {

struct BigLimits {
    std::size_t maximum_entries = 1'000'000;
    std::size_t maximum_table_bytes = 64U * 1024U * 1024U;
    std::size_t maximum_name_bytes = 4096;
    std::uint64_t maximum_archive_bytes = 8ULL * 1024ULL * 1024ULL * 1024ULL;
    std::uint64_t maximum_entry_bytes = 2ULL * 1024ULL * 1024ULL * 1024ULL;
};

struct BigEntry {
    std::string logical_name;
    std::string normalized_name;
    std::uint64_t offset = 0;
    std::uint64_t size = 0;
};

class BigArchive {
public:
    static BigArchive open(const std::filesystem::path& path, const BigLimits& limits = {});

    const std::filesystem::path& path() const noexcept { return path_; }
    const std::vector<BigEntry>& entries() const noexcept { return entries_; }
    std::vector<foundation::UInt8> read_prefix(const BigEntry& entry, std::size_t maximum_bytes) const;

private:
    std::filesystem::path path_;
    std::vector<BigEntry> entries_;
};

} // namespace zh::data
