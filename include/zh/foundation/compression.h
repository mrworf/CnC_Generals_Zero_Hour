#pragma once

#include "zh/foundation/types.h"

#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

namespace zh::foundation {

enum class CompressionKind { none, nox_lzh, zlib, refpack, btree, huffman };
enum class CompressionErrorCode { unsupported, malformed, size_limit, codec_failure };

class CompressionError : public std::runtime_error {
public:
    CompressionError(CompressionErrorCode code, std::string source, const std::string& reason);
    CompressionErrorCode code() const noexcept { return code_; }
    const std::string& source() const noexcept { return source_; }

private:
    CompressionErrorCode code_;
    std::string source_;
};

CompressionKind compression_kind(ByteView input) noexcept;
std::vector<UInt8> compress_zlib_tagged(ByteView input, int level, std::size_t maximum_output_size);
std::vector<UInt8> decompress_tagged(
    ByteView input,
    std::size_t maximum_output_size,
    std::string_view logical_source);

// Executes the original EAC RefPack translation units behind the bounded port boundary.
std::vector<UInt8> original_refpack_encode(ByteView input, std::size_t maximum_output_size);
std::vector<UInt8> original_refpack_decode(
    ByteView input,
    std::size_t maximum_output_size,
    std::string_view logical_source);
std::string original_refpack_provider();

} // namespace zh::foundation
