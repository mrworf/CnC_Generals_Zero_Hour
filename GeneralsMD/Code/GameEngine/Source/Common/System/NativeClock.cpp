// SPDX-License-Identifier: GPL-3.0-or-later
#include "Common/NativeClock.h"
#include <chrono>
#include <thread>
UnsignedInt nativeMilliseconds() noexcept {
    const auto count=std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::steady_clock::now().time_since_epoch()).count();
    return static_cast<UnsignedInt>(count);
}
void nativeSleepMilliseconds(UnsignedInt milliseconds) {
    if (milliseconds==0) std::this_thread::yield();
    else std::this_thread::sleep_for(std::chrono::milliseconds(milliseconds));
}
