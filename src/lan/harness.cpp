#include "zh/lan/harness.h"

#include "zh/lan/posix_udp.h"
#include "zh/lan/session.h"

#include <chrono>
#include <ostream>

namespace zh::lan {
namespace {

std::uint32_t monotonic_milliseconds() noexcept
{
    const auto value = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::steady_clock::now().time_since_epoch()).count();
    return static_cast<std::uint32_t>(value);
}

} // namespace

int run_headless_peer(const HarnessConfig& config, std::ostream& output, std::ostream& errors)
{
    try {
        PosixUdpEndpoint endpoint(config.bind_address);
        SessionConfig session_config;
        session_config.role = config.role == HarnessRole::host ? SessionRole::host : SessionRole::joiner;
        session_config.peer_id = config.peer_id;
        session_config.data_identity = config.data_identity;
        session_config.map_identity = config.map_identity;
        session_config.discovery_address = config.discovery_address;
        session_config.direct_connect = config.direct_connect;
        session_config.retry_ticks = 50;
        session_config.timeout_ticks = config.timeout_milliseconds;
        LanSession session(endpoint, session_config);

        const auto started = monotonic_milliseconds();
        session.start(started);
        output << "lan: bound=" << format_address(endpoint.local_address())
               << " lobby-port=" << lobby_port << " game-base-port=" << game_port_base << '\n';
        output << "lan: role=" << (config.role == HarnessRole::host ? "host" : "joiner")
               << " discovery=" << format_address(config.discovery_address) << '\n';

        bool reported_connection = false;
        bool sent_command = false;
        while (true) {
            const auto now = monotonic_milliseconds();
            session.pump(now);
            if (session.state() == SessionState::connected && !reported_connection) {
                output << "lan: joined peer=" << format_address(*session.remote_address())
                       << " data-identity=" << config.data_identity
                       << " map-identity=" << config.map_identity << '\n';
                reported_connection = true;
            }
            if (config.role == HarnessRole::joiner && session.state() == SessionState::connected && !sent_command) {
                session.send_command("m11-command", now);
                output << "lan: command sent=m11-command\n";
                session.disconnect("clean shutdown", now);
                output << "lan: disconnected cleanly\n";
                return 0;
            }
            if (config.role == HarnessRole::host && session.state() == SessionState::disconnected) {
                if (session.received_commands() != std::vector<std::string>{"m11-command"}) {
                    errors << "lan: peer disconnected without expected command\n";
                    return 4;
                }
                output << "lan: command received=m11-command\nlan: disconnected cleanly\n";
                return 0;
            }
            if (session.state() == SessionState::rejected || session.state() == SessionState::timed_out) {
                for (const auto& diagnostic : session.diagnostics()) errors << "lan: " << diagnostic << '\n';
                return 3;
            }
            if (tick_elapsed(now, started) >= config.timeout_milliseconds) break;
            (void)endpoint.wait_readable(5);
        }
        errors << "lan: peer timed out at harness deadline after " << config.timeout_milliseconds << " ms\n";
        return 3;
    } catch (const std::exception& error) {
        errors << "lan: " << error.what() << '\n';
        return 2;
    }
}

} // namespace zh::lan
