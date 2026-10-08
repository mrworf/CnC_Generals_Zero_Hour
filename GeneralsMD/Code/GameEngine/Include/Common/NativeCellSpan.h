// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include "Lib/BaseType.h"
#include <algorithm>
#include <cstddef>
#include <limits>

struct NativeCellSpan {
    Int first;
    Int last;
    std::size_t offset;
};
// Admit indices before pointer formation. This does not prove backing ownership.
inline bool nativeCellSpan(Int width, Int height, Int x1, Int x2, Int y,
                           NativeCellSpan& output) noexcept
{
    if (width <= 0 || height <= 0 || y < 0 || y >= height || x1 > x2 ||
        x1 >= width || x2 < 0 ||
        static_cast<std::size_t>(height) > std::numeric_limits<std::size_t>::max()/static_cast<std::size_t>(width))
        return false;
    const Int first = std::max(x1,0);
    const Int last = std::min(x2,width-1);
    const NativeCellSpan candidate{first,last,
        static_cast<std::size_t>(y)*static_cast<std::size_t>(width)+static_cast<std::size_t>(first)};
    output = candidate;
    return true;
}
