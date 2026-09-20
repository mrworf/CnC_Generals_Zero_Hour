#pragma once

#include "zh/foundation/types.h"

#include <cstdint>
#include <deque>
#include <memory>
#include <optional>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

namespace zh::lan {

struct EndpointAddress {
    std::string host;
    std::uint16_t port = 0;

    bool operator==(const EndpointAddress& other) const { return host == other.host && port == other.port; }
    bool operator!=(const EndpointAddress& other) const { return !(*this == other); }
};

std::string format_address(const EndpointAddress& address);

struct Datagram {
    EndpointAddress source;
    std::vector<foundation::UInt8> bytes;
};

class TransportError : public std::runtime_error {
public:
    using std::runtime_error::runtime_error;
};

class DatagramEndpoint {
public:
    virtual ~DatagramEndpoint() = default;
    virtual const EndpointAddress& local_address() const noexcept = 0;
    virtual void send_to(const EndpointAddress& destination, foundation::ByteView bytes) = 0;
    virtual std::optional<Datagram> receive() = 0;
};

enum class VirtualFault {
    none,
    drop,
    duplicate,
    reorder,
    corrupt,
    delay,
};

class VirtualNetwork;

class VirtualEndpoint final : public DatagramEndpoint {
public:
    ~VirtualEndpoint() override;
    const EndpointAddress& local_address() const noexcept override;
    void send_to(const EndpointAddress& destination, foundation::ByteView bytes) override;
    std::optional<Datagram> receive() override;

private:
    friend class VirtualNetwork;
    VirtualEndpoint(VirtualNetwork& network, EndpointAddress address);
    VirtualNetwork* network_;
    EndpointAddress address_;
};

class VirtualNetwork {
public:
    std::unique_ptr<VirtualEndpoint> open(EndpointAddress address);
    void set_tick(std::uint32_t tick) noexcept { tick_ = tick; }
    std::uint32_t tick() const noexcept { return tick_; }
    void inject_next(VirtualFault fault, std::uint32_t delay_ticks = 1);

private:
    friend class VirtualEndpoint;
    struct QueuedDatagram {
        EndpointAddress source;
        EndpointAddress destination;
        std::vector<foundation::UInt8> bytes;
        std::uint32_t delivery_tick = 0;
        std::uint64_t order = 0;
    };
    void close(const EndpointAddress& address) noexcept;
    void send(const EndpointAddress& source, const EndpointAddress& destination, foundation::ByteView bytes);
    std::optional<Datagram> receive(const EndpointAddress& destination);

    std::vector<EndpointAddress> endpoints_;
    std::deque<QueuedDatagram> queue_;
    VirtualFault next_fault_ = VirtualFault::none;
    std::uint32_t next_delay_ = 1;
    std::uint32_t tick_ = 0;
    std::uint64_t next_order_ = 0;
};

} // namespace zh::lan
