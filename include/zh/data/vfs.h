#pragma once

#include "zh/data/big_archive.h"

#include <filesystem>
#include <map>
#include <memory>
#include <optional>
#include <string>
#include <vector>

namespace zh::data {

enum class MountLayer {
    zero_hour_loose,
    mod_archive,
    zero_hour_archive,
    generals_loose,
    generals_archive,
};

std::string_view mount_layer_name(MountLayer layer) noexcept;

struct VfsResource {
    std::string logical_name;
    std::string normalized_name;
    MountLayer layer;
    std::string source_label;
    std::filesystem::path host_path;
    std::shared_ptr<const BigArchive> archive;
    std::size_t archive_entry = 0;
    std::uint64_t size = 0;
};

class VirtualFileSystem {
public:
    static VirtualFileSystem mount(const DataSelection& selection, const BigLimits& limits = {});

    const VfsResource* find(std::string_view logical_name) const;
    std::vector<foundation::UInt8> read_prefix(std::string_view logical_name, std::size_t maximum_bytes) const;
    const std::map<std::string, VfsResource>& resources() const noexcept { return resources_; }
    const std::vector<std::string>& archive_mount_order() const noexcept { return archive_mount_order_; }

private:
    std::map<std::string, VfsResource> resources_;
    std::vector<std::string> archive_mount_order_;
};

} // namespace zh::data
