#pragma once

#include "zh/data/vfs.h"

#include <filesystem>
#include <iosfwd>
#include <map>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

namespace zh::data {

enum class AssetFormat {
    ini,
    csf,
    w3d,
    dds,
    tga,
    wav,
    mp3,
    bink,
    truetype,
    opentype,
    refpack,
    zlib,
    wwshade,
};

std::string_view asset_format_name(AssetFormat format) noexcept;
std::optional<AssetFormat> asset_format_for_name(std::string_view name);

struct CorpusRequirement {
    std::string logical_name;
    AssetFormat format;
    std::string expectation;
};

struct CorpusManifest {
    std::string locale;
    std::vector<std::string> expectations;
    std::vector<CorpusRequirement> required;
};

CorpusManifest load_corpus_manifest(const std::filesystem::path& path);
CorpusManifest parse_corpus_manifest(std::string_view text);
void validate_asset_metadata(
    AssetFormat format,
    std::string_view logical_name,
    foundation::ByteView prefix,
    std::uint64_t total_size);
void verify_corpus(
    const VirtualFileSystem& vfs,
    const CorpusManifest& manifest,
    std::string_view selected_language,
    std::ostream& output);

} // namespace zh::data
