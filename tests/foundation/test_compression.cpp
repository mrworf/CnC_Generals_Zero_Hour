#include "zh/foundation/compression.h"

#include <algorithm>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

using namespace zh::foundation;

namespace {

void check(bool condition, const char* message)
{
    if (!condition) throw std::runtime_error(message);
}

template <typename Function>
CompressionError rejects(Function function, const char* message)
{
    try {
        function();
    } catch (const CompressionError& error) {
        return error;
    }
    throw std::runtime_error(message);
}

std::vector<UInt8> envelope(const char tag[4], UInt32 size, std::initializer_list<UInt8> payload)
{
    std::vector<UInt8> bytes(tag, tag + 4);
    for (unsigned n = 0; n < 4; ++n) bytes.push_back(static_cast<UInt8>(size >> (n * 8U)));
    bytes.insert(bytes.end(), payload);
    return bytes;
}

void test_zlib()
{
    const std::vector<UInt8> payload{0, 1, 2, 3, 0, 0xff, 'Z', 'H'};
    for (int level = 1; level <= 9; ++level) {
        const auto compressed = compress_zlib_tagged({payload.data(), payload.size()}, level, 1024);
        check(compression_kind({compressed.data(), compressed.size()}) == CompressionKind::zlib, "zlib tag classification");
        check(compressed[2] == static_cast<UInt8>('0' + level), "zlib level tag changed");
        check(decompress_tagged({compressed.data(), compressed.size()}, payload.size(), "synthetic/zlib") == payload, "zlib round trip");
    }
    const auto empty = compress_zlib_tagged({nullptr, 0}, 5, 128);
    check(decompress_tagged({empty.data(), empty.size()}, 0, "synthetic/empty").empty(), "empty zlib round trip");
    rejects([] { compress_zlib_tagged({nullptr, 0}, 0, 128); }, "invalid zlib level accepted");
    rejects([&] { compress_zlib_tagged({payload.data(), payload.size()}, 5, 8); }, "zlib allocation limit ignored");

    auto corrupt = compress_zlib_tagged({payload.data(), payload.size()}, 5, 1024);
    corrupt.pop_back();
    rejects([&] { decompress_tagged({corrupt.data(), corrupt.size()}, 1024, "synthetic/truncated"); }, "truncated zlib accepted");
    auto mismatch = compress_zlib_tagged({payload.data(), payload.size()}, 5, 1024);
    mismatch[4] = static_cast<UInt8>(payload.size() + 1);
    rejects([&] { decompress_tagged({mismatch.data(), mismatch.size()}, 1024, "synthetic/mismatch"); }, "zlib size mismatch accepted");
}

void test_refpack()
{
    const auto literal = envelope("EAR", 4, {0x10, 0xfb, 0x00, 0x00, 0x04, 0xe0, 'A', 'B', 'C', 'D', 0xfc});
    check(decompress_tagged({literal.data(), literal.size()}, 4, "synthetic/literal") ==
        std::vector<UInt8>({'A', 'B', 'C', 'D'}), "RefPack literal decode");
    const auto overlap = envelope("EAR", 6, {0x10, 0xfb, 0x00, 0x00, 0x06, 0x09, 0x00, 'A', 0xfc});
    check(decompress_tagged({overlap.data(), overlap.size()}, 6, "synthetic/backref") ==
        std::vector<UInt8>(6, 'A'), "RefPack overlapping back-reference decode");
    const auto bad_reference = envelope("EAR", 3, {0x10, 0xfb, 0x00, 0x00, 0x03, 0x00, 0x00, 0xfc});
    rejects([&] { decompress_tagged({bad_reference.data(), bad_reference.size()}, 3, "synthetic/bad-ref"); }, "invalid RefPack reference accepted");
    auto trailing = literal;
    trailing.push_back(0);
    rejects([&] { decompress_tagged({trailing.data(), trailing.size()}, 4, "synthetic/trailing"); }, "RefPack trailing data accepted");
    rejects([&] { decompress_tagged({literal.data(), literal.size()}, 3, "synthetic/limited"); }, "RefPack output limit ignored");
}

void test_classification_and_errors()
{
    const auto nox = envelope("NOX", 0, {});
    const auto nox_error = rejects([&] { decompress_tagged({nox.data(), nox.size()}, 10, "archive.big:data/file"); }, "NOX accepted");
    check(nox_error.code() == CompressionErrorCode::unsupported, "NOX error is not typed unsupported");
    check(std::string(nox_error.what()).find("archive.big:data/file") != std::string::npos, "NOX error omitted logical source");
    const auto unknown = envelope("BAD", 0, {});
    rejects([&] { decompress_tagged({unknown.data(), unknown.size()}, 10, "synthetic/unknown"); }, "unknown compression accepted");
    const UInt8 short_input[] = {'Z', 'L', '5'};
    rejects([&] { decompress_tagged({short_input, sizeof(short_input)}, 10, "synthetic/short"); }, "short envelope accepted");
}

} // namespace

int main()
{
    try {
        test_zlib();
        test_refpack();
        test_classification_and_errors();
        std::cout << "bounded compression tests: ok\n";
        return 0;
    } catch (const std::exception& error) {
        std::cerr << error.what() << '\n';
        return 1;
    }
}
