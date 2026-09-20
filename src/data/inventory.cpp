#include "zh/data/inventory.h"

#include <algorithm>
#include <array>
#include <cctype>
#include <fstream>
#include <limits>
#include <set>
#include <sstream>

namespace zh::data {
namespace {

std::uint16_t read_le16(const foundation::UInt8* value) noexcept
{
    return static_cast<std::uint16_t>(value[0]) | (static_cast<std::uint16_t>(value[1]) << 8U);
}

std::uint32_t read_le32(const foundation::UInt8* value) noexcept
{
    return static_cast<std::uint32_t>(value[0]) | (static_cast<std::uint32_t>(value[1]) << 8U) |
        (static_cast<std::uint32_t>(value[2]) << 16U) | (static_cast<std::uint32_t>(value[3]) << 24U);
}

std::string lower(std::string value)
{
    std::transform(value.begin(), value.end(), value.begin(), [](unsigned char character) {
        return static_cast<char>(std::tolower(character));
    });
    return value;
}

std::optional<AssetFormat> infer_format(std::string_view logical_name)
{
    const auto extension = lower(std::filesystem::path(logical_name).extension().string());
    if (extension == ".ini") return AssetFormat::ini;
    if (extension == ".csf") return AssetFormat::csf;
    if (extension == ".w3d") return AssetFormat::w3d;
    if (extension == ".dds") return AssetFormat::dds;
    if (extension == ".tga") return AssetFormat::tga;
    if (extension == ".wav") return AssetFormat::wav;
    if (extension == ".mp3") return AssetFormat::mp3;
    if (extension == ".bik") return AssetFormat::bink;
    if (extension == ".ttf" || extension == ".ttc") return AssetFormat::truetype;
    if (extension == ".otf") return AssetFormat::opentype;
    if (extension == ".refpack") return AssetFormat::refpack;
    if (extension == ".zlib") return AssetFormat::zlib;
    if (extension == ".fx" || extension == ".fxo" || extension == ".vsh" || extension == ".psh" ||
        extension == ".vso" || extension == ".pso") {
        return AssetFormat::wwshade;
    }
    return std::nullopt;
}

void require(bool condition, std::string_view logical_name, std::string_view reason)
{
    if (!condition) throw DataError("corrupt " + std::string(logical_name) + ": " + std::string(reason));
}

bool starts_with(foundation::ByteView bytes, std::string_view signature)
{
    return bytes.size >= signature.size() &&
        std::equal(signature.begin(), signature.end(), reinterpret_cast<const char*>(bytes.data));
}

std::vector<std::string> split_tabs(const std::string& line)
{
    std::vector<std::string> fields;
    std::size_t begin = 0;
    while (true) {
        const auto tab = line.find('\t', begin);
        fields.push_back(line.substr(begin, tab == std::string::npos ? tab : tab - begin));
        if (tab == std::string::npos) return fields;
        begin = tab + 1;
    }
}

} // namespace

std::string_view asset_format_name(AssetFormat format) noexcept
{
    switch (format) {
    case AssetFormat::ini: return "ini";
    case AssetFormat::csf: return "csf";
    case AssetFormat::w3d: return "w3d";
    case AssetFormat::dds: return "dds";
    case AssetFormat::tga: return "tga";
    case AssetFormat::wav: return "wav";
    case AssetFormat::mp3: return "mp3";
    case AssetFormat::bink: return "bink";
    case AssetFormat::truetype: return "truetype";
    case AssetFormat::opentype: return "opentype";
    case AssetFormat::refpack: return "refpack";
    case AssetFormat::zlib: return "zlib";
    case AssetFormat::wwshade: return "wwshade";
    }
    return "unknown";
}

std::optional<AssetFormat> asset_format_for_name(std::string_view name)
{
    constexpr std::array formats{AssetFormat::ini, AssetFormat::csf, AssetFormat::w3d, AssetFormat::dds,
        AssetFormat::tga, AssetFormat::wav, AssetFormat::mp3, AssetFormat::bink, AssetFormat::truetype,
        AssetFormat::opentype, AssetFormat::refpack, AssetFormat::zlib, AssetFormat::wwshade};
    const auto found = std::find_if(formats.begin(), formats.end(),
        [&](AssetFormat format) { return asset_format_name(format) == name; });
    return found == formats.end() ? std::nullopt : std::optional<AssetFormat>(*found);
}

CorpusManifest load_corpus_manifest(const std::filesystem::path& path)
{
    std::ifstream stream(path);
    if (!stream) throw DataError("cannot read project corpus manifest");
    const std::string text{std::istreambuf_iterator<char>(stream), std::istreambuf_iterator<char>()};
    if (stream.bad()) throw DataError("failed reading project corpus manifest");
    return parse_corpus_manifest(text);
}

CorpusManifest parse_corpus_manifest(std::string_view text)
{
    std::istringstream stream{std::string(text)};
    CorpusManifest manifest;
    std::set<std::string> identities;
    std::string line;
    std::size_t line_number = 0;
    while (std::getline(stream, line)) {
        ++line_number;
        if (line.empty() || line.front() == '#') continue;
        const auto fields = split_tabs(line);
        if (fields.size() == 2 && fields[0] == "locale") {
            if (!manifest.locale.empty()) throw DataError("duplicate locale in corpus manifest");
            manifest.locale = fields[1];
            continue;
        }
        if (fields.size() != 3) throw DataError("invalid corpus manifest line " + std::to_string(line_number));
        if (!fields[0].empty() && fields[0].front() == '@') {
            manifest.expectations.push_back(fields[0].substr(1) + "=" + fields[1] + " (" + fields[2] + ")");
            continue;
        }
        const auto format = asset_format_for_name(fields[1]);
        if (!format) throw DataError("unsupported format '" + fields[1] + "' in corpus manifest");
        std::string normalized;
        try { normalized = foundation::normalize_logical_path(fields[0]); }
        catch (const foundation::PlatformError& error) {
            throw DataError("invalid corpus manifest logical name: " + std::string(error.what()));
        }
        if (!identities.insert(normalized).second) throw DataError("duplicate logical name in corpus manifest: " + fields[0]);
        manifest.required.push_back({fields[0], *format, fields[2]});
    }
    if (manifest.locale.empty()) throw DataError("corpus manifest has no locale");
    if (manifest.required.empty()) throw DataError("corpus manifest has no required logical assets");
    return manifest;
}

void validate_asset_metadata(
    AssetFormat format,
    std::string_view logical_name,
    foundation::ByteView bytes,
    std::uint64_t total_size)
{
    require(bytes.size <= total_size, logical_name, "metadata prefix exceeds resource size");
    switch (format) {
    case AssetFormat::ini:
        require(std::find(bytes.data, bytes.data + bytes.size, foundation::UInt8{0}) == bytes.data + bytes.size,
            logical_name, "INI prefix contains NUL");
        break;
    case AssetFormat::csf:
        require(bytes.size >= 4 && (starts_with(bytes, " FSC") || starts_with(bytes, "CSF ")),
            logical_name, "invalid CSF signature");
        break;
    case AssetFormat::w3d: {
        require(bytes.size >= 8, logical_name, "truncated W3D chunk header");
        const auto chunk_size = static_cast<std::uint64_t>(read_le32(bytes.data + 4) & 0x7fffffffU);
        require(chunk_size <= total_size - 8, logical_name, "W3D chunk exceeds resource");
        break;
    }
    case AssetFormat::dds:
        require(bytes.size >= 128 && starts_with(bytes, "DDS ") && read_le32(bytes.data + 4) == 124,
            logical_name, "invalid DDS header");
        require(read_le32(bytes.data + 12) != 0 && read_le32(bytes.data + 16) != 0 &&
                read_le32(bytes.data + 12) <= 32768 && read_le32(bytes.data + 16) <= 32768,
            logical_name, "invalid DDS dimensions");
        break;
    case AssetFormat::tga: {
        require(bytes.size >= 18, logical_name, "truncated TGA header");
        const auto image_type = bytes.data[2];
        require(image_type == 1 || image_type == 2 || image_type == 3 || image_type == 9 ||
                image_type == 10 || image_type == 11,
            logical_name, "unsupported TGA image type");
        const auto width = read_le16(bytes.data + 12); const auto height = read_le16(bytes.data + 14);
        require(width != 0 && height != 0 && width <= 16384 && height <= 16384,
            logical_name, "invalid TGA dimensions");
        break;
    }
    case AssetFormat::wav:
        require(bytes.size >= 12 && starts_with(bytes, "RIFF") &&
                std::equal(bytes.data + 8, bytes.data + 12, reinterpret_cast<const foundation::UInt8*>("WAVE")),
            logical_name, "invalid WAV container");
        break;
    case AssetFormat::mp3:
        require(bytes.size >= 3 && (starts_with(bytes, "ID3") ||
                (bytes.data[0] == 0xff && (bytes.data[1] & 0xe0U) == 0xe0U)),
            logical_name, "invalid MP3 frame/tag signature");
        break;
    case AssetFormat::bink:
        require(bytes.size >= 8 && bytes.data[0] == 'B' && bytes.data[1] == 'I' && bytes.data[2] == 'K',
            logical_name, "invalid Bink signature");
        break;
    case AssetFormat::truetype:
        require(bytes.size >= 4 && ((bytes.data[0] == 0 && bytes.data[1] == 1 && bytes.data[2] == 0 && bytes.data[3] == 0) ||
                starts_with(bytes, "true") || starts_with(bytes, "ttcf")),
            logical_name, "invalid TrueType signature");
        break;
    case AssetFormat::opentype:
        require(bytes.size >= 4 && starts_with(bytes, "OTTO"), logical_name, "invalid OpenType signature");
        break;
    case AssetFormat::refpack:
        require(bytes.size >= 2 && bytes.data[1] == 0xfb &&
                (bytes.data[0] == 0x10 || bytes.data[0] == 0x80 || bytes.data[0] == 0x90),
            logical_name, "invalid RefPack signature");
        break;
    case AssetFormat::zlib:
        require(bytes.size >= 2 && (bytes.data[0] & 0x0fU) == 8 &&
                ((static_cast<unsigned>(bytes.data[0]) << 8U) + bytes.data[1]) % 31U == 0,
            logical_name, "invalid zlib header");
        break;
    case AssetFormat::wwshade:
        require(total_size != 0, logical_name, "empty WWShade effect/shader metadata");
        break;
    }
}

void verify_corpus(
    const VirtualFileSystem& vfs,
    const CorpusManifest& manifest,
    std::string_view selected_language,
    std::ostream& output)
{
    if (lower(manifest.locale) != lower(std::string(selected_language))) {
        throw DataError("corpus manifest locale '" + manifest.locale + "' does not match selected locale '" +
            std::string(selected_language) + "'");
    }
    std::map<AssetFormat, std::size_t> counts;
    std::map<AssetFormat, std::string> samples;
    std::size_t nox_entries = 0;
    std::size_t granny_entries = 0;
    for (const auto& [identity, resource] : vfs.resources()) {
        (void)identity;
        const auto extension = lower(std::filesystem::path(resource.logical_name).extension().string());
        if (extension == ".nox") ++nox_entries;
        if (extension == ".gr2" || extension == ".granny") ++granny_entries;
        const auto format = infer_format(resource.logical_name);
        if (!format) continue;
        const auto bytes = vfs.read_prefix(resource.logical_name, 512);
        validate_asset_metadata(*format, resource.logical_name, {bytes.data(), bytes.size()}, resource.size);
        ++counts[*format];
        samples.emplace(*format, resource.logical_name);
    }
    for (const auto& requirement : manifest.required) {
        const auto* resource = vfs.find(requirement.logical_name);
        if (resource == nullptr) throw DataError("missing required logical asset '" + requirement.logical_name + "'");
        const auto bytes = vfs.read_prefix(requirement.logical_name, 512);
        validate_asset_metadata(requirement.format, requirement.logical_name, {bytes.data(), bytes.size()}, resource->size);
        output << "verification: required=" << requirement.logical_name << " format="
               << asset_format_name(requirement.format) << " expectation=" << requirement.expectation << '\n';
    }
    for (const auto& expectation : manifest.expectations) output << "verification: expectation=" << expectation << '\n';
    constexpr std::array supported{AssetFormat::ini, AssetFormat::csf, AssetFormat::w3d, AssetFormat::dds,
        AssetFormat::tga, AssetFormat::wav, AssetFormat::mp3, AssetFormat::bink, AssetFormat::truetype,
        AssetFormat::opentype, AssetFormat::refpack, AssetFormat::zlib, AssetFormat::wwshade};
    for (const auto format : supported) {
        output << "verification: format=" << asset_format_name(format) << " count=" << counts[format];
        const auto sample = samples.find(format);
        if (sample != samples.end()) output << " sample=" << sample->second;
        output << '\n';
    }
    output << "verification: unsupported-probes nox=" << nox_entries << " granny=" << granny_entries << '\n';
    if (nox_entries != 0) throw DataError("unsupported NOX/LZH content is present; classify its consuming logical asset");
    output << "verification: corpus verification ok\n";
}

} // namespace zh::data
