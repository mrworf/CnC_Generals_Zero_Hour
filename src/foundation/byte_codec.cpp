#include "zh/foundation/byte_codec.h"

#include <cstring>

namespace zh::foundation {

ByteWriter::ByteWriter(std::size_t capacity_limit) : capacity_limit_(capacity_limit)
{
}

void ByteWriter::require(std::size_t count)
{
    if (count > capacity_limit_ - bytes_.size()) {
        throw CodecError("byte writer capacity exceeded");
    }
}

void ByteWriter::write_u8(UInt8 value)
{
    require(1);
    bytes_.push_back(value);
}

void ByteWriter::write_u16_le(UInt16 value)
{
    require(2);
    bytes_.push_back(static_cast<UInt8>(value));
    bytes_.push_back(static_cast<UInt8>(value >> 8U));
}

void ByteWriter::write_u16_be(UInt16 value)
{
    require(2);
    bytes_.push_back(static_cast<UInt8>(value >> 8U));
    bytes_.push_back(static_cast<UInt8>(value));
}

void ByteWriter::write_u32_le(UInt32 value)
{
    require(4);
    for (unsigned shift = 0; shift < 32; shift += 8) {
        bytes_.push_back(static_cast<UInt8>(value >> shift));
    }
}

void ByteWriter::write_u32_be(UInt32 value)
{
    require(4);
    for (int shift = 24; shift >= 0; shift -= 8) {
        bytes_.push_back(static_cast<UInt8>(value >> static_cast<unsigned>(shift)));
    }
}

void ByteWriter::write_u64_le(UInt64 value)
{
    require(8);
    for (unsigned shift = 0; shift < 64; shift += 8) {
        bytes_.push_back(static_cast<UInt8>(value >> shift));
    }
}

void ByteWriter::write_f32_le(float value)
{
    UInt32 bits = 0;
    std::memcpy(&bits, &value, sizeof(bits));
    write_u32_le(bits);
}

void ByteWriter::write_bytes(ByteView bytes)
{
    require(bytes.size);
    if (bytes.size == 0) return;
    if (bytes.size != 0 && bytes.data == nullptr) {
        throw CodecError("null byte source");
    }
    bytes_.insert(bytes_.end(), bytes.data, bytes.data + bytes.size);
}

void ByteWriter::write_utf16le(std::u16string_view value, std::size_t max_code_units)
{
    if (value.size() > max_code_units || value.size() > std::numeric_limits<UInt32>::max()) {
        throw CodecError("UTF-16 string exceeds code-unit limit");
    }
    const std::size_t required = 4 + value.size() * 2;
    require(required);
    write_u32_le(static_cast<UInt32>(value.size()));
    for (WideChar unit : value) {
        write_u16_le(static_cast<UInt16>(unit));
    }
}

void ByteReader::require(std::size_t count) const
{
    if (count > remaining()) {
        throw CodecError("truncated byte stream at offset " + std::to_string(position_));
    }
    if (count != 0 && bytes_.data == nullptr) {
        throw CodecError("null byte stream");
    }
}

UInt8 ByteReader::read_u8()
{
    require(1);
    return bytes_.data[position_++];
}

UInt16 ByteReader::read_u16_le()
{
    require(2);
    const UInt16 value = static_cast<UInt16>(bytes_.data[position_]) |
        static_cast<UInt16>(static_cast<UInt16>(bytes_.data[position_ + 1]) << 8U);
    position_ += 2;
    return value;
}

UInt16 ByteReader::read_u16_be()
{
    require(2);
    const UInt16 value = static_cast<UInt16>(static_cast<UInt16>(bytes_.data[position_]) << 8U) |
        bytes_.data[position_ + 1];
    position_ += 2;
    return value;
}

UInt32 ByteReader::read_u32_le()
{
    require(4);
    UInt32 value = 0;
    for (unsigned n = 0; n < 4; ++n) {
        value |= static_cast<UInt32>(bytes_.data[position_ + n]) << (n * 8U);
    }
    position_ += 4;
    return value;
}

UInt32 ByteReader::read_u32_be()
{
    require(4);
    UInt32 value = 0;
    for (unsigned n = 0; n < 4; ++n) {
        value = (value << 8U) | bytes_.data[position_ + n];
    }
    position_ += 4;
    return value;
}

UInt64 ByteReader::read_u64_le()
{
    require(8);
    UInt64 value = 0;
    for (unsigned n = 0; n < 8; ++n) {
        value |= static_cast<UInt64>(bytes_.data[position_ + n]) << (n * 8U);
    }
    position_ += 8;
    return value;
}

float ByteReader::read_f32_le()
{
    const UInt32 bits = read_u32_le();
    float value = 0.0F;
    std::memcpy(&value, &bits, sizeof(value));
    return value;
}

std::vector<UInt8> ByteReader::read_bytes(std::size_t count)
{
    require(count);
    if (count == 0) return {};
    std::vector<UInt8> value(bytes_.data + position_, bytes_.data + position_ + count);
    position_ += count;
    return value;
}

std::u16string ByteReader::read_utf16le(std::size_t max_code_units)
{
    const UInt32 count = read_u32_le();
    if (count > max_code_units || static_cast<std::size_t>(count) > remaining() / 2) {
        throw CodecError("invalid UTF-16 code-unit count");
    }
    std::u16string value;
    value.reserve(count);
    for (UInt32 n = 0; n < count; ++n) {
        value.push_back(static_cast<WideChar>(read_u16_le()));
    }
    return value;
}

std::string bytes_to_hex(ByteView bytes)
{
    static constexpr char digits[] = "0123456789abcdef";
    std::string output;
    output.reserve(bytes.size * 2);
    for (std::size_t i = 0; i < bytes.size; ++i) {
        output.push_back(digits[bytes.data[i] >> 4U]);
        output.push_back(digits[bytes.data[i] & 0x0fU]);
    }
    return output;
}

} // namespace zh::foundation
