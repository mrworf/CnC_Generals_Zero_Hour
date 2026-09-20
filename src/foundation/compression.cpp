#include "zh/foundation/compression.h"

#include "zh/foundation/byte_codec.h"

#include <algorithm>
#include <array>
#include <cstring>
#include <limits>
#include <zlib.h>

namespace zh::foundation {
namespace {

bool tagged(ByteView input, const char (&tag)[5]) noexcept
{
    return input.size >= 4 && input.data != nullptr && std::memcmp(input.data, tag, 4) == 0;
}

[[noreturn]] void fail(CompressionErrorCode code, std::string_view source, const std::string& reason)
{
    throw CompressionError(code, std::string(source), reason);
}

UInt32 read_be_size(ByteView input, std::size_t& cursor, unsigned width, std::string_view source)
{
    if ((width != 3 && width != 4) || cursor > input.size || width > input.size - cursor) {
        fail(CompressionErrorCode::malformed, source, "truncated RefPack size field");
    }
    UInt32 value = 0;
    for (unsigned n = 0; n < width; ++n) value = (value << 8U) | input.data[cursor++];
    return value;
}

std::vector<UInt8> decode_refpack(
    ByteView input,
    std::size_t expected_size,
    std::size_t maximum_output_size,
    std::string_view source)
{
    if (input.data == nullptr || input.size < 5) {
        fail(CompressionErrorCode::malformed, source, "truncated RefPack header");
    }
    std::size_t cursor = 0;
    const UInt16 type = static_cast<UInt16>((static_cast<UInt16>(input.data[cursor]) << 8U) | input.data[cursor + 1]);
    cursor += 2;
    if (type != 0x10fbU && type != 0x11fbU && type != 0x90fbU && type != 0x91fbU) {
        fail(CompressionErrorCode::malformed, source, "invalid RefPack stream type");
    }
    const unsigned size_width = (type & 0x8000U) ? 4U : 3U;
    if ((type & 0x0100U) != 0) {
        const UInt32 declared_compressed = read_be_size(input, cursor, size_width, source);
        if (declared_compressed > input.size) {
            fail(CompressionErrorCode::malformed, source, "RefPack compressed-size field exceeds input");
        }
    }
    const UInt32 internal_size = read_be_size(input, cursor, size_width, source);
    if (internal_size != expected_size) {
        fail(CompressionErrorCode::malformed, source, "RefPack inner and outer sizes disagree");
    }
    if (expected_size > maximum_output_size) {
        fail(CompressionErrorCode::size_limit, source, "RefPack output exceeds configured size limit");
    }

    std::vector<UInt8> output;
    output.reserve(expected_size);
    const auto copy_literals = [&](std::size_t count) {
        if (cursor > input.size || count > input.size - cursor) {
            fail(CompressionErrorCode::malformed, source, "truncated RefPack literal run");
        }
        if (count > expected_size - output.size()) {
            fail(CompressionErrorCode::malformed, source, "RefPack literal run exceeds advertised output");
        }
        output.insert(output.end(), input.data + cursor, input.data + cursor + count);
        cursor += count;
    };
    const auto copy_reference = [&](std::size_t distance, std::size_t count) {
        if (distance == 0 || distance > output.size()) {
            fail(CompressionErrorCode::malformed, source, "RefPack back-reference precedes output");
        }
        if (count > expected_size - output.size()) {
            fail(CompressionErrorCode::malformed, source, "RefPack back-reference exceeds advertised output");
        }
        for (std::size_t n = 0; n < count; ++n) output.push_back(output[output.size() - distance]);
    };

    bool ended = false;
    while (!ended) {
        if (cursor >= input.size) fail(CompressionErrorCode::malformed, source, "RefPack stream has no end command");
        const UInt8 first = input.data[cursor++];
        if ((first & 0x80U) == 0) {
            if (cursor >= input.size) fail(CompressionErrorCode::malformed, source, "truncated RefPack short command");
            const UInt8 second = input.data[cursor++];
            copy_literals(first & 3U);
            const std::size_t distance = 1U + ((static_cast<std::size_t>(first & 0x60U) << 3U) | second);
            copy_reference(distance, ((first & 0x1cU) >> 2U) + 3U);
        } else if ((first & 0x40U) == 0) {
            if (cursor > input.size || 2 > input.size - cursor) fail(CompressionErrorCode::malformed, source, "truncated RefPack medium command");
            const UInt8 second = input.data[cursor++];
            const UInt8 third = input.data[cursor++];
            copy_literals(second >> 6U);
            const std::size_t distance = 1U + ((static_cast<std::size_t>(second & 0x3fU) << 8U) | third);
            copy_reference(distance, (first & 0x3fU) + 4U);
        } else if ((first & 0x20U) == 0) {
            if (cursor > input.size || 3 > input.size - cursor) fail(CompressionErrorCode::malformed, source, "truncated RefPack long command");
            const UInt8 second = input.data[cursor++];
            const UInt8 third = input.data[cursor++];
            const UInt8 fourth = input.data[cursor++];
            copy_literals(first & 3U);
            const std::size_t distance = 1U + ((static_cast<std::size_t>((first & 0x10U) >> 4U) << 16U) |
                (static_cast<std::size_t>(second) << 8U) | third);
            const std::size_t count = (static_cast<std::size_t>((first & 0x0cU) >> 2U) << 8U) + fourth + 5U;
            copy_reference(distance, count);
        } else {
            const std::size_t count = (static_cast<std::size_t>(first & 0x1fU) << 2U) + 4U;
            if (count <= 112U) {
                copy_literals(count);
            } else {
                copy_literals(first & 3U);
                ended = true;
            }
        }
    }
    if (output.size() != expected_size) fail(CompressionErrorCode::malformed, source, "RefPack output size does not match header");
    if (cursor != input.size) fail(CompressionErrorCode::malformed, source, "trailing bytes after RefPack end command");
    return output;
}

std::vector<UInt8> inflate_exact(ByteView payload, std::size_t expected_size, std::string_view source)
{
    if (payload.size > std::numeric_limits<uInt>::max() || expected_size > std::numeric_limits<uInt>::max()) {
        fail(CompressionErrorCode::size_limit, source, "zlib input or output exceeds codec range");
    }
    std::vector<UInt8> output(expected_size == 0 ? 1 : expected_size);
    z_stream stream{};
    stream.next_in = const_cast<Bytef*>(reinterpret_cast<const Bytef*>(payload.data));
    stream.avail_in = static_cast<uInt>(payload.size);
    stream.next_out = reinterpret_cast<Bytef*>(output.data());
    stream.avail_out = static_cast<uInt>(output.size());
    if (inflateInit(&stream) != Z_OK) fail(CompressionErrorCode::codec_failure, source, "zlib initialization failed");
    const int status = inflate(&stream, Z_FINISH);
    inflateEnd(&stream);
    if (status != Z_STREAM_END) fail(CompressionErrorCode::malformed, source, "invalid or truncated zlib stream");
    if (stream.total_out != expected_size) fail(CompressionErrorCode::malformed, source, "zlib output size does not match header");
    if (stream.total_in != payload.size) fail(CompressionErrorCode::malformed, source, "trailing bytes after zlib stream");
    output.resize(expected_size);
    return output;
}

} // namespace

CompressionError::CompressionError(CompressionErrorCode code, std::string source, const std::string& reason)
    : std::runtime_error("compression error in '" + source + "': " + reason), code_(code), source_(std::move(source))
{
}

CompressionKind compression_kind(ByteView input) noexcept
{
    if (tagged(input, "NOX\0")) return CompressionKind::nox_lzh;
    if (input.size >= 4 && input.data != nullptr && input.data[0] == 'Z' && input.data[1] == 'L' &&
        input.data[2] >= '1' && input.data[2] <= '9' && input.data[3] == 0) return CompressionKind::zlib;
    if (tagged(input, "EAR\0")) return CompressionKind::refpack;
    if (tagged(input, "EAB\0")) return CompressionKind::btree;
    if (tagged(input, "EAH\0")) return CompressionKind::huffman;
    return CompressionKind::none;
}

std::vector<UInt8> compress_zlib_tagged(ByteView input, int level, std::size_t maximum_output_size)
{
    constexpr std::string_view source = "zlib encoder";
    if (level < 1 || level > 9) fail(CompressionErrorCode::malformed, source, "compression level must be 1 through 9");
    if (input.size != 0 && input.data == nullptr) fail(CompressionErrorCode::malformed, source, "null input bytes");
    if (input.size > std::numeric_limits<UInt32>::max() || input.size > std::numeric_limits<uLong>::max()) {
        fail(CompressionErrorCode::size_limit, source, "input exceeds tagged zlib format range");
    }
    const uLong bound = compressBound(static_cast<uLong>(input.size));
    if (bound > maximum_output_size || maximum_output_size - static_cast<std::size_t>(bound) < 8U) {
        fail(CompressionErrorCode::size_limit, source, "compressed output exceeds configured size limit");
    }
    std::vector<UInt8> output(8U + static_cast<std::size_t>(bound));
    output[0] = 'Z'; output[1] = 'L'; output[2] = static_cast<UInt8>('0' + level); output[3] = 0;
    const UInt32 size = static_cast<UInt32>(input.size);
    for (unsigned n = 0; n < 4; ++n) output[4 + n] = static_cast<UInt8>(size >> (n * 8U));
    uLongf compressed_size = bound;
    const int status = compress2(output.data() + 8, &compressed_size, input.data, static_cast<uLong>(input.size), level);
    if (status != Z_OK) fail(CompressionErrorCode::codec_failure, source, "zlib compression failed with code " + std::to_string(status));
    output.resize(8U + static_cast<std::size_t>(compressed_size));
    return output;
}

std::vector<UInt8> decompress_tagged(ByteView input, std::size_t maximum_output_size, std::string_view logical_source)
{
    if (input.data == nullptr || input.size < 8) fail(CompressionErrorCode::malformed, logical_source, "truncated compression envelope");
    const CompressionKind kind = compression_kind(input);
    if (kind == CompressionKind::none) fail(CompressionErrorCode::malformed, logical_source, "unknown compression tag");
    if (kind == CompressionKind::nox_lzh) fail(CompressionErrorCode::unsupported, logical_source, "NOX/LZH compression is unsupported");
    if (kind == CompressionKind::btree || kind == CompressionKind::huffman) {
        fail(CompressionErrorCode::unsupported, logical_source, "legacy compression kind is not yet available through this adapter");
    }
    ByteReader header(input);
    header.read_u32_le();
    const UInt32 expected_size = header.read_u32_le();
    if (expected_size > maximum_output_size) fail(CompressionErrorCode::size_limit, logical_source, "advertised output exceeds configured size limit");
    const ByteView payload{input.data + 8, input.size - 8};
    if (kind == CompressionKind::zlib) return inflate_exact(payload, expected_size, logical_source);
    return decode_refpack(payload, expected_size, maximum_output_size, logical_source);
}

} // namespace zh::foundation
