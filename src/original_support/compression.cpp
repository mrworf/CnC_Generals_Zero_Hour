#include "zh/foundation/compression.h"

extern "C" {
#include "codex.h"
#include "refcodex.h"
}

#include <cstdlib>
#include <limits>

namespace zh::foundation {
namespace {

[[noreturn]] void original_fail(CompressionErrorCode code, std::string_view source, const char* reason)
{
    throw CompressionError(code, std::string(source), reason);
}

std::vector<UInt8> bounded_reference_decode(ByteView input, std::size_t maximum_output_size, std::string_view source)
{
    if (input.data == nullptr || input.size < 6) original_fail(CompressionErrorCode::malformed, source, "truncated original RefPack stream");
    if (!REF_is(input.data)) original_fail(CompressionErrorCode::malformed, source, "invalid original RefPack signature");
    const int declared = REF_size(input.data);
    if (declared < 0 || static_cast<std::size_t>(declared) > maximum_output_size) {
        original_fail(CompressionErrorCode::size_limit, source, "original RefPack output exceeds configured size limit");
    }

    std::vector<UInt8> envelope{'E', 'A', 'R', 0};
    const auto expected = static_cast<UInt32>(declared);
    for (unsigned index = 0; index < 4; ++index) envelope.push_back(static_cast<UInt8>(expected >> (index * 8U)));
    envelope.insert(envelope.end(), input.data, input.data + input.size);
    const auto checked = decompress_tagged({envelope.data(), envelope.size()}, maximum_output_size, source);

    std::vector<UInt8> decoded(checked.size());
    int consumed = 0;
    const int produced = REF_decode(decoded.data(), input.data, &consumed);
    if (produced != declared || consumed < 0 || static_cast<std::size_t>(consumed) != input.size || decoded != checked) {
        original_fail(CompressionErrorCode::codec_failure, source, "original RefPack decoder disagrees with bounded validation");
    }
    return decoded;
}

} // namespace

std::vector<UInt8> original_refpack_encode(ByteView input, std::size_t maximum_output_size)
{
    constexpr std::string_view source = "original EAC RefPack encoder";
    if (input.size != 0 && input.data == nullptr) original_fail(CompressionErrorCode::malformed, source, "null input bytes");
    if (input.size > 0x00ffffffU || input.size > static_cast<std::size_t>(std::numeric_limits<int>::max())) {
        original_fail(CompressionErrorCode::size_limit, source, "input exceeds the characterized original RefPack range");
    }
    const std::size_t bound = input.size + input.size / 4U + 64U;
    if (bound > maximum_output_size) original_fail(CompressionErrorCode::size_limit, source, "encoded output exceeds configured size limit");
    std::vector<UInt8> encoded(bound);
    const int size = REF_encode(encoded.data(), input.data, static_cast<int>(input.size), nullptr);
    if (size <= 0 || static_cast<std::size_t>(size) > encoded.size()) {
        original_fail(CompressionErrorCode::codec_failure, source, "original RefPack encoder failed");
    }
    encoded.resize(static_cast<std::size_t>(size));
    return encoded;
}

std::vector<UInt8> original_refpack_decode(ByteView input, std::size_t maximum_output_size, std::string_view logical_source)
{
    return bounded_reference_decode(input, maximum_output_size, logical_source);
}

std::string original_refpack_provider()
{
    CODEXABOUT* about = REF_about();
    if (about == nullptr) original_fail(CompressionErrorCode::codec_failure, "original EAC RefPack", "provider metadata allocation failed");
    const std::string provider = std::string("GeneralsMD EAC ") + about->longtypestr + " " + about->versionstr;
    std::free(about);
    return provider;
}

} // namespace zh::foundation

const char* zh_compression_bootstrap_component() noexcept
{
    return "zh_compression:original-eac-refpack";
}
