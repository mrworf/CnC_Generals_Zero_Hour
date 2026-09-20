/*
** Command & Conquer Generals Zero Hour(tm)
** Copyright 2025 Electronic Arts Inc.
** GPL-3.0-or-later
*/

// M27 extraction provenance: Xfer::xferBool, Xfer::xferInt, Xfer::xferInt64,
// Xfer::xferReal, Xfer::xferAsciiString and Xfer::xferUnicodeString in Xfer.cpp. Explicit
// little-endian operations replace the original host-width implementation.

#include "zh/original_data.h"

#include <cstring>
#include <limits>

namespace zh::original_data {
namespace {

class Writer {
public:
    explicit Writer(const Limits& limits) : limits_(limits) {}
    void u8(std::uint8_t value) { bytes_.push_back(value); bounded(); }
    void u32(std::uint32_t value)
    {
        for (unsigned shift = 0; shift < 32; shift += 8) bytes_.push_back(static_cast<std::uint8_t>(value >> shift));
        bounded();
    }
    void u64(std::uint64_t value)
    {
        for (unsigned shift = 0; shift < 64; shift += 8) bytes_.push_back(static_cast<std::uint8_t>(value >> shift));
        bounded();
    }
    void raw(const void* data, std::size_t size)
    {
        if (size > limits_.maximum_file_bytes - std::min(bytes_.size(), limits_.maximum_file_bytes))
            throw Error("Xfer output exceeds configured limit");
        const auto* begin = static_cast<const std::uint8_t*>(data);
        bytes_.insert(bytes_.end(), begin, begin + size);
    }
    std::vector<std::uint8_t> finish() && { return std::move(bytes_); }
private:
    void bounded() const { if (bytes_.size() > limits_.maximum_file_bytes) throw Error("Xfer output exceeds configured limit"); }
    const Limits& limits_;
    std::vector<std::uint8_t> bytes_;
};

class Reader {
public:
    Reader(const std::vector<std::uint8_t>& bytes, const Limits& limits) : bytes_(bytes), limits_(limits)
    {
        if (bytes.size() > limits.maximum_file_bytes) throw Error("Xfer input exceeds configured limit");
    }
    std::uint8_t u8() { require(1); return bytes_[cursor_++]; }
    std::uint32_t u32()
    {
        require(4); std::uint32_t result = 0;
        for (unsigned shift = 0; shift < 32; shift += 8) result |= static_cast<std::uint32_t>(bytes_[cursor_++]) << shift;
        return result;
    }
    std::uint64_t u64()
    {
        require(8); std::uint64_t result = 0;
        for (unsigned shift = 0; shift < 64; shift += 8) result |= static_cast<std::uint64_t>(bytes_[cursor_++]) << shift;
        return result;
    }
    std::string ascii(std::uint32_t length)
    {
        if (length > limits_.maximum_string_code_units) throw Error("Xfer ASCII string exceeds configured limit");
        require(length);
        std::string result(bytes_.begin() + static_cast<std::ptrdiff_t>(cursor_),
            bytes_.begin() + static_cast<std::ptrdiff_t>(cursor_ + length));
        cursor_ += length; return result;
    }
    std::u16string unicode(std::uint32_t length)
    {
        if (length > limits_.maximum_string_code_units || length > remaining() / 2U)
            throw Error("Xfer Unicode string is truncated or exceeds configured limit");
        std::u16string result; result.reserve(length);
        for (std::uint32_t index = 0; index < length; ++index) result.push_back(static_cast<char16_t>(u32_width16()));
        return result;
    }
    bool empty() const noexcept { return cursor_ == bytes_.size(); }
private:
    std::uint16_t u32_width16()
    {
        require(2); const auto result = static_cast<std::uint16_t>(bytes_[cursor_]) |
            static_cast<std::uint16_t>(bytes_[cursor_ + 1] << 8U); cursor_ += 2; return result;
    }
    std::size_t remaining() const noexcept { return bytes_.size() - cursor_; }
    void require(std::size_t size) { if (size > remaining()) throw Error("truncated Xfer input"); }
    const std::vector<std::uint8_t>& bytes_; const Limits& limits_; std::size_t cursor_ = 0;
};

} // namespace

std::vector<std::uint8_t> write_xfer_record(const XferRecord& record, const Limits& limits)
{
    if (record.ascii.size() > UINT32_MAX || record.unicode.size() > UINT32_MAX)
        throw Error("Xfer string length cannot be represented");
    Writer writer(limits);
    writer.u8(record.boolean ? 1U : 0U);
    writer.u32(static_cast<std::uint32_t>(record.integer));
    writer.u64(static_cast<std::uint64_t>(record.integer64));
    std::uint32_t real = 0; static_assert(sizeof(real) == sizeof(record.real));
    std::memcpy(&real, &record.real, sizeof(real)); writer.u32(real);
    writer.u32(static_cast<std::uint32_t>(record.ascii.size())); writer.raw(record.ascii.data(), record.ascii.size());
    writer.u32(static_cast<std::uint32_t>(record.unicode.size()));
    for (const char16_t unit : record.unicode) {
        const auto value = static_cast<std::uint16_t>(unit);
        writer.u8(static_cast<std::uint8_t>(value)); writer.u8(static_cast<std::uint8_t>(value >> 8U));
    }
    return std::move(writer).finish();
}

XferRecord read_xfer_record(const std::vector<std::uint8_t>& bytes, const Limits& limits)
{
    Reader reader(bytes, limits); XferRecord result;
    const auto boolean = reader.u8();
    if (boolean > 1U) throw Error("invalid Xfer Bool encoding");
    result.boolean = boolean != 0;
    result.integer = static_cast<std::int32_t>(reader.u32());
    result.integer64 = static_cast<std::int64_t>(reader.u64());
    const auto real = reader.u32(); std::memcpy(&result.real, &real, sizeof(real));
    result.ascii = reader.ascii(reader.u32());
    result.unicode = reader.unicode(reader.u32());
    if (!reader.empty()) throw Error("Xfer input has trailing data");
    return result;
}

const char* provider_xfer_identity() noexcept { return "OriginalXfer.cpp"; }

} // namespace zh::original_data
