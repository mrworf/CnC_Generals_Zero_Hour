#pragma once

#include "zh/lan/protocol.h"
#include "zh/lan/transport.h"

#include <cstdint>
#include <iosfwd>
#include <optional>

namespace zh::lan {

enum class HarnessRole { host, joiner };

struct HarnessConfig {
    HarnessRole role = HarnessRole::host;
    EndpointAddress bind_address;
    EndpointAddress discovery_address{"255.255.255.255", lobby_port};
    std::optional<EndpointAddress> direct_connect;
    std::uint32_t peer_id = 1;
    std::uint32_t data_identity = 0x5a484441U;
    std::uint32_t map_identity = 0x5a484d41U;
    std::uint32_t timeout_milliseconds = 3000;
};

int run_headless_peer(const HarnessConfig& config, std::ostream& output, std::ostream& errors);

} // namespace zh::lan
