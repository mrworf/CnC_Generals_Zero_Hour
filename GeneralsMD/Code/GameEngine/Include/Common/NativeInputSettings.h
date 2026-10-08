// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include "Lib/BaseType.h"
// Match the adopted SDL mouse event owner's public double-click policy.
// SDL_MOUSE_DOUBLE_CLICK_TIME overrides SDL's documented 500ms default.
UnsignedInt nativeDoubleClickMilliseconds() noexcept;
