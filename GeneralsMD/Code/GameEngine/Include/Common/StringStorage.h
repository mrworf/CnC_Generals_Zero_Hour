// SPDX-License-Identifier: GPL-3.0-or-later
// Native ownership helpers for the original copy-on-write string classes.
#pragma once
#include <atomic>
#include <cstdint>
#include <limits>
#include "Common/Errors.h"
namespace OriginalStringStorage {
inline void retain(std::atomic<std::uint32_t>& refs) {
    auto count=refs.load(std::memory_order_relaxed);
    do {
        if(count==0 || count==std::numeric_limits<std::uint32_t>::max())throw ERROR_OUT_OF_MEMORY;
    } while(!refs.compare_exchange_weak(count,count+1,std::memory_order_relaxed));
}
inline bool release(std::atomic<std::uint32_t>& refs) noexcept {
    return refs.fetch_sub(1,std::memory_order_acq_rel)==1;
}
}
