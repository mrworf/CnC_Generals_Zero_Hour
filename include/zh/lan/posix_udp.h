#pragma once

#include "zh/lan/transport.h"

#include <cstdint>

namespace zh::lan {

class PosixUdpEndpoint final : public DatagramEndpoint {
public:
    explicit PosixUdpEndpoint(EndpointAddress bind_address);
    ~PosixUdpEndpoint() override;

    PosixUdpEndpoint(const PosixUdpEndpoint&) = delete;
    PosixUdpEndpoint& operator=(const PosixUdpEndpoint&) = delete;

    const EndpointAddress& local_address() const noexcept override { return local_address_; }
    void send_to(const EndpointAddress& destination, foundation::ByteView bytes) override;
    std::optional<Datagram> receive() override;

    bool wait_readable(int timeout_milliseconds);

private:
    int socket_ = -1;
    EndpointAddress local_address_;
};

} // namespace zh::lan
