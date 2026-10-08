/*
**	Command & Conquer Generals Zero Hour(tm)
**	Copyright 2025 Electronic Arts Inc.
**
**	This program is free software: you can redistribute it and/or modify
**	it under the terms of the GNU General Public License as published by
**	the Free Software Foundation, either version 3 of the License, or
**	(at your option) any later version.
**
**	This program is distributed in the hope that it will be useful,
**	but WITHOUT ANY WARRANTY; without even the implied warranty of
**	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
**	GNU General Public License for more details.
**
**	You should have received a copy of the GNU General Public License
**	along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

////////////////////////////////////////////////////////////////////////////////
//																																						//
//  (c) 2001-2003 Electronic Arts Inc.																				//
//																																						//
////////////////////////////////////////////////////////////////////////////////

#include "PreRTS.h"
#include "GameNetwork/IPEnumeration.h"
#include <arpa/inet.h>
#include <ifaddrs.h>
#include <net/if.h>
#include <unistd.h>
#include <cstring>
#include <memory>
namespace {
struct IPChain {
    EnumeratedIP* head = nullptr;
    ~IPChain() {
        while (head) {
            EnumeratedIP* next = head->getNext();
            head->deleteInstance(); head = next;
        }
    }
    EnumeratedIP* release() noexcept { auto* result=head; head=nullptr; return result; }
};
void appendIP(IPChain& candidate, UnsignedInt ip) {
    auto* node = newInstance(EnumeratedIP);
    MemoryPoolObjectHolder owner(node);
    AsciiString label;
    label.format("%u.%u.%u.%u", (ip>>24)&255u, (ip>>16)&255u, (ip>>8)&255u, ip&255u);
    node->setIPstring(label); node->setIP(ip);
    EnumeratedIP* previous = nullptr;
    EnumeratedIP* position = candidate.head;
    while (position && position->getIP() <= ip) {
        previous = position; position = position->getNext();
    }
    node->setNext(position);
    if (previous) previous->setNext(node); else candidate.head=node;
    owner.release();
}
}
IPEnumeration::IPEnumeration() : m_IPlist(nullptr) {}
IPEnumeration::~IPEnumeration() { IPChain retired{m_IPlist}; m_IPlist=nullptr; }
void IPEnumeration::replaceAddresses(std::span<const UnsignedInt> addresses) {
    IPChain candidate;
    for (UnsignedInt ip: addresses) appendIP(candidate,ip);
    IPChain retired{m_IPlist};
    m_IPlist=candidate.release();
}
EnumeratedIP* IPEnumeration::getAddresses() {
    if (m_IPlist) return m_IPlist;
    ifaddrs* interfaces = nullptr;
    if (::getifaddrs(&interfaces) != 0) return nullptr;
    std::unique_ptr<ifaddrs, decltype(&::freeifaddrs)> native(interfaces, &::freeifaddrs);
    IPChain candidate;
    for (const ifaddrs* item=interfaces; item; item=item->ifa_next) {
        if (!item->ifa_addr || item->ifa_addr->sa_family != AF_INET ||
            !(item->ifa_flags & IFF_UP)) continue;
        const auto* address = reinterpret_cast<const sockaddr_in*>(item->ifa_addr);
        const UnsignedInt ip = ntohl(address->sin_addr.s_addr);
        appendIP(candidate,ip); // Same complete offside chain owner as replacement.
    }
    m_IPlist = candidate.release();
    return m_IPlist;
}
AsciiString IPEnumeration::getMachineName() {
    char name[256]{};
    if (::gethostname(name, sizeof(name)) != 0 || !std::memchr(name, 0, sizeof(name)))
        return AsciiString();
    return AsciiString(name);
}
