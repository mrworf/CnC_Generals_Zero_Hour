#include "zh/data/vfs.h"

#include "zh/foundation/platform.h"

#include <algorithm>
#include <cctype>
#include <fstream>
#include <set>

namespace zh::data {
namespace {

std::string ascii_fold(std::string value)
{
    std::transform(value.begin(), value.end(), value.begin(), [](unsigned char character) {
        return static_cast<char>(std::tolower(character));
    });
    return value;
}

bool archive_path(const std::filesystem::path& path)
{
    return ascii_fold(path.extension().string()) == ".big";
}

std::vector<std::filesystem::path> discover_archives(
    const std::filesystem::path& root,
    const std::optional<std::filesystem::path>& excluded_root = std::nullopt)
{
    std::vector<std::filesystem::path> archives;
    std::error_code error;
    const auto excluded = excluded_root ? std::filesystem::weakly_canonical(*excluded_root, error) : std::filesystem::path{};
    error.clear();
    for (std::filesystem::recursive_directory_iterator iterator(root,
             std::filesystem::directory_options::skip_permission_denied, error), end;
         !error && iterator != end; iterator.increment(error)) {
        if (excluded_root) {
            std::error_code canonical_error;
            const auto current = std::filesystem::weakly_canonical(iterator->path(), canonical_error);
            if (!canonical_error && current == excluded) {
                if (iterator->is_directory(canonical_error)) iterator.disable_recursion_pending();
                continue;
            }
        }
        std::error_code type_error;
        if (iterator->is_regular_file(type_error) && !type_error && archive_path(iterator->path())) {
            archives.push_back(iterator->path());
        }
    }
    if (error) throw DataError("cannot enumerate archives below retail root: " + error.message());
    const auto relative_name = [&](const std::filesystem::path& path) {
        return std::filesystem::relative(path, root).generic_string();
    };
    std::sort(archives.begin(), archives.end(), [&](const auto& left, const auto& right) {
        const auto left_name = relative_name(left); const auto right_name = relative_name(right);
        const auto left_folded = ascii_fold(left_name); const auto right_folded = ascii_fold(right_name);
        return left_folded == right_folded ? left_name < right_name : left_folded < right_folded;
    });
    for (std::size_t index = 1; index < archives.size(); ++index) {
        const auto previous = relative_name(archives[index - 1]); const auto current = relative_name(archives[index]);
        if (previous != current && ascii_fold(previous) == ascii_fold(current)) {
            throw DataError("case-fold archive collision: '" + previous + "' and '" + current + "'");
        }
    }
    return archives;
}

void add_resource(
    std::map<std::string, VfsResource>& resources,
    VfsResource resource)
{
    resources.emplace(resource.normalized_name, std::move(resource));
}

void mount_loose(
    std::map<std::string, VfsResource>& resources,
    const std::filesystem::path& root,
    MountLayer layer,
    const std::optional<std::filesystem::path>& excluded_root)
{
    std::vector<std::pair<std::string, std::filesystem::path>> files;
    std::error_code error;
    const auto excluded = excluded_root ? std::filesystem::weakly_canonical(*excluded_root, error) : std::filesystem::path{};
    error.clear();
    std::filesystem::recursive_directory_iterator iterator(root,
        std::filesystem::directory_options::skip_permission_denied, error), end;
    for (; !error && iterator != end; iterator.increment(error)) {
        if (excluded_root) {
            std::error_code canonical_error;
            const auto current = std::filesystem::weakly_canonical(iterator->path(), canonical_error);
            if (!canonical_error && current == excluded) {
                if (iterator->is_directory(canonical_error)) iterator.disable_recursion_pending();
                continue;
            }
        }
        std::error_code type_error;
        if (!iterator->is_regular_file(type_error) || type_error || archive_path(iterator->path())) continue;
        const auto relative = std::filesystem::relative(iterator->path(), root, type_error);
        if (type_error) throw DataError("cannot make loose retail path relative");
        files.emplace_back(relative.generic_string(), iterator->path());
    }
    if (error) throw DataError("cannot enumerate loose retail files: " + error.message());
    std::sort(files.begin(), files.end(), [](const auto& left, const auto& right) {
        const auto a = ascii_fold(left.first); const auto b = ascii_fold(right.first);
        return a == b ? left.first < right.first : a < b;
    });
    std::map<std::string, std::string> original_names;
    for (const auto& [logical, host] : files) {
        const auto normalized = foundation::normalize_logical_path(logical);
        const auto [known, inserted] = original_names.emplace(normalized, logical);
        if (!inserted && known->second != logical) {
            throw DataError("case-fold loose collision: '" + known->second + "' and '" + logical + "'");
        }
        std::error_code size_error;
        const auto size = std::filesystem::file_size(host, size_error);
        if (size_error) throw DataError("cannot determine loose resource size for '" + logical + "'");
        resources.emplace(normalized, VfsResource{logical, normalized, layer,
            std::string(mount_layer_name(layer)) + ":loose", host, {}, 0, size});
    }
}

void mount_archives(
    std::map<std::string, VfsResource>& resources,
    std::vector<std::string>& order,
    const std::vector<std::filesystem::path>& paths,
    MountLayer layer,
    const BigLimits& limits,
    const std::optional<std::filesystem::path>& label_root = std::nullopt)
{
    for (const auto& path : paths) {
        auto archive = std::make_shared<BigArchive>(BigArchive::open(path, limits));
        auto display_name = path.filename().generic_string();
        if (label_root) {
            const auto relative = path.lexically_relative(*label_root);
            if (!relative.empty() && *relative.begin() != "..") display_name = relative.generic_string();
        }
        const auto label = std::string(mount_layer_name(layer)) + ":" + display_name;
        order.push_back(label);
        for (std::size_t index = 0; index < archive->entries().size(); ++index) {
            const auto& entry = archive->entries()[index];
            add_resource(resources, {entry.logical_name, entry.normalized_name, layer, label, {}, archive, index, entry.size});
        }
    }
}

std::vector<std::filesystem::path> resolve_mod_archives(const std::vector<std::filesystem::path>& mods)
{
    std::vector<std::filesystem::path> result;
    for (const auto& mod : mods) {
        if (!mod.is_absolute()) throw DataError("--mod paths must be absolute");
        std::error_code error;
        if (std::filesystem::is_directory(mod, error) && !error) {
            auto discovered = discover_archives(mod);
            result.insert(result.end(), discovered.begin(), discovered.end());
        } else if (!error && std::filesystem::is_regular_file(mod, error) && !error && archive_path(mod)) {
            result.push_back(mod);
        } else {
            throw DataError("--mod must name a readable BIG file or directory");
        }
    }
    return result;
}

} // namespace

std::string_view mount_layer_name(MountLayer layer) noexcept
{
    switch (layer) {
    case MountLayer::zero_hour_loose: return "zero-hour-loose";
    case MountLayer::mod_archive: return "mod-big";
    case MountLayer::zero_hour_archive: return "zero-hour-big";
    case MountLayer::generals_loose: return "generals-loose";
    case MountLayer::generals_archive: return "generals-big";
    }
    return "unknown";
}

VirtualFileSystem VirtualFileSystem::mount(const DataSelection& selection, const BigLimits& limits)
{
    VirtualFileSystem vfs;
    std::optional<std::filesystem::path> excluded;
    std::error_code error;
    const auto zh_canonical = std::filesystem::weakly_canonical(selection.zero_hour_root, error);
    error.clear();
    const auto generals_canonical = std::filesystem::weakly_canonical(selection.generals_root, error);
    if (!error && generals_canonical != zh_canonical) {
        const auto relative = generals_canonical.lexically_relative(zh_canonical);
        if (!relative.empty() && *relative.begin() != "..") excluded = selection.generals_root;
    }
    mount_loose(vfs.resources_, selection.zero_hour_root, MountLayer::zero_hour_loose, excluded);
    mount_archives(vfs.resources_, vfs.archive_mount_order_, resolve_mod_archives(selection.mods),
        MountLayer::mod_archive, limits);
    mount_archives(vfs.resources_, vfs.archive_mount_order_, discover_archives(selection.zero_hour_root, excluded),
        MountLayer::zero_hour_archive, limits, selection.zero_hour_root);
    if (generals_canonical != zh_canonical) {
        mount_loose(vfs.resources_, selection.generals_root, MountLayer::generals_loose, std::nullopt);
        mount_archives(vfs.resources_, vfs.archive_mount_order_, discover_archives(selection.generals_root),
            MountLayer::generals_archive, limits, selection.generals_root);
    }
    return vfs;
}

const VfsResource* VirtualFileSystem::find(std::string_view logical_name) const
{
    const auto normalized = foundation::normalize_logical_path(logical_name);
    const auto found = resources_.find(normalized);
    return found == resources_.end() ? nullptr : &found->second;
}

std::vector<foundation::UInt8> VirtualFileSystem::read_prefix(
    std::string_view logical_name,
    std::size_t maximum_bytes) const
{
    const auto* resource = find(logical_name);
    if (resource == nullptr) throw DataError("missing logical resource '" + std::string(logical_name) + "'");
    if (resource->archive) {
        return resource->archive->read_prefix(resource->archive->entries()[resource->archive_entry], maximum_bytes);
    }
    std::ifstream stream(resource->host_path, std::ios::binary);
    if (!stream) throw DataError("cannot read loose logical resource '" + resource->logical_name + "'");
    const auto amount = static_cast<std::size_t>(std::min<std::uint64_t>(resource->size, maximum_bytes));
    std::vector<foundation::UInt8> bytes(amount);
    if (amount != 0) {
        stream.read(reinterpret_cast<char*>(bytes.data()), static_cast<std::streamsize>(amount));
        if (!stream || stream.gcount() != static_cast<std::streamsize>(amount)) {
            throw DataError("short read for loose logical resource '" + resource->logical_name + "'");
        }
    }
    return bytes;
}

} // namespace zh::data
