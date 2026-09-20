/*
** Command & Conquer Generals Zero Hour(tm)
** Copyright 2025 Electronic Arts Inc.
** GPL-3.0-or-later
*/

// M27 extraction provenance:
// FileSystem::openFile/getFileListInDirectory and ArchiveFileSystem's normalized
// directory tree remain authoritative. This native adapter binds those operations
// to the already accepted M4 VFS rather than creating a second mount authority.

#include "zh/original_data.h"

#include "zh/foundation/platform.h"

#include <algorithm>
#include <cctype>

namespace zh::original_data {
namespace {

bool extension_matches(std::string_view path, std::string_view extension)
{
    if (path.size() < extension.size()) return false;
    const auto offset = path.size() - extension.size();
    return std::equal(path.begin() + static_cast<std::ptrdiff_t>(offset), path.end(), extension.begin(), extension.end(),
        [](unsigned char left, unsigned char right) { return std::tolower(left) == std::tolower(right); });
}

} // namespace

LogicalFiles::LogicalFiles(const data::VirtualFileSystem& vfs, Limits limits) : vfs_(vfs), limits_(limits)
{
    if (limits_.maximum_file_bytes == 0 || limits_.maximum_line_bytes == 0 || limits_.maximum_records == 0) {
        throw Error("original file limits must be nonzero");
    }
}

const data::VfsResource& LogicalFiles::describe(std::string_view logical_name) const
{
    const data::VfsResource* resource = nullptr;
    try { resource = vfs_.find(logical_name); }
    catch (const foundation::PlatformError& error) { throw Error("invalid original logical file: " + std::string(error.what())); }
    if (resource == nullptr) throw Error("missing original logical file '" + std::string(logical_name) + "'");
    return *resource;
}

std::vector<std::uint8_t> LogicalFiles::read(std::string_view logical_name) const
{
    const auto& resource = describe(logical_name);
    if (resource.size > limits_.maximum_file_bytes) {
        throw Error("original logical file exceeds configured limit: '" + resource.logical_name + "'");
    }
    return vfs_.read_prefix(resource.normalized_name, limits_.maximum_file_bytes);
}

std::vector<std::string> LogicalFiles::enumerate_directory(
    std::string_view directory, std::string_view extension, bool recursive) const
{
    auto normalized_directory = foundation::normalize_logical_path(directory);
    if (!normalized_directory.empty() && normalized_directory.back() != '/') normalized_directory.push_back('/');
    std::vector<std::string> root_files;
    std::vector<std::string> descendants;
    for (const auto& [normalized, resource] : vfs_.resources()) {
        if (normalized.compare(0, normalized_directory.size(), normalized_directory) != 0) continue;
        const auto relative = normalized.substr(normalized_directory.size());
        if (relative.empty() || !extension_matches(relative, extension)) continue;
        const bool nested = relative.find('/') != std::string::npos;
        if (nested && !recursive) continue;
        (nested ? descendants : root_files).push_back(resource.logical_name);
    }
    const auto insensitive_less = [](const std::string& left, const std::string& right) {
        const auto folded = [](const std::string& value) {
            std::string result(value);
            std::transform(result.begin(), result.end(), result.begin(),
                [](unsigned char character) { return static_cast<char>(std::tolower(character)); });
            return result;
        };
        const auto a = folded(left);
        const auto b = folded(right);
        return a == b ? left < right : a < b;
    };
    std::sort(root_files.begin(), root_files.end(), insensitive_less);
    std::sort(descendants.begin(), descendants.end(), insensitive_less);
    root_files.insert(root_files.end(), descendants.begin(), descendants.end());
    if (root_files.size() > limits_.maximum_records) throw Error("original file enumeration exceeds configured limit");
    return root_files;
}

const char* provider_file_identity() noexcept { return "OriginalFileSystem.cpp"; }

} // namespace zh::original_data
