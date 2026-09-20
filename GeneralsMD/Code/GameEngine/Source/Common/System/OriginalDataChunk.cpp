/*
** Command & Conquer Generals Zero Hour(tm)
** Copyright 2025 Electronic Arts Inc.
** GPL-3.0-or-later
*/

// M27 extraction provenance: DataChunkTableOfContents and
// DataChunkInput::openDataChunk/closeDataChunk in DataChunk.cpp. The portable
// representation keeps label/version/size framing and bounded skip semantics.

#include "zh/original_data.h"

#include <set>

namespace zh::original_data {
namespace {
void append_u16(std::vector<std::uint8_t>& out, std::uint16_t value)
{ out.push_back(static_cast<std::uint8_t>(value)); out.push_back(static_cast<std::uint8_t>(value >> 8U)); }
void append_u32(std::vector<std::uint8_t>& out, std::uint32_t value)
{ for (unsigned shift = 0; shift < 32; shift += 8) out.push_back(static_cast<std::uint8_t>(value >> shift)); }
std::uint16_t read_u16(const std::vector<std::uint8_t>& bytes, std::size_t& cursor)
{
    if (bytes.size() - cursor < 2) throw Error("truncated data chunk header");
    const auto result = static_cast<std::uint16_t>(bytes[cursor]) | static_cast<std::uint16_t>(bytes[cursor + 1] << 8U);
    cursor += 2; return result;
}
std::uint32_t read_u32(const std::vector<std::uint8_t>& bytes, std::size_t& cursor)
{
    if (bytes.size() - cursor < 4) throw Error("truncated data chunk header");
    std::uint32_t result = 0; for (unsigned shift = 0; shift < 32; shift += 8) result |= static_cast<std::uint32_t>(bytes[cursor++]) << shift;
    return result;
}
} // namespace

std::vector<std::uint8_t> write_data_chunks(const std::vector<DataChunk>& chunks, const Limits& limits)
{
    if (chunks.size() > limits.maximum_records) throw Error("data chunk count exceeds configured limit");
    std::vector<std::uint8_t> result;
    for (const auto& chunk : chunks) {
        if (chunk.label.empty() || chunk.label.size() > UINT16_MAX || chunk.payload.size() > UINT32_MAX)
            throw Error("data chunk label or payload cannot be represented");
        if (result.size() > limits.maximum_file_bytes || chunk.payload.size() + chunk.label.size() + 8U >
            limits.maximum_file_bytes - result.size()) throw Error("data chunk output exceeds configured limit");
        append_u16(result, static_cast<std::uint16_t>(chunk.label.size()));
        result.insert(result.end(), chunk.label.begin(), chunk.label.end());
        append_u16(result, chunk.version); append_u32(result, static_cast<std::uint32_t>(chunk.payload.size()));
        result.insert(result.end(), chunk.payload.begin(), chunk.payload.end());
    }
    return result;
}

std::vector<DataChunk> read_data_chunks(const std::vector<std::uint8_t>& bytes, const Limits& limits)
{
    if (bytes.size() > limits.maximum_file_bytes) throw Error("data chunk input exceeds configured limit");
    std::vector<DataChunk> result; std::size_t cursor = 0;
    while (cursor != bytes.size()) {
        const auto label_length = read_u16(bytes, cursor);
        if (label_length == 0 || label_length > bytes.size() - cursor) throw Error("invalid or truncated data chunk label");
        DataChunk chunk; chunk.label.assign(bytes.begin() + static_cast<std::ptrdiff_t>(cursor),
            bytes.begin() + static_cast<std::ptrdiff_t>(cursor + label_length)); cursor += label_length;
        chunk.version = read_u16(bytes, cursor); const auto payload_size = read_u32(bytes, cursor);
        if (payload_size > bytes.size() - cursor) throw Error("truncated data chunk payload");
        chunk.payload.assign(bytes.begin() + static_cast<std::ptrdiff_t>(cursor),
            bytes.begin() + static_cast<std::ptrdiff_t>(cursor + payload_size)); cursor += payload_size;
        result.push_back(std::move(chunk));
        if (result.size() > limits.maximum_records) throw Error("data chunk count exceeds configured limit");
    }
    return result;
}

const char* provider_chunk_identity() noexcept { return "OriginalDataChunk.cpp"; }

} // namespace zh::original_data
