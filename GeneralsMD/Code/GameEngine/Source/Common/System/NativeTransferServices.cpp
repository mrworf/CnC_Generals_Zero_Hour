// SPDX-License-Identifier: GPL-3.0-or-later
#include "Common/NativeTransferServices.h"
#include "Common/Xfer.h"
namespace { NativeTransferServices active; }
const NativeTransferServices& nativeTransferServices() noexcept { return active; }
void nativeBindTransferServices(NativeTransferServices services) {
    if (!services.owner || !services.encodeMap || !services.decodeMap || !services.postProcess
        || !services.encodeScience || !services.decodeScience || !services.encodeUpgrades || !services.decodeUpgrade
        || (active.owner && active.owner!=services.owner)) throw XFER_INVALID_PARAMETERS;
    active=services;
}
void nativeWithdrawTransferServices(const void* owner) noexcept {
    if (active.owner==owner) active={};
}
