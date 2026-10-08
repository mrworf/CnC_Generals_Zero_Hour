// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include "Common/AsciiString.h"
#include "Common/ScienceType.h"
#include "Common/Upgrade.h"
#include <vector>
class Snapshot;

// Borrowed original-runtime services. The real GameState registers after init;
// transports must never construct a parent game merely to obtain a codec.
struct NativeTransferServices {
    void* owner=nullptr;
    AsciiString (*encodeMap)(void*,const AsciiString&)=nullptr;
    AsciiString (*decodeMap)(void*,const AsciiString&)=nullptr;
    void (*postProcess)(void*,Snapshot*)=nullptr;
    AsciiString (*encodeScience)(void*,ScienceType)=nullptr;
    ScienceType (*decodeScience)(void*,const AsciiString&)=nullptr;
    std::vector<AsciiString> (*encodeUpgrades)(void*,const UpgradeMaskType&)=nullptr;
    UpgradeMaskType (*decodeUpgrade)(void*,const AsciiString&)=nullptr;
};
const NativeTransferServices& nativeTransferServices() noexcept;
void nativeBindTransferServices(NativeTransferServices services);
void nativeWithdrawTransferServices(const void* owner) noexcept;
