// SPDX-License-Identifier: GPL-3.0-or-later
#include "Common/NativeInputSettings.h"
#include <SDL3/SDL_hints.h>
#include <charconv>
#include <cstring>
#include <limits>
UnsignedInt nativeDoubleClickMilliseconds() noexcept {
    constexpr UnsignedInt defaultMilliseconds = 500;
    const char* hint = SDL_GetHint(SDL_HINT_MOUSE_DOUBLE_CLICK_TIME);
    if (!hint) return defaultMilliseconds;
    UnsignedInt value = 0;
    const char* end = hint + std::strlen(hint);
    const auto result = std::from_chars(hint, end, value);
    // SDL's event owner uses signed millisecond arithmetic.
    return result.ec == std::errc{} && result.ptr == end &&
           value <= UnsignedInt(std::numeric_limits<Int>::max())
        ? value : defaultMilliseconds;
}
