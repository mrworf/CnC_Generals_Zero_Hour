// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include "Lib/BaseType.h"
UnsignedInt nativeMilliseconds() noexcept;
void nativeSleepMilliseconds(UnsignedInt milliseconds);
constexpr UnsignedInt nativeElapsedMilliseconds(UnsignedInt now, UnsignedInt then) noexcept {
    return now-then;
}
inline UnsignedInt nativeFrameDelayMilliseconds(Int fps) noexcept {
    return fps>0 && fps<1000 ? UnsignedInt(1000.0f/fps-1.0f) : 0;
}
