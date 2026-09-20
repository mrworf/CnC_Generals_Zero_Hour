#pragma once

#include "zh/foundation/types.h"

#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

namespace zh::foundation {

class CodecError : public std::runtime_error {
public:
    using std::runtime_error::runtime_error;
};

class ByteWriter {
public:
    explicit ByteWriter(std::size_t capacity_limit);

    void write_u8(UInt8 value);
    void write_u16_le(UInt16 value);
    void write_u16_be(UInt16 value);
    void write_u32_le(UInt32 value);
    void write_u32_be(UInt32 value);
    void write_u64_le(UInt64 value);
    void write_f32_le(float value);
    void write_bytes(ByteView bytes);
    void write_utf16le(std::u16string_view value, std::size_t max_code_units);

    const std::vector<UInt8>& bytes() const noexcept { return bytes_; }

private:
    void require(std::size_t count);
    std::vector<UInt8> bytes_;
    std::size_t capacity_limit_;
};

class ByteReader {
public:
    explicit ByteReader(ByteView bytes) : bytes_(bytes) {}

    UInt8 read_u8();
    UInt16 read_u16_le();
    UInt16 read_u16_be();
    UInt32 read_u32_le();
    UInt32 read_u32_be();
    UInt64 read_u64_le();
    float read_f32_le();
    std::vector<UInt8> read_bytes(std::size_t count);
    std::u16string read_utf16le(std::size_t max_code_units);

    std::size_t remaining() const noexcept { return bytes_.size - position_; }
    std::size_t position() const noexcept { return position_; }

private:
    void require(std::size_t count) const;
    ByteView bytes_;
    std::size_t position_ = 0;
};

std::string bytes_to_hex(ByteView bytes);

} // namespace zh::foundation
