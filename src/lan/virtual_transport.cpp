#include "zh/lan/transport.h"

#include "zh/lan/protocol.h"

#include <algorithm>

namespace zh::lan {
namespace {

constexpr std::size_t maximum_queued_datagrams = 256;

bool is_broadcast(const EndpointAddress& address)
{
    return address.host == "255.255.255.255" || address.host == "broadcast";
}

} // namespace

std::string format_address(const EndpointAddress& address)
{
    return address.host + ":" + std::to_string(address.port);
}

VirtualEndpoint::VirtualEndpoint(VirtualNetwork& network, EndpointAddress address)
    : network_(&network), address_(std::move(address))
{
}

VirtualEndpoint::~VirtualEndpoint()
{
    if (network_ != nullptr) network_->close(address_);
}

const EndpointAddress& VirtualEndpoint::local_address() const noexcept
{
    return address_;
}

void VirtualEndpoint::send_to(const EndpointAddress& destination, foundation::ByteView bytes)
{
    network_->send(address_, destination, bytes);
}

std::optional<Datagram> VirtualEndpoint::receive()
{
    return network_->receive(address_);
}

std::unique_ptr<VirtualEndpoint> VirtualNetwork::open(EndpointAddress address)
{
    if (address.host.empty() || address.port == 0) throw TransportError("virtual endpoint address is incomplete");
    if (std::find(endpoints_.begin(), endpoints_.end(), address) != endpoints_.end()) {
        throw TransportError("virtual endpoint already bound: " + format_address(address));
    }
    endpoints_.push_back(address);
    return std::unique_ptr<VirtualEndpoint>(new VirtualEndpoint(*this, std::move(address)));
}

void VirtualNetwork::inject_next(VirtualFault fault, std::uint32_t delay_ticks)
{
    if (next_fault_ != VirtualFault::none) throw TransportError("a virtual datagram fault is already pending");
    if ((fault == VirtualFault::delay || fault == VirtualFault::reorder) && delay_ticks == 0) {
        throw TransportError("virtual datagram delay must be positive");
    }
    next_fault_ = fault;
    next_delay_ = delay_ticks;
}

void VirtualNetwork::close(const EndpointAddress& address) noexcept
{
    endpoints_.erase(std::remove(endpoints_.begin(), endpoints_.end(), address), endpoints_.end());
    queue_.erase(std::remove_if(queue_.begin(), queue_.end(), [&](const auto& item) {
        return item.source == address || item.destination == address;
    }), queue_.end());
}

void VirtualNetwork::send(
    const EndpointAddress& source, const EndpointAddress& destination, foundation::ByteView bytes)
{
    if (bytes.size > maximum_datagram_bytes) throw TransportError("virtual datagram exceeds packet limit");
    if (bytes.size != 0 && bytes.data == nullptr) throw TransportError("virtual datagram has null bytes");
    if (queue_.size() >= maximum_queued_datagrams) throw TransportError("virtual datagram queue is full");

    const auto fault = next_fault_;
    next_fault_ = VirtualFault::none;
    if (fault == VirtualFault::drop) return;

    auto enqueue = [&](const EndpointAddress& target, std::uint32_t delay) {
        QueuedDatagram item;
        item.source = source;
        item.destination = target;
        item.bytes.assign(bytes.data, bytes.data + bytes.size);
        item.delivery_tick = tick_ + delay;
        item.order = next_order_++;
        if (fault == VirtualFault::corrupt && item.bytes.size() > 4) item.bytes[4] ^= 0x7fU;
        queue_.push_back(std::move(item));
    };

    const std::uint32_t delay = (fault == VirtualFault::delay || fault == VirtualFault::reorder) ? next_delay_ : 0;
    if (is_broadcast(destination)) {
        for (const auto& target : endpoints_) {
            if (target != source && target.port == destination.port) enqueue(target, delay);
        }
    } else {
        enqueue(destination, delay);
    }
    if (fault == VirtualFault::duplicate && !queue_.empty()) {
        auto duplicate = queue_.back();
        duplicate.order = next_order_++;
        queue_.push_back(std::move(duplicate));
    }
}

std::optional<Datagram> VirtualNetwork::receive(const EndpointAddress& destination)
{
    auto selected = queue_.end();
    for (auto iterator = queue_.begin(); iterator != queue_.end(); ++iterator) {
        if (iterator->destination != destination || !tick_after_or_equal(tick_, iterator->delivery_tick)) continue;
        if (selected == queue_.end() || iterator->delivery_tick < selected->delivery_tick ||
            (iterator->delivery_tick == selected->delivery_tick && iterator->order < selected->order)) {
            selected = iterator;
        }
    }
    if (selected == queue_.end()) return std::nullopt;
    Datagram result{selected->source, std::move(selected->bytes)};
    queue_.erase(selected);
    return result;
}

} // namespace zh::lan
