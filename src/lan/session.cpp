#include "zh/lan/session.h"

#include <utility>

namespace zh::lan {
namespace {

constexpr std::size_t maximum_pending_command_packets = 8;
constexpr std::size_t maximum_receive_batch = 64;

bool active(SessionState state) noexcept
{
    return state == SessionState::advertising || state == SessionState::discovering ||
        state == SessionState::joining || state == SessionState::connected;
}

} // namespace

std::string_view session_state_name(SessionState state) noexcept
{
    switch (state) {
    case SessionState::idle: return "idle";
    case SessionState::advertising: return "advertising";
    case SessionState::discovering: return "discovering";
    case SessionState::joining: return "joining";
    case SessionState::connected: return "connected";
    case SessionState::rejected: return "rejected";
    case SessionState::timed_out: return "timed-out";
    case SessionState::disconnected: return "disconnected";
    }
    return "unknown";
}

LanSession::LanSession(DatagramEndpoint& endpoint, SessionConfig config)
    : endpoint_(endpoint), config_(std::move(config))
{
    if (config_.peer_id == 0) throw TransportError("LAN peer id must be nonzero");
    if (config_.retry_ticks == 0 || config_.timeout_ticks <= config_.retry_ticks) {
        throw TransportError("LAN timeout must be greater than a nonzero retry interval");
    }
}

void LanSession::start(std::uint32_t now)
{
    if (state_ != SessionState::idle && state_ != SessionState::disconnected &&
        state_ != SessionState::rejected && state_ != SessionState::timed_out) {
        throw TransportError("LAN session is already active");
    }
    remote_.reset();
    pending_commands_.clear();
    received_commands_.clear();
    diagnostics_.clear();
    next_command_sequence_ = 1;
    expected_command_sequence_ = 1;
    started_at_ = last_send_at_ = last_receive_at_ = now;
    if (config_.role == SessionRole::host) {
        state_ = SessionState::advertising;
        auto advertisement = base_packet(PacketType::advertise, now);
        advertisement.text = "Zero Hour LAN host";
        send(std::move(advertisement), config_.discovery_address);
    } else if (config_.direct_connect) {
        remote_ = config_.direct_connect;
        state_ = SessionState::joining;
        send_join(now);
    } else {
        state_ = SessionState::discovering;
    }
}

Packet LanSession::base_packet(PacketType type, std::uint32_t now) const
{
    Packet packet;
    packet.type = type;
    packet.tick = now;
    packet.peer_id = config_.peer_id;
    packet.data_identity = config_.data_identity;
    packet.map_identity = config_.map_identity;
    return packet;
}

void LanSession::send(Packet packet, const EndpointAddress& destination)
{
    const auto bytes = encode_packet(packet);
    endpoint_.send_to(destination, {bytes.data(), bytes.size()});
}

void LanSession::send_join(std::uint32_t now)
{
    if (!remote_) throw TransportError("join has no remote endpoint");
    send(base_packet(PacketType::join, now), *remote_);
    last_send_at_ = now;
}

bool LanSession::identities_match(const Packet& packet) const noexcept
{
    return packet.data_identity == config_.data_identity && packet.map_identity == config_.map_identity;
}

void LanSession::reject(const EndpointAddress& destination, std::string reason, std::uint32_t now)
{
    auto packet = base_packet(PacketType::reject, now);
    packet.text = reason;
    send(std::move(packet), destination);
    diagnostics_.push_back(std::move(reason));
}

void LanSession::handle(const Datagram& datagram, const Packet& packet, std::uint32_t now)
{
    if (packet.peer_id == config_.peer_id) return;
    if (packet.type == PacketType::advertise && config_.role == SessionRole::joiner &&
        state_ == SessionState::discovering) {
        remote_ = datagram.source;
        state_ = SessionState::joining;
        send_join(now);
        return;
    }
    if (packet.type == PacketType::join && config_.role == SessionRole::host &&
        (state_ == SessionState::advertising || state_ == SessionState::connected)) {
        if (!identities_match(packet)) {
            reject(datagram.source,
                packet.data_identity != config_.data_identity ? "data identity mismatch" : "map identity mismatch", now);
            return;
        }
        remote_ = datagram.source;
        state_ = SessionState::connected;
        last_receive_at_ = now;
        send(base_packet(PacketType::accept, now), datagram.source);
        return;
    }
    if (packet.type == PacketType::accept && config_.role == SessionRole::joiner && state_ == SessionState::joining) {
        if (!identities_match(packet)) {
            state_ = SessionState::rejected;
            diagnostics_.push_back("accept packet identity mismatch");
            return;
        }
        remote_ = datagram.source;
        state_ = SessionState::connected;
        last_receive_at_ = now;
        return;
    }
    if (packet.type == PacketType::reject && config_.role == SessionRole::joiner && state_ == SessionState::joining) {
        state_ = SessionState::rejected;
        diagnostics_.push_back("join rejected: " + packet.text);
        return;
    }
    if (!remote_ || datagram.source != *remote_) return;
    if (packet.type == PacketType::disconnect && state_ == SessionState::connected) {
        state_ = SessionState::disconnected;
        diagnostics_.push_back("peer disconnected: " + packet.text);
        return;
    }
    if (packet.type == PacketType::command && state_ == SessionState::connected) {
        last_receive_at_ = now;
        if (packet.sequence < expected_command_sequence_) return;
        if (packet.sequence - expected_command_sequence_ >= maximum_pending_command_packets) {
            diagnostics_.push_back("command sequence exceeds reorder window");
            return;
        }
        pending_commands_.emplace(packet.sequence, packet.commands);
        while (true) {
            auto next = pending_commands_.find(expected_command_sequence_);
            if (next == pending_commands_.end()) break;
            received_commands_.insert(received_commands_.end(), next->second.begin(), next->second.end());
            pending_commands_.erase(next);
            ++expected_command_sequence_;
        }
    }
}

void LanSession::pump(std::uint32_t now)
{
    if (!active(state_)) return;
    for (std::size_t count = 0; count < maximum_receive_batch; ++count) {
        const auto datagram = endpoint_.receive();
        if (!datagram) break;
        try {
            const auto packet = decode_packet({datagram->bytes.data(), datagram->bytes.size()});
            handle(*datagram, packet, now);
        } catch (const ProtocolError& error) {
            diagnostics_.push_back("rejected datagram from " + format_address(datagram->source) + ": " + error.what());
        }
    }

    if (state_ == SessionState::advertising && tick_elapsed(now, last_send_at_) >= config_.retry_ticks) {
        auto advertisement = base_packet(PacketType::advertise, now);
        advertisement.text = "Zero Hour LAN host";
        send(std::move(advertisement), config_.discovery_address);
        last_send_at_ = now;
    } else if (state_ == SessionState::joining && tick_elapsed(now, last_send_at_) >= config_.retry_ticks) {
        send_join(now);
    }

    const bool waiting_timed_out = (state_ == SessionState::discovering || state_ == SessionState::joining) &&
        tick_elapsed(now, started_at_) >= config_.timeout_ticks;
    const bool connected_timed_out = state_ == SessionState::connected &&
        tick_elapsed(now, last_receive_at_) >= config_.timeout_ticks;
    if (waiting_timed_out || connected_timed_out) {
        state_ = SessionState::timed_out;
        diagnostics_.push_back("LAN peer timed out after " + std::to_string(config_.timeout_ticks) + " ticks");
    }
}

void LanSession::send_command(std::string command, std::uint32_t now)
{
    if (state_ != SessionState::connected || !remote_) throw TransportError("cannot send command while LAN session is not connected");
    auto packet = base_packet(PacketType::command, now);
    packet.sequence = next_command_sequence_++;
    packet.commands.push_back(std::move(command));
    send(std::move(packet), *remote_);
}

void LanSession::disconnect(std::string reason, std::uint32_t now)
{
    if (state_ != SessionState::connected || !remote_) throw TransportError("cannot disconnect an inactive LAN session");
    auto packet = base_packet(PacketType::disconnect, now);
    packet.text = std::move(reason);
    send(std::move(packet), *remote_);
    state_ = SessionState::disconnected;
}

} // namespace zh::lan
