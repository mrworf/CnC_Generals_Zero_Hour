#pragma once

#include "zh/foundation/types.h"

#include <cstddef>
#include <cstdint>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

namespace zh::lan {

constexpr std::uint16_t protocol_version = 1;
constexpr std::uint16_t lobby_port = 8086;
constexpr std::uint16_t game_port_base = 8088;
constexpr std::size_t maximum_datagram_bytes = 1200;
constexpr std::size_t maximum_text_bytes = 128;
constexpr std::size_t maximum_commands = 8;
constexpr std::size_t maximum_command_bytes = 256;

enum class PacketType : std::uint16_t {
    advertise = 1,
    join = 2,
    accept = 3,
    reject = 4,
    command = 5,
    disconnect = 6,
};

struct Packet {
    PacketType type = PacketType::advertise;
    std::uint32_t sequence = 0;
    std::uint32_t tick = 0;
    std::uint32_t peer_id = 0;
    std::uint32_t data_identity = 0;
    std::uint32_t map_identity = 0;
    std::string text;
    std::vector<std::string> commands;

    bool operator==(const Packet& other) const
    {
        return type == other.type && sequence == other.sequence && tick == other.tick && peer_id == other.peer_id &&
            data_identity == other.data_identity && map_identity == other.map_identity && text == other.text &&
            commands == other.commands;
    }
};

class ProtocolError : public std::runtime_error {
public:
    using std::runtime_error::runtime_error;
};

std::string_view packet_type_name(PacketType type) noexcept;
std::vector<foundation::UInt8> encode_packet(const Packet& packet);
Packet decode_packet(foundation::ByteView bytes);

// Valid when compared intervals are less than half of the uint32 clock range.
bool tick_after_or_equal(std::uint32_t value, std::uint32_t reference) noexcept;
std::uint32_t tick_elapsed(std::uint32_t now, std::uint32_t then) noexcept;

} // namespace zh::lan
