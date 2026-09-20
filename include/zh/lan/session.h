#pragma once

#include "zh/lan/protocol.h"
#include "zh/lan/transport.h"

#include <cstdint>
#include <map>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

namespace zh::lan {

enum class SessionRole { host, joiner };
enum class SessionState { idle, advertising, discovering, joining, connected, rejected, timed_out, disconnected };

struct SessionConfig {
    SessionRole role = SessionRole::joiner;
    std::uint32_t peer_id = 0;
    std::uint32_t data_identity = 0;
    std::uint32_t map_identity = 0;
    EndpointAddress discovery_address{"255.255.255.255", lobby_port};
    std::optional<EndpointAddress> direct_connect;
    std::uint32_t retry_ticks = 10;
    std::uint32_t timeout_ticks = 100;
};

class LanSession {
public:
    LanSession(DatagramEndpoint& endpoint, SessionConfig config);

    void start(std::uint32_t now);
    void pump(std::uint32_t now);
    void send_command(std::string command, std::uint32_t now);
    void disconnect(std::string reason, std::uint32_t now);

    SessionState state() const noexcept { return state_; }
    const std::optional<EndpointAddress>& remote_address() const noexcept { return remote_; }
    const std::vector<std::string>& received_commands() const noexcept { return received_commands_; }
    const std::vector<std::string>& diagnostics() const noexcept { return diagnostics_; }

private:
    void send(Packet packet, const EndpointAddress& destination);
    void send_join(std::uint32_t now);
    void handle(const Datagram& datagram, const Packet& packet, std::uint32_t now);
    void reject(const EndpointAddress& destination, std::string reason, std::uint32_t now);
    bool identities_match(const Packet& packet) const noexcept;
    Packet base_packet(PacketType type, std::uint32_t now) const;

    DatagramEndpoint& endpoint_;
    SessionConfig config_;
    SessionState state_ = SessionState::idle;
    std::optional<EndpointAddress> remote_;
    std::uint32_t started_at_ = 0;
    std::uint32_t last_send_at_ = 0;
    std::uint32_t last_receive_at_ = 0;
    std::uint32_t next_command_sequence_ = 1;
    std::uint32_t expected_command_sequence_ = 1;
    std::map<std::uint32_t, std::vector<std::string>> pending_commands_;
    std::vector<std::string> received_commands_;
    std::vector<std::string> diagnostics_;
};

std::string_view session_state_name(SessionState state) noexcept;

} // namespace zh::lan
