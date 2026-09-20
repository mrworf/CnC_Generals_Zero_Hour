#include "zh/lan/posix_udp.h"

#include "zh/lan/protocol.h"

#include <arpa/inet.h>
#include <cerrno>
#include <cstring>
#include <fcntl.h>
#include <netdb.h>
#include <poll.h>
#include <sys/socket.h>
#include <unistd.h>

#include <array>
#include <memory>

namespace zh::lan {
namespace {

struct AddrInfoDeleter {
    void operator()(addrinfo* value) const noexcept { if (value != nullptr) freeaddrinfo(value); }
};

using AddrInfo = std::unique_ptr<addrinfo, AddrInfoDeleter>;

AddrInfo resolve(const EndpointAddress& address, bool passive)
{
    addrinfo hints{};
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_DGRAM;
    hints.ai_protocol = IPPROTO_UDP;
    if (passive) hints.ai_flags = AI_PASSIVE;
    addrinfo* raw = nullptr;
    const auto service = std::to_string(address.port);
    const int result = getaddrinfo(address.host.empty() ? nullptr : address.host.c_str(), service.c_str(), &hints, &raw);
    if (result != 0) {
        throw TransportError("cannot resolve UDP address '" + format_address(address) + "': " + gai_strerror(result));
    }
    return AddrInfo(raw);
}

std::string errno_message(std::string_view operation)
{
    return std::string(operation) + ": " + std::strerror(errno);
}

EndpointAddress from_sockaddr(const sockaddr_in& address)
{
    std::array<char, INET_ADDRSTRLEN> text{};
    if (inet_ntop(AF_INET, &address.sin_addr, text.data(), text.size()) == nullptr) {
        throw TransportError(errno_message("cannot format UDP peer address"));
    }
    return {text.data(), ntohs(address.sin_port)};
}

} // namespace

PosixUdpEndpoint::PosixUdpEndpoint(EndpointAddress bind_address)
{
    const auto resolved = resolve(bind_address, true);
    socket_ = ::socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (socket_ < 0) throw TransportError(errno_message("cannot create UDP socket"));
    try {
        const int enabled = 1;
        if (setsockopt(socket_, SOL_SOCKET, SO_BROADCAST, &enabled, sizeof(enabled)) != 0) {
            throw TransportError(errno_message("cannot enable UDP broadcast"));
        }
        const int flags = fcntl(socket_, F_GETFL, 0);
        if (flags < 0 || fcntl(socket_, F_SETFL, flags | O_NONBLOCK) != 0) {
            throw TransportError(errno_message("cannot make UDP socket nonblocking"));
        }
        if (::bind(socket_, resolved->ai_addr, resolved->ai_addrlen) != 0) {
            throw TransportError(errno_message("cannot bind UDP socket to " + format_address(bind_address)));
        }
        sockaddr_in actual{};
        socklen_t actual_size = sizeof(actual);
        if (getsockname(socket_, reinterpret_cast<sockaddr*>(&actual), &actual_size) != 0) {
            throw TransportError(errno_message("cannot query bound UDP address"));
        }
        local_address_ = from_sockaddr(actual);
    } catch (...) {
        ::close(socket_);
        socket_ = -1;
        throw;
    }
}

PosixUdpEndpoint::~PosixUdpEndpoint()
{
    if (socket_ >= 0) ::close(socket_);
}

void PosixUdpEndpoint::send_to(const EndpointAddress& destination, foundation::ByteView bytes)
{
    if (bytes.size > maximum_datagram_bytes) throw TransportError("UDP datagram exceeds packet limit");
    if (bytes.size != 0 && bytes.data == nullptr) throw TransportError("UDP datagram has null bytes");
    const auto resolved = resolve(destination, false);
    const auto sent = ::sendto(socket_, bytes.data, bytes.size, 0, resolved->ai_addr, resolved->ai_addrlen);
    if (sent < 0) throw TransportError(errno_message("cannot send UDP datagram to " + format_address(destination)));
    if (static_cast<std::size_t>(sent) != bytes.size) throw TransportError("UDP datagram send was incomplete");
}

std::optional<Datagram> PosixUdpEndpoint::receive()
{
    std::array<foundation::UInt8, maximum_datagram_bytes + 1> buffer{};
    sockaddr_in source{};
    socklen_t source_size = sizeof(source);
    const auto received = recvfrom(socket_, buffer.data(), buffer.size(), 0,
        reinterpret_cast<sockaddr*>(&source), &source_size);
    if (received < 0) {
        if (errno == EAGAIN || errno == EWOULDBLOCK) return std::nullopt;
        throw TransportError(errno_message("cannot receive UDP datagram"));
    }
    if (static_cast<std::size_t>(received) > maximum_datagram_bytes) {
        throw TransportError("received UDP datagram exceeds packet limit");
    }
    Datagram datagram;
    datagram.source = from_sockaddr(source);
    datagram.bytes.assign(buffer.begin(), buffer.begin() + received);
    return datagram;
}

bool PosixUdpEndpoint::wait_readable(int timeout_milliseconds)
{
    if (timeout_milliseconds < 0) throw TransportError("UDP poll timeout must be nonnegative");
    pollfd descriptor{socket_, POLLIN, 0};
    int result;
    do {
        result = poll(&descriptor, 1, timeout_milliseconds);
    } while (result < 0 && errno == EINTR);
    if (result < 0) throw TransportError(errno_message("UDP poll failed"));
    if ((descriptor.revents & (POLLERR | POLLHUP | POLLNVAL)) != 0) {
        throw TransportError("UDP poll reported socket error events " + std::to_string(descriptor.revents));
    }
    return result > 0 && (descriptor.revents & POLLIN) != 0;
}

} // namespace zh::lan
