// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include "Lib/BaseType.h"
#include <ctime>
class UnicodeString;

// Original SYSTEMTIME field widths/order, not a host struct tm or time_t encoding.
struct NativeCalendarTime {
    UnsignedShort wYear = 0, wMonth = 0, wDayOfWeek = 0, wDay = 0;
    UnsignedShort wHour = 0, wMinute = 0, wSecond = 0, wMilliseconds = 0;
};
bool validNativeCalendar(const NativeCalendarTime&) noexcept;
NativeCalendarTime nativeCalendarFromLocalTime(std::time_t, UnsignedShort milliseconds);
NativeCalendarTime nativeCalendarNow();
UnicodeString getUnicodeTimeBuffer(NativeCalendarTime);
UnicodeString getUnicodeDateBuffer(NativeCalendarTime);
