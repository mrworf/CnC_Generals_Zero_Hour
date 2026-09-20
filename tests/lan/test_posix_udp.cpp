#include "zh/lan/posix_udp.h"
#include "zh/lan/protocol.h"

#include <iostream>
#include <stdexcept>
#include <string_view>
#include <vector>

namespace {

void check(bool condition, const char* message)
{
    if (!condition) throw std::runtime_error(message);
}

} // namespace

int main()
{
    using namespace zh::lan;
    PosixUdpEndpoint first({"127.0.0.2", game_port_base});
    PosixUdpEndpoint second({"127.0.0.3", game_port_base});
    check(!second.receive(), "nonblocking receive returns empty");

    Packet packet;
    packet.type = PacketType::join;
    packet.peer_id = 42;
    packet.data_identity = 100;
    packet.map_identity = 200;
    const auto bytes = encode_packet(packet);
    first.send_to(second.local_address(), {bytes.data(), bytes.size()});
    check(second.wait_readable(500), "poll reports direct UDP datagram");
    const auto received = second.receive();
    check(received.has_value(), "direct UDP datagram received");
    check(received->source == first.local_address(), "source address retained");
    check(decode_packet({received->bytes.data(), received->bytes.size()}) == packet, "direct UDP packet round trip");

    std::vector<zh::foundation::UInt8> oversized(maximum_datagram_bytes + 1);
    bool oversized_rejected = false;
    try {
        first.send_to(second.local_address(), {oversized.data(), oversized.size()});
    } catch (const TransportError& error) {
        oversized_rejected = std::string_view(error.what()).find("exceeds") != std::string_view::npos;
    }
    check(oversized_rejected, "oversized UDP datagram rejected");

    // Physical loopback-broadcast delivery is host-dependent and is explicitly
    // not a release gate. Record the observed result while direct UDP remains mandatory.
    first.send_to({"127.255.255.255", game_port_base}, {bytes.data(), bytes.size()});
    const bool broadcast_delivered = second.wait_readable(100);
    if (broadcast_delivered) {
        const auto broadcast = second.receive();
        check(broadcast.has_value(), "broadcast readiness has datagram");
        std::cout << "loopback broadcast: delivered\n";
    } else {
        std::cout << "loopback broadcast: unverified (virtual discovery is the accepted fallback)\n";
    }
    std::cout << "POSIX nonblocking UDP direct transport: ok\n";
    return 0;
}
