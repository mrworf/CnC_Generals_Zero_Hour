#include "zh/lan/protocol.h"

#include "zh/foundation/byte_codec.h"
#include "zh/foundation/unicode.h"

#include <limits>

namespace zh::lan {
namespace {

constexpr std::uint32_t packet_magic = 0x4e4c485aU; // "ZHLN" as explicit little-endian bytes.
constexpr std::size_t header_size = 12;

bool valid_type(std::uint16_t raw) noexcept
{
    return raw >= static_cast<std::uint16_t>(PacketType::advertise) &&
        raw <= static_cast<std::uint16_t>(PacketType::disconnect);
}

void validate_utf8(std::string_view value, std::string_view field)
{
    try {
        (void)foundation::utf8_to_utf16(value);
    } catch (const foundation::UnicodeError& error) {
        throw ProtocolError(std::string(field) + " is not valid UTF-8: " + error.what());
    }
}

void write_string(foundation::ByteWriter& writer, std::string_view value, std::size_t limit, std::string_view field)
{
    if (value.size() > limit || value.size() > std::numeric_limits<std::uint16_t>::max()) {
        throw ProtocolError(std::string(field) + " length exceeds protocol limit");
    }
    validate_utf8(value, field);
    writer.write_u16_le(static_cast<std::uint16_t>(value.size()));
    writer.write_bytes({reinterpret_cast<const foundation::UInt8*>(value.data()), value.size()});
}

std::string read_string(foundation::ByteReader& reader, std::size_t limit, std::string_view field)
{
    const auto count = reader.read_u16_le();
    if (count > limit) throw ProtocolError(std::string(field) + " length exceeds protocol limit");
    if (count > reader.remaining()) throw ProtocolError(std::string(field) + " length exceeds remaining packet bytes");
    const auto bytes = reader.read_bytes(count);
    std::string value(reinterpret_cast<const char*>(bytes.data()), bytes.size());
    validate_utf8(value, field);
    return value;
}

void validate_packet_shape(const Packet& packet)
{
    const auto raw_type = static_cast<std::uint16_t>(packet.type);
    if (!valid_type(raw_type)) throw ProtocolError("packet type is unknown");
    if (packet.commands.size() > maximum_commands) throw ProtocolError("command count exceeds protocol limit");
    if (packet.type == PacketType::command && packet.commands.empty()) {
        throw ProtocolError("command packet requires at least one command");
    }
    if (packet.type != PacketType::command && !packet.commands.empty()) {
        throw ProtocolError("command count must be zero for " + std::string(packet_type_name(packet.type)) + " packet");
    }
    if (packet.type != PacketType::reject && packet.type != PacketType::disconnect &&
        packet.type != PacketType::advertise && !packet.text.empty()) {
        throw ProtocolError("text must be empty for " + std::string(packet_type_name(packet.type)) + " packet");
    }
}

} // namespace

std::string_view packet_type_name(PacketType type) noexcept
{
    switch (type) {
    case PacketType::advertise: return "advertise";
    case PacketType::join: return "join";
    case PacketType::accept: return "accept";
    case PacketType::reject: return "reject";
    case PacketType::command: return "command";
    case PacketType::disconnect: return "disconnect";
    }
    return "unknown";
}

std::vector<foundation::UInt8> encode_packet(const Packet& packet)
{
    validate_packet_shape(packet);
    try {
        foundation::ByteWriter body(maximum_datagram_bytes - header_size);
        body.write_u32_le(packet.sequence);
        body.write_u32_le(packet.tick);
        body.write_u32_le(packet.peer_id);
        body.write_u32_le(packet.data_identity);
        body.write_u32_le(packet.map_identity);
        write_string(body, packet.text, maximum_text_bytes, "text");
        body.write_u16_le(static_cast<std::uint16_t>(packet.commands.size()));
        for (const auto& command : packet.commands) {
            write_string(body, command, maximum_command_bytes, "command");
        }

        foundation::ByteWriter output(maximum_datagram_bytes);
        output.write_u32_le(packet_magic);
        output.write_u16_le(protocol_version);
        output.write_u16_le(static_cast<std::uint16_t>(packet.type));
        output.write_u32_le(static_cast<std::uint32_t>(body.bytes().size()));
        output.write_bytes({body.bytes().data(), body.bytes().size()});
        return output.bytes();
    } catch (const foundation::CodecError& error) {
        throw ProtocolError(std::string("packet encoding failed: ") + error.what());
    }
}

Packet decode_packet(foundation::ByteView bytes)
{
    if (bytes.size > maximum_datagram_bytes) throw ProtocolError("packet length exceeds datagram limit");
    if (bytes.size < header_size) throw ProtocolError("packet header is truncated");
    try {
        foundation::ByteReader reader(bytes);
        if (reader.read_u32_le() != packet_magic) throw ProtocolError("packet magic mismatch");
        const auto version = reader.read_u16_le();
        if (version != protocol_version) {
            throw ProtocolError("packet version " + std::to_string(version) + " is unsupported; expected " +
                std::to_string(protocol_version));
        }
        const auto raw_type = reader.read_u16_le();
        if (!valid_type(raw_type)) throw ProtocolError("packet type " + std::to_string(raw_type) + " is unknown");
        const auto body_length = reader.read_u32_le();
        if (body_length != reader.remaining()) {
            throw ProtocolError("declared packet length " + std::to_string(body_length) +
                " does not match remaining bytes " + std::to_string(reader.remaining()));
        }

        Packet packet;
        packet.type = static_cast<PacketType>(raw_type);
        packet.sequence = reader.read_u32_le();
        packet.tick = reader.read_u32_le();
        packet.peer_id = reader.read_u32_le();
        packet.data_identity = reader.read_u32_le();
        packet.map_identity = reader.read_u32_le();
        packet.text = read_string(reader, maximum_text_bytes, "text");
        const auto command_count = reader.read_u16_le();
        if (command_count > maximum_commands) throw ProtocolError("command count exceeds protocol limit");
        packet.commands.reserve(command_count);
        for (std::uint16_t index = 0; index < command_count; ++index) {
            packet.commands.push_back(read_string(reader, maximum_command_bytes, "command"));
        }
        if (reader.remaining() != 0) throw ProtocolError("packet has trailing bytes");
        validate_packet_shape(packet);
        return packet;
    } catch (const foundation::CodecError& error) {
        throw ProtocolError(std::string("packet is truncated: ") + error.what());
    }
}

bool tick_after_or_equal(std::uint32_t value, std::uint32_t reference) noexcept
{
    return static_cast<std::int32_t>(value - reference) >= 0;
}

std::uint32_t tick_elapsed(std::uint32_t now, std::uint32_t then) noexcept
{
    return now - then;
}

} // namespace zh::lan
