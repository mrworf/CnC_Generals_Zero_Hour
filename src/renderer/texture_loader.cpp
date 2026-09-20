#include "zh/renderer/texture_loader.h"

#include <algorithm>
#include <array>
#include <cstddef>
#include <limits>

namespace zh::renderer {
namespace {

constexpr UInt32 max_dimension = 16384;
constexpr UInt64 max_texture_bytes = RendererLimits::maximum_upload_bytes;

TextureLoadResult fail(std::string message) { TextureLoadResult result; result.error = std::move(message); return result; }

bool range(const std::vector<UInt8>& bytes, std::size_t offset, std::size_t count)
{
    return offset <= bytes.size() && count <= bytes.size() - offset;
}

UInt16 le16(const std::vector<UInt8>& bytes, std::size_t offset)
{
    return static_cast<UInt16>(bytes[offset]) | static_cast<UInt16>(bytes[offset + 1]) << 8U;
}

UInt32 le32(const std::vector<UInt8>& bytes, std::size_t offset)
{
    return static_cast<UInt32>(bytes[offset]) | static_cast<UInt32>(bytes[offset + 1]) << 8U
        | static_cast<UInt32>(bytes[offset + 2]) << 16U | static_cast<UInt32>(bytes[offset + 3]) << 24U;
}

bool checked_add(UInt64& total, UInt64 value)
{
    if (value > max_texture_bytes || total > max_texture_bytes - value) return false;
    total += value;
    return true;
}

std::array<UInt8, 4> color565(UInt16 value)
{
    const UInt8 r5 = static_cast<UInt8>((value >> 11U) & 31U);
    const UInt8 g6 = static_cast<UInt8>((value >> 5U) & 63U);
    const UInt8 b5 = static_cast<UInt8>(value & 31U);
    return {static_cast<UInt8>((r5 << 3U) | (r5 >> 2U)), static_cast<UInt8>((g6 << 2U) | (g6 >> 4U)),
        static_cast<UInt8>((b5 << 3U) | (b5 >> 2U)), 255};
}

std::array<std::array<UInt8, 4>, 4> color_table(const UInt8* block, bool bc1)
{
    const UInt16 first_value = static_cast<UInt16>(block[0] | block[1] << 8U);
    const UInt16 second_value = static_cast<UInt16>(block[2] | block[3] << 8U);
    std::array<std::array<UInt8, 4>, 4> colors{};
    colors[0] = color565(first_value); colors[1] = color565(second_value);
    if (bc1 && first_value <= second_value) {
        for (unsigned channel = 0; channel < 3; ++channel)
            colors[2][channel] = static_cast<UInt8>((static_cast<unsigned>(colors[0][channel]) + colors[1][channel]) / 2U);
        colors[2][3] = 255;
        colors[3] = {0, 0, 0, 0};
    } else {
        for (unsigned channel = 0; channel < 3; ++channel) {
            colors[2][channel] = static_cast<UInt8>((2U * colors[0][channel] + colors[1][channel]) / 3U);
            colors[3][channel] = static_cast<UInt8>((colors[0][channel] + 2U * colors[1][channel]) / 3U);
        }
        colors[2][3] = colors[3][3] = 255;
    }
    return colors;
}

void decode_block(const UInt8* block, TextureFormat format, UInt32 width, UInt32 height,
    UInt32 block_x, UInt32 block_y, std::vector<UInt8>& output, std::size_t output_base)
{
    const bool bc1 = format == TextureFormat::bc1;
    const UInt8* color = block + (bc1 ? 0 : 8);
    const auto colors = color_table(color, bc1);
    const UInt32 indices = static_cast<UInt32>(color[4]) | static_cast<UInt32>(color[5]) << 8U
        | static_cast<UInt32>(color[6]) << 16U | static_cast<UInt32>(color[7]) << 24U;
    std::array<UInt8, 16> alpha{};
    alpha.fill(255);
    if (format == TextureFormat::bc2) {
        UInt64 bits = 0;
        for (unsigned i = 0; i < 8; ++i) bits |= static_cast<UInt64>(block[i]) << (8U * i);
        for (unsigned i = 0; i < 16; ++i) alpha[i] = static_cast<UInt8>(((bits >> (4U * i)) & 15U) * 17U);
    } else if (format == TextureFormat::bc3) {
        std::array<UInt8, 8> table{};
        table[0] = block[0]; table[1] = block[1];
        if (table[0] > table[1]) {
            for (unsigned i = 1; i <= 6; ++i) table[i + 1] = static_cast<UInt8>(((7U - i) * table[0] + i * table[1]) / 7U);
        } else {
            for (unsigned i = 1; i <= 4; ++i) table[i + 1] = static_cast<UInt8>(((5U - i) * table[0] + i * table[1]) / 5U);
            table[6] = 0; table[7] = 255;
        }
        UInt64 bits = 0;
        for (unsigned i = 0; i < 6; ++i) bits |= static_cast<UInt64>(block[2 + i]) << (8U * i);
        for (unsigned i = 0; i < 16; ++i) alpha[i] = table[(bits >> (3U * i)) & 7U];
    }
    for (UInt32 y = 0; y < 4; ++y) for (UInt32 x = 0; x < 4; ++x) {
        const UInt32 px = block_x * 4 + x, py = block_y * 4 + y;
        if (px >= width || py >= height) continue;
        const unsigned pixel = y * 4 + x;
        const auto& rgba = colors[(indices >> (2U * pixel)) & 3U];
        const std::size_t destination = output_base + (static_cast<std::size_t>(py) * width + px) * 4U;
        output[destination] = rgba[0]; output[destination + 1] = rgba[1]; output[destination + 2] = rgba[2];
        output[destination + 3] = bc1 ? rgba[3] : alpha[pixel];
    }
}

TextureLoadResult parse_dds(const std::vector<UInt8>& bytes, bool backend_supports_bc)
{
    if (!range(bytes, 0, 128)) return fail("DDS header is truncated");
    if (le32(bytes, 0) != 0x20534444U) return fail("DDS magic is invalid");
    if (le32(bytes, 4) != 124 || le32(bytes, 76) != 32) return fail("DDS fixed header sizes are invalid");
    const UInt32 height = le32(bytes, 12), width = le32(bytes, 16);
    const UInt32 mip_count = std::max<UInt32>(1, le32(bytes, 28));
    if (width == 0 || height == 0 || width > max_dimension || height > max_dimension) return fail("DDS dimensions are zero or exceed the supported limit");
    const UInt32 fourcc = le32(bytes, 84);
    TextureFormat format{};
    bool premultiplied = false;
    switch (fourcc) {
    case 0x31545844U: format = TextureFormat::bc1; break; // DXT1
    case 0x32545844U: format = TextureFormat::bc2; premultiplied = true; break; // DXT2
    case 0x33545844U: format = TextureFormat::bc2; break; // DXT3
    case 0x34545844U: format = TextureFormat::bc3; premultiplied = true; break; // DXT4
    case 0x35545844U: format = TextureFormat::bc3; break; // DXT5
    default: return fail("DDS compression is unsupported (expected DXT1/2/3/4/5)");
    }
    UInt64 compressed_size = 0, rgba_size = 0;
    UInt32 level_width = width, level_height = height;
    const UInt64 block_size = format == TextureFormat::bc1 ? 8 : 16;
    for (UInt32 mip = 0; mip < mip_count; ++mip) {
        const UInt64 blocks = static_cast<UInt64>((level_width + 3U) / 4U) * ((level_height + 3U) / 4U);
        if (!checked_add(compressed_size, blocks * block_size)
            || !checked_add(rgba_size, static_cast<UInt64>(level_width) * level_height * 4U))
            return fail("DDS mip payload exceeds the supported size limit");
        level_width = std::max<UInt32>(1, level_width / 2U); level_height = std::max<UInt32>(1, level_height / 2U);
    }
    if (compressed_size > bytes.size() - 128U) return fail("DDS block payload is truncated");
    TextureLoadResult result;
    result.texture.descriptor.width = width; result.texture.descriptor.height = height;
    result.texture.descriptor.mip_levels = mip_count; result.texture.descriptor.format = format;
    result.texture.premultiplied_alpha = premultiplied;
    if (backend_supports_bc) {
        result.texture.bytes.assign(bytes.begin() + 128, bytes.begin() + 128 + static_cast<std::ptrdiff_t>(compressed_size));
        return result;
    }
    result.texture.descriptor.format = TextureFormat::rgba8;
    result.texture.decoded_bc_fallback = true;
    result.texture.bytes.resize(static_cast<std::size_t>(rgba_size));
    std::size_t source = 128, destination = 0;
    level_width = width; level_height = height;
    for (UInt32 mip = 0; mip < mip_count; ++mip) {
        const UInt32 blocks_x = (level_width + 3U) / 4U, blocks_y = (level_height + 3U) / 4U;
        for (UInt32 y = 0; y < blocks_y; ++y) for (UInt32 x = 0; x < blocks_x; ++x) {
            decode_block(bytes.data() + source, format, level_width, level_height, x, y, result.texture.bytes, destination);
            source += static_cast<std::size_t>(block_size);
        }
        destination += static_cast<std::size_t>(level_width) * level_height * 4U;
        level_width = std::max<UInt32>(1, level_width / 2U); level_height = std::max<UInt32>(1, level_height / 2U);
    }
    return result;
}

TextureLoadResult parse_tga(const std::vector<UInt8>& bytes)
{
    if (!range(bytes, 0, 18)) return fail("TGA header is truncated");
    const UInt8 id_length = bytes[0], color_map = bytes[1], image_type = bytes[2], bits = bytes[16];
    if (color_map != 0 || image_type != 2) return fail("TGA must be uncompressed true color without a color map");
    if (bits != 24 && bits != 32) return fail("TGA pixel depth must be 24 or 32 bits");
    const UInt32 width = le16(bytes, 12), height = le16(bytes, 14);
    if (width == 0 || height == 0 || width > max_dimension || height > max_dimension) return fail("TGA dimensions are zero or exceed the supported limit");
    const UInt64 pixel_count = static_cast<UInt64>(width) * height;
    const UInt64 source_size = pixel_count * (bits / 8U), destination_size = pixel_count * 4U;
    if (source_size > max_texture_bytes || destination_size > max_texture_bytes) return fail("TGA pixel payload exceeds the supported size limit");
    const std::size_t offset = 18U + id_length;
    if (!range(bytes, offset, static_cast<std::size_t>(source_size))) return fail("TGA pixel payload is truncated");
    const bool top_origin = (bytes[17] & 0x20U) != 0;
    TextureLoadResult result;
    result.texture.descriptor.width = width; result.texture.descriptor.height = height;
    result.texture.descriptor.format = TextureFormat::rgba8;
    result.texture.bytes.resize(static_cast<std::size_t>(destination_size));
    const std::size_t stride = bits / 8U;
    for (UInt32 y = 0; y < height; ++y) for (UInt32 x = 0; x < width; ++x) {
        const UInt32 source_y = top_origin ? y : height - 1U - y;
        const std::size_t source = offset + (static_cast<std::size_t>(source_y) * width + x) * stride;
        const std::size_t destination = (static_cast<std::size_t>(y) * width + x) * 4U;
        result.texture.bytes[destination] = bytes[source + 2];
        result.texture.bytes[destination + 1] = bytes[source + 1];
        result.texture.bytes[destination + 2] = bytes[source];
        result.texture.bytes[destination + 3] = bits == 32 ? bytes[source + 3] : 255;
    }
    return result;
}

} // namespace

TextureLoadResult parse_texture(const std::vector<UInt8>& bytes, bool backend_supports_bc)
{
    if (bytes.size() >= 4 && le32(bytes, 0) == 0x20534444U) return parse_dds(bytes, backend_supports_bc);
    return parse_tga(bytes);
}

} // namespace zh::renderer
