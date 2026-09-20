#include "zh/lan/session.h"

#include <cstdint>
#include <iostream>
#include <limits>
#include <stdexcept>
#include <string_view>

namespace {

void check(bool condition, const char* message)
{
    if (!condition) throw std::runtime_error(message);
}

zh::lan::SessionConfig host_config()
{
    return {zh::lan::SessionRole::host, 1, 0x11112222U, 0x33334444U,
        {"broadcast", zh::lan::lobby_port}, std::nullopt, 3, 20};
}

zh::lan::SessionConfig join_config(bool direct = false)
{
    auto config = host_config();
    config.role = zh::lan::SessionRole::joiner;
    config.peer_id = 2;
    if (direct) config.direct_connect = zh::lan::EndpointAddress{"127.0.0.2", zh::lan::lobby_port};
    return config;
}

void pump_pair(zh::lan::VirtualNetwork& network, zh::lan::LanSession& first, zh::lan::LanSession& second,
    std::uint32_t begin, std::uint32_t end)
{
    for (std::uint32_t tick = begin; tick <= end; ++tick) {
        network.set_tick(tick);
        first.pump(tick);
        second.pump(tick);
    }
}

bool has_diagnostic(const zh::lan::LanSession& session, std::string_view text)
{
    for (const auto& diagnostic : session.diagnostics()) {
        if (diagnostic.find(text) != std::string::npos) return true;
    }
    return false;
}

} // namespace

int main()
{
    using namespace zh::lan;
    {
        VirtualNetwork network;
        auto host_endpoint = network.open({"127.0.0.2", lobby_port});
        auto join_endpoint = network.open({"127.0.0.3", lobby_port});
        LanSession host(*host_endpoint, host_config());
        LanSession joiner(*join_endpoint, join_config());
        host.start(0);
        joiner.start(0);
        pump_pair(network, host, joiner, 0, 2);
        check(host.state() == SessionState::connected && joiner.state() == SessionState::connected,
            "virtual discovery connects peers");

        network.inject_next(VirtualFault::reorder, 2);
        joiner.send_command("first", 3);
        joiner.send_command("second", 3);
        network.set_tick(3);
        host.pump(3);
        check(host.received_commands().empty(), "reordered command waits for gap");
        network.set_tick(5);
        host.pump(5);
        check(host.received_commands() == std::vector<std::string>({"first", "second"}), "reordered commands restored");

        network.inject_next(VirtualFault::duplicate);
        joiner.send_command("once", 6);
        network.set_tick(6);
        host.pump(6);
        check(host.received_commands().back() == "once" && host.received_commands().size() == 3,
            "duplicate command suppressed");

        network.inject_next(VirtualFault::delay, 2);
        joiner.send_command("delayed", 7);
        network.set_tick(7);
        host.pump(7);
        check(host.received_commands().size() == 3, "delayed datagram withheld");
        network.set_tick(9);
        host.pump(9);
        check(host.received_commands().back() == "delayed", "delayed datagram delivered");

        joiner.disconnect("test complete", 10);
        network.set_tick(10);
        host.pump(10);
        check(host.state() == SessionState::disconnected, "peer observes clean disconnect");
    }
    {
        VirtualNetwork network;
        auto host_endpoint = network.open({"127.0.0.2", lobby_port});
        auto join_endpoint = network.open({"127.0.0.3", lobby_port});
        LanSession host(*host_endpoint, host_config());
        LanSession joiner(*join_endpoint, join_config(true));
        network.inject_next(VirtualFault::drop);
        host.start(0); // Drop only the advertisement; direct connect does not need it.
        network.inject_next(VirtualFault::drop);
        joiner.start(0); // Drop first join and prove retry.
        pump_pair(network, host, joiner, 0, 6);
        check(host.state() == SessionState::connected && joiner.state() == SessionState::connected,
            "dropped join retried over direct connect");

        network.inject_next(VirtualFault::corrupt);
        joiner.send_command("corrupt", 7);
        network.set_tick(7);
        host.pump(7);
        check(host.received_commands().empty(), "corrupt packet does not mutate commands");
        check(has_diagnostic(host, "version"), "corruption diagnostic reports packet rejection");
    }
    for (const bool data_mismatch : {false, true}) {
        VirtualNetwork network;
        auto host_endpoint = network.open({"127.0.0.2", lobby_port});
        auto join_endpoint = network.open({"127.0.0.3", lobby_port});
        LanSession host(*host_endpoint, host_config());
        auto mismatch = join_config(true);
        if (data_mismatch) mismatch.data_identity ^= 1U;
        else mismatch.map_identity ^= 1U;
        LanSession joiner(*join_endpoint, mismatch);
        host.start(0);
        joiner.start(0);
        pump_pair(network, host, joiner, 0, 2);
        check(joiner.state() == SessionState::rejected, "identity mismatch rejected");
        check(has_diagnostic(joiner, data_mismatch ? "data identity mismatch" : "map identity mismatch"),
            "identity mismatch is actionable");
    }
    {
        VirtualNetwork network;
        auto endpoint = network.open({"127.0.0.3", lobby_port});
        auto config = join_config(true);
        config.timeout_ticks = 8;
        LanSession joiner(*endpoint, config);
        const auto start = std::numeric_limits<std::uint32_t>::max() - 3;
        network.set_tick(start);
        joiner.start(start);
        for (std::uint32_t elapsed = 0; elapsed <= 8; ++elapsed) {
            const auto now = start + elapsed;
            network.set_tick(now);
            joiner.pump(now);
        }
        check(joiner.state() == SessionState::timed_out, "silent peer times out across clock wrap");
        check(has_diagnostic(joiner, "timed out"), "timeout is diagnostic");
    }
    {
        VirtualNetwork network;
        auto endpoint = network.open({"127.0.0.3", lobby_port});
        bool duplicate_rejected = false;
        try {
            auto duplicate = network.open({"127.0.0.3", lobby_port});
        } catch (const TransportError& error) {
            duplicate_rejected = std::string_view(error.what()).find("already bound") != std::string_view::npos;
        }
        check(duplicate_rejected, "duplicate virtual bind rejected");
        network.inject_next(VirtualFault::delay, 1);
        bool pending_fault_rejected = false;
        try {
            network.inject_next(VirtualFault::drop);
        } catch (const TransportError&) {
            pending_fault_rejected = true;
        }
        check(pending_fault_rejected, "overlapping fault injection rejected");
    }
    std::cout << "deterministic virtual LAN transport: ok\n";
    return 0;
}
