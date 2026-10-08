// Diagnostic only: unchanged candidate's published C API; not a game provider.
#include <array>
#include <cstdio>
#include <memory>
extern "C" {
#include "LZHL.h"
}

int main()
{
    std::unique_ptr<void, decltype(&delComp)> encoder(initComp(), delComp);
    std::unique_ptr<void, decltype(&delDecomp)> decoder(initDecomp(), delDecomp);
    std::array<unsigned char, 257> source;
    source.fill(42);
    std::array<unsigned char, 4096> compressed{};
    std::array<unsigned char, 257> decoded;
    decoded.fill(0xa5);
    const auto encodedSize = compress(encoder.get(), source.data(), source.size(), compressed.data());
    if (!encodedSize || encodedSize > compressed.size()) {
        std::puts("generated-roundtrip: INVALID_ENCODE_COUNT");
        return 1;
    }
    const auto decodedSize = decompress(decoder.get(), compressed.data(), encodedSize,
                                        decoded.data(), decoded.size());
    if (decodedSize != source.size() || decoded != source) {
        std::puts("generated-roundtrip: FAIL");
        return 1;
    }
    std::puts("generated-roundtrip: PASS");

    // Empty/truncated source cannot encode a complete EOF-terminated stream.
    // Supply nonnull backing even at zero count, with an untouched destination.
    decoder.reset(initDecomp());
    decoded.fill(0xa5);
    const auto reported = decompress(decoder.get(), compressed.data(), 0,
                                     decoded.data(), decoded.size());
    bool untouched = true;
    for (auto byte : decoded) untouched = untouched && byte == 0xa5;
    if (reported == decoded.size() && untouched) {
        std::puts("rejected-empty-input: FULL_CAPACITY_REPORTED_WITHOUT_PAYLOAD");
        return 2;
    }
    std::puts("rejected-empty-input: OTHER_RESULT_REQUIRES_REVIEW");
    return 3;
}
