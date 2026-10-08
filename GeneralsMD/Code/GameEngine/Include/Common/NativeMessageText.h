// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include "Common/UnicodeString.h"
#include <utility>

// Complete offside formatting precedes any UI delivery. The only C varargs
// anchor is the original formatter's plain WideChar pointer.
template<class... Args>
UnicodeString formatOriginalMessageText(const UnicodeString& format,Args&&... args) {
    UnicodeString prepared;
    prepared.format(format.str(),std::forward<Args>(args)...);
    return prepared;
}
