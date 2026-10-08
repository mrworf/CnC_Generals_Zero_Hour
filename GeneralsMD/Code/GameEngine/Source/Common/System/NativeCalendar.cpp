// SPDX-License-Identifier: GPL-3.0-or-later
#include "Common/NativeCalendar.h"
#include "Common/UnicodeString.h"
#include <array>
#include <chrono>
#include <cwchar>
#include <locale.h>
#include <stdexcept>

bool validNativeCalendar(const NativeCalendarTime& value) noexcept {
    if (value.wYear == 0 || value.wMonth == 0 || value.wMonth > 12 ||
        value.wDayOfWeek > 6 || value.wDay == 0 || value.wHour > 23 ||
        value.wMinute > 59 || value.wSecond > 59 || value.wMilliseconds > 999)
        return false;
    constexpr std::array<unsigned,12> days{31,28,31,30,31,30,31,31,30,31,30,31};
    const unsigned year = value.wYear;
    const bool leap = year % 4 == 0 && (year % 100 != 0 || year % 400 == 0);
    return value.wDay <= days[value.wMonth-1] + (value.wMonth == 2 && leap);
}
NativeCalendarTime nativeCalendarFromLocalTime(std::time_t seconds, UnsignedShort milliseconds) {
    std::tm local{};
    if (!localtime_r(&seconds, &local) || local.tm_year < -1899 ||
        local.tm_year > 65535-1900 || local.tm_mon < 0 || local.tm_mon > 11 ||
        local.tm_mday < 1 || local.tm_mday > 31 || local.tm_wday < 0 || local.tm_wday > 6 ||
        local.tm_hour < 0 || local.tm_hour > 23 || local.tm_min < 0 || local.tm_min > 59 ||
        local.tm_sec < 0 || local.tm_sec > 59 || milliseconds > 999)
        throw std::invalid_argument("invalid native local calendar time");
    NativeCalendarTime value{UnsignedShort(local.tm_year+1900), UnsignedShort(local.tm_mon+1),
        UnsignedShort(local.tm_wday), UnsignedShort(local.tm_mday), UnsignedShort(local.tm_hour),
        UnsignedShort(local.tm_min), UnsignedShort(local.tm_sec), milliseconds};
    if (!validNativeCalendar(value)) throw std::invalid_argument("invalid native calendar fields");
    return value;
}
NativeCalendarTime nativeCalendarNow() {
    const auto now = std::chrono::system_clock::now();
    const auto whole = std::chrono::floor<std::chrono::seconds>(now);
    const auto millis = std::chrono::duration_cast<std::chrono::milliseconds>(now-whole).count();
    return nativeCalendarFromLocalTime(std::chrono::system_clock::to_time_t(whole), UnsignedShort(millis));
}
namespace {
UnicodeString formatCalendar(NativeCalendarTime value, const wchar_t* pattern) {
    if (!validNativeCalendar(value)) throw std::invalid_argument("invalid calendar display fields");
    std::tm local{};
    local.tm_year = value.wYear-1900; local.tm_mon = value.wMonth-1;
    local.tm_wday = value.wDayOfWeek; local.tm_mday = value.wDay;
    local.tm_hour = value.wHour; local.tm_min = value.wMinute; local.tm_sec = value.wSecond;
    struct LocaleOwner {
        locale_t value = newlocale(LC_TIME_MASK | LC_CTYPE_MASK, "", nullptr);
        ~LocaleOwner() { if (value) freelocale(value); }
    } locale;
    if (!locale.value) throw std::runtime_error("native calendar locale unavailable");
    std::array<wchar_t,256> buffer{};
    if (!wcsftime_l(buffer.data(), buffer.size(), pattern, &local, locale.value))
        throw std::runtime_error("native calendar display exceeds capacity");
    UnicodeString result;
    result.set(buffer.data());
    return result;
}
}
UnicodeString getUnicodeDateBuffer(NativeCalendarTime value) { return formatCalendar(value, L"%x"); }
UnicodeString getUnicodeTimeBuffer(NativeCalendarTime value) { return formatCalendar(value, L"%H:%M"); }
