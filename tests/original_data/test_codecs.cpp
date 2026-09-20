#include "zh/original_data.h"
#include "zh/original_process.h"

#include <cmath>
#include <cstdint>
#include <cstring>
#include <iostream>
#include <limits>
#include <string_view>

namespace {
int failures = 0;
void check(bool condition, std::string_view message)
{
    if (!condition) { std::cerr << "FAIL: " << message << '\n'; ++failures; }
}
template <typename Function>
void expect_error(Function&& function, std::string_view text)
{
    try { function(); check(false, "expected original-data error"); }
    catch (const zh::original_data::Error& error) {
        check(std::string_view(error.what()).find(text) != std::string_view::npos, "error diagnostic");
    }
}
}

int main()
{
    using namespace zh::original_data;
    static_assert(sizeof(char16_t) == 2);
    const XferRecord original{true, std::numeric_limits<std::int32_t>::min(),
        std::numeric_limits<std::int64_t>::max(), -123.5F, "payload",
        std::u16string{u'A', char16_t{0xd83d}, char16_t{0xde80}, u'Z'}};
    const auto encoded = write_xfer_record(original);
    const auto decoded = read_xfer_record(encoded);
    check(decoded.boolean == original.boolean && decoded.integer == original.integer &&
        decoded.integer64 == original.integer64 && decoded.real == original.real, "fixed-width values round trip");
    check(decoded.ascii == original.ascii && decoded.unicode == original.unicode, "ASCII and UTF-16 round trip");
    check(encoded[0] == 1 && encoded[1] == 0 && encoded[2] == 0 && encoded[3] == 0 && encoded[4] == 0x80,
        "Bool and Int use source-defined widths and little endian");

    auto invalid_bool = encoded; invalid_bool[0] = 2;
    expect_error([&] { read_xfer_record(invalid_bool); }, "Bool");
    auto truncated = encoded; truncated.pop_back();
    expect_error([&] { read_xfer_record(truncated); }, "truncated");
    auto trailing = encoded; trailing.push_back(0);
    expect_error([&] { read_xfer_record(trailing); }, "trailing");
    Limits small; small.maximum_file_bytes = 8;
    expect_error([&] { write_xfer_record(original, small); }, "limit");

    const auto nested = write_data_chunks({{"Inner", 7, encoded}});
    const std::vector<DataChunk> source_chunks{{"Known", 3, nested}, {"Unknown", 1, {1, 2, 3}}};
    const auto chunk_bytes = write_data_chunks(source_chunks);
    const auto chunks = read_data_chunks(chunk_bytes);
    check(chunks.size() == 2 && chunks[0].label == "Known" && chunks[1].label == "Unknown", "known and unknown chunks framed");
    const auto children = read_data_chunks(chunks[0].payload);
    check(children.size() == 1 && children[0].label == "Inner" && children[0].version == 7, "nested chunk parsed");
    check(read_xfer_record(children[0].payload).unicode == original.unicode, "chunk payload consumed by Xfer");
    auto bad_chunk = chunk_bytes; bad_chunk.pop_back();
    expect_error([&] { read_data_chunks(bad_chunk); }, "truncated");
    expect_error([&] { write_data_chunks({{"", 1, {}}}); }, "label");

    const auto baseline = characterize_random_streams(0x12345678U, 8, 0, 0);
    const auto noisy = characterize_random_streams(0x12345678U, 8, 17, 23);
    const auto repeated = characterize_random_streams(0x12345678U, 8, 17, 23);
    check(baseline.initial_logic_crc == noisy.initial_logic_crc &&
        baseline.logic_crc_after_draws == noisy.logic_crc_after_draws &&
        baseline.logic_values == noisy.logic_values, "client/audio activity cannot perturb logic checkpoints");
    check(noisy.logic_values == repeated.logic_values && noisy.client_values == repeated.client_values &&
        noisy.audio_values == repeated.audio_values, "random streams repeat from source seed");
    check(noisy.client_values != noisy.audio_values, "client and audio streams advance independently by draw count");
    check(noisy.initial_logic_crc == 0x933b34acU && noisy.logic_crc_after_draws == 0xc4405f5cU,
        "six-word source seed CRC is characterized");
    expect_error([&] { characterize_random_streams(1, 1000001, 0, 0); }, "draw count");
    const auto raw_before = zh::original_process::live_raw_allocations();
    { const auto ownership_probe = read_xfer_record(encoded); check(ownership_probe.ascii == "payload", "ownership probe decoded"); }
    check(zh::original_process::live_raw_allocations() == raw_before, "codec consumer releases raw allocations");

    check(std::string_view(provider_xfer_identity()) == "OriginalXfer.cpp", "Xfer provider witness");
    check(std::string_view(provider_chunk_identity()) == "OriginalDataChunk.cpp", "chunk provider witness");
    check(std::string_view(provider_random_identity()) == "RandomValue.cpp", "actual random provider witness");
    std::cout << "original-data codecs: ok providers=" << provider_xfer_identity() << ','
              << provider_chunk_identity() << ',' << provider_random_identity()
              << " initial-crc=" << std::hex << noisy.initial_logic_crc
              << " logic-crc=" << noisy.logic_crc_after_draws << std::dec
              << " raw=" << zh::original_process::live_raw_allocations() << '\n';
    return failures == 0 ? 0 : 1;
}
