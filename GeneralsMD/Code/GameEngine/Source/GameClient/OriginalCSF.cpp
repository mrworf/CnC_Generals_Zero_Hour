/*
** Command & Conquer Generals Zero Hour(tm)
** Copyright 2025 Electronic Arts Inc.
** GPL-3.0-or-later
*/

// M27 extraction provenance: GameTextManager::getCSFInfo and
// GameTextManager::parseCSF in GameText.cpp. Bounds and explicit LE decoding
// are the Linux portability corrections; source inversion semantics are kept.

#include "zh/original_data.h"

#include <algorithm>

namespace zh::original_data {
namespace {

constexpr std::uint32_t tag(char a, char b, char c, char d) noexcept
{
    return (static_cast<std::uint32_t>(a) << 24U) | (static_cast<std::uint32_t>(b) << 16U) |
        (static_cast<std::uint32_t>(c) << 8U) | static_cast<std::uint32_t>(d);
}

class Reader {
public:
    Reader(const std::vector<std::uint8_t>& bytes, const Limits& limits) : bytes_(bytes), limits_(limits)
    {
        if (bytes.size() > limits.maximum_file_bytes) throw Error("CSF input exceeds configured limit");
    }

    std::uint32_t u32()
    {
        require(4);
        const auto result = static_cast<std::uint32_t>(bytes_[cursor_]) |
            (static_cast<std::uint32_t>(bytes_[cursor_ + 1]) << 8U) |
            (static_cast<std::uint32_t>(bytes_[cursor_ + 2]) << 16U) |
            (static_cast<std::uint32_t>(bytes_[cursor_ + 3]) << 24U);
        cursor_ += 4;
        return result;
    }

    std::string ascii(std::uint32_t length)
    {
        if (length > limits_.maximum_string_code_units) throw Error("CSF ASCII string exceeds configured limit");
        require(length);
        std::string result(bytes_.begin() + static_cast<std::ptrdiff_t>(cursor_),
            bytes_.begin() + static_cast<std::ptrdiff_t>(cursor_ + length));
        cursor_ += length;
        return result;
    }

    std::u16string inverted_utf16(std::uint32_t length)
    {
        if (length > limits_.maximum_string_code_units || length > (remaining() / 2U))
            throw Error("CSF UTF-16 string is truncated or exceeds configured limit");
        std::u16string result;
        result.reserve(length);
        for (std::uint32_t index = 0; index < length; ++index) {
            const auto encoded = static_cast<std::uint16_t>(bytes_[cursor_]) |
                static_cast<std::uint16_t>(bytes_[cursor_ + 1] << 8U);
            cursor_ += 2;
            result.push_back(static_cast<char16_t>(~encoded));
        }
        return result;
    }

    bool empty() const noexcept { return cursor_ == bytes_.size(); }

private:
    std::size_t remaining() const noexcept { return bytes_.size() - cursor_; }
    void require(std::size_t count)
    {
        if (count > remaining()) throw Error("truncated CSF input");
    }
    const std::vector<std::uint8_t>& bytes_;
    const Limits& limits_;
    std::size_t cursor_ = 0;
};

} // namespace

CsfDocument parse_csf(const std::vector<std::uint8_t>& bytes, const Limits& limits)
{
    Reader reader(bytes, limits);
    if (reader.u32() != tag('C', 'S', 'F', ' ')) throw Error("invalid CSF identifier");
    const auto version = reader.u32();
    if (version < 2U || version > 3U) throw Error("unsupported CSF version");
    const auto label_count = reader.u32();
    const auto string_count = reader.u32();
    (void)reader.u32();
    CsfDocument result;
    result.language = static_cast<std::int32_t>(reader.u32());
    if (label_count > limits.maximum_records || string_count > limits.maximum_records)
        throw Error("CSF record count exceeds configured limit");
    std::uint64_t observed_strings = 0;
    for (std::uint32_t label = 0; label < label_count; ++label) {
        if (reader.u32() != tag('L', 'B', 'L', ' ')) throw Error("invalid CSF label record");
        const auto strings = reader.u32();
        const auto label_length = reader.u32();
        CsfEntry entry;
        entry.label = reader.ascii(label_length);
        for (std::uint32_t index = 0; index < strings; ++index) {
            const auto kind = reader.u32();
            if (kind != tag('S', 'T', 'R', ' ') && kind != tag('S', 'T', 'R', 'W'))
                throw Error("invalid CSF string record");
            const auto text = reader.inverted_utf16(reader.u32());
            std::string speech;
            if (kind == tag('S', 'T', 'R', 'W')) speech = reader.ascii(reader.u32());
            if (index == 0) { entry.text = text; entry.speech = std::move(speech); }
            ++observed_strings;
            if (observed_strings > limits.maximum_records) throw Error("CSF string count exceeds configured limit");
        }
        result.entries.push_back(std::move(entry));
    }
    if (observed_strings != string_count) throw Error("CSF declared string count mismatch");
    if (!reader.empty()) throw Error("CSF input has trailing data");
    return result;
}

const char* provider_csf_identity() noexcept { return "OriginalCSF.cpp"; }

} // namespace zh::original_data
