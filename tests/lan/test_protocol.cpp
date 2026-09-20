#include "zh/lan/protocol.h"

#include <cstdint>
#include <iostream>
#include <limits>
#include <stdexcept>
#include <string>
#include <vector>

namespace {

void check(bool condition, const char* message)
{
    if (!condition) throw std::runtime_error(message);
}

template <typename Function>
void rejects(Function function, std::string_view diagnostic)
{
    try {
        function();
    } catch (const zh::lan::ProtocolError& error) {
        check(std::string_view(error.what()).find(diagnostic) != std::string_view::npos, "wrong protocol diagnostic");
        return;
    }
    throw std::runtime_error("malformed packet was accepted");
}

zh::lan::Packet sample(zh::lan::PacketType type)
{
    zh::lan::Packet packet;
    packet.type = type;
    packet.sequence = 0x12345678U;
    packet.tick = 0xfffffff0U;
    packet.peer_id = 7;
    packet.data_identity = 0xaabbccddU;
    packet.map_identity = 0x10203040U;
    if (type == zh::lan::PacketType::advertise) packet.text = "Zero Hour · LAN";
    if (type == zh::lan::PacketType::reject) packet.text = "map identity mismatch";
    if (type == zh::lan::PacketType::command) packet.commands = {"move:4,8", "select:2"};
    if (type == zh::lan::PacketType::disconnect) packet.text = "clean shutdown";
    return packet;
}

void set_u16(std::vector<std::uint8_t>& bytes, std::size_t offset, std::uint16_t value)
{
    bytes[offset] = static_cast<std::uint8_t>(value);
    bytes[offset + 1] = static_cast<std::uint8_t>(value >> 8U);
}

void set_u32(std::vector<std::uint8_t>& bytes, std::size_t offset, std::uint32_t value)
{
    for (unsigned byte = 0; byte != 4; ++byte) bytes[offset + byte] = static_cast<std::uint8_t>(value >> (byte * 8U));
}

} // namespace

int main()
{
    using namespace zh::lan;
    for (const auto type : {PacketType::advertise, PacketType::join, PacketType::accept,
             PacketType::reject, PacketType::command, PacketType::disconnect}) {
        const auto packet = sample(type);
        const auto bytes = encode_packet(packet);
        check(bytes.size() <= maximum_datagram_bytes, "encoded packet bounded");
        check(decode_packet({bytes.data(), bytes.size()}) == packet, "packet round trip");
    }

    auto maximum = sample(PacketType::command);
    maximum.commands.assign(maximum_commands, std::string(maximum_command_bytes, 'x'));
    rejects([&] { (void)encode_packet(maximum); }, "capacity");
    auto too_many = sample(PacketType::command);
    too_many.commands.resize(maximum_commands + 1, "x");
    rejects([&] { (void)encode_packet(too_many); }, "command count");
    auto bad_utf8 = sample(PacketType::reject);
    bad_utf8.text = "\xc0\x80";
    rejects([&] { (void)encode_packet(bad_utf8); }, "UTF-8");

    const auto good = encode_packet(sample(PacketType::advertise));
    auto malformed = good;
    malformed.resize(8);
    rejects([&] { (void)decode_packet({malformed.data(), malformed.size()}); }, "header");
    malformed = good;
    set_u16(malformed, 4, protocol_version + 1);
    rejects([&] { (void)decode_packet({malformed.data(), malformed.size()}); }, "version");
    malformed = good;
    set_u16(malformed, 6, 99);
    rejects([&] { (void)decode_packet({malformed.data(), malformed.size()}); }, "type");
    malformed = good;
    set_u32(malformed, 8, 1);
    rejects([&] { (void)decode_packet({malformed.data(), malformed.size()}); }, "declared packet length");
    malformed = good;
    malformed.pop_back();
    rejects([&] { (void)decode_packet({malformed.data(), malformed.size()}); }, "declared packet length");
    malformed = good;
    // Header (12) + five fixed u32 fields (20) puts text length at byte 32.
    set_u16(malformed, 32, static_cast<std::uint16_t>(maximum_text_bytes + 1));
    rejects([&] { (void)decode_packet({malformed.data(), malformed.size()}); }, "text length");
    malformed = good;
    // Skip the two-byte length and known advertisement text to reach command count.
    const auto command_count_offset = 34 + sample(PacketType::advertise).text.size();
    set_u16(malformed, command_count_offset, static_cast<std::uint16_t>(maximum_commands + 1));
    rejects([&] { (void)decode_packet({malformed.data(), malformed.size()}); }, "command count");
    malformed = good;
    malformed.push_back(0);
    set_u32(malformed, 8, static_cast<std::uint32_t>(malformed.size() - 12));
    rejects([&] { (void)decode_packet({malformed.data(), malformed.size()}); }, "trailing");
    malformed = good;
    malformed[34] = 0xc0;
    malformed[35] = 0x80;
    rejects([&] { (void)decode_packet({malformed.data(), malformed.size()}); }, "UTF-8");

    check(tick_after_or_equal(3, std::numeric_limits<std::uint32_t>::max() - 2), "wrap-safe ordering");
    check(tick_elapsed(3, std::numeric_limits<std::uint32_t>::max() - 2) == 6, "wrap-safe elapsed");
    check(!tick_after_or_equal(90, 100), "older tick rejected");
    std::cout << "bounded LAN packet codec: ok\n";
    return 0;
}
