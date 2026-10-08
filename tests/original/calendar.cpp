// SPDX-License-Identifier: GPL-3.0-or-later
#include "AllocationFault.h"
#include "Common/NativeCalendar.h"
#include "Common/NativeClock.h"
#include "Common/GameMemory.h"
#include "Common/UnicodeString.h"
#include <array>
#include <bit>
#include <cstddef>
#include <cstdio>
#include <cstring>
#include <stdexcept>
#include <string_view>

static_assert(sizeof(NativeCalendarTime) == 16 && alignof(NativeCalendarTime) == 2);
static_assert(offsetof(NativeCalendarTime,wYear) == 0 && offsetof(NativeCalendarTime,wMonth) == 2 &&
    offsetof(NativeCalendarTime,wDayOfWeek) == 4 && offsetof(NativeCalendarTime,wDay) == 6 &&
    offsetof(NativeCalendarTime,wHour) == 8 && offsetof(NativeCalendarTime,wMinute) == 10 &&
    offsetof(NativeCalendarTime,wSecond) == 12 && offsetof(NativeCalendarTime,wMilliseconds) == 14);
static_assert(std::endian::native == std::endian::little, "current Linux x86-64 replay boundary");
namespace {
void require(bool yes, const char* message) { if (!yes) throw std::runtime_error(message); }
void functional() {
    const NativeCalendarTime authored{2000,2,2,29,23,59,58,999};
    require(validNativeCalendar(authored), "Gregorian leap date");
    const std::array<unsigned char,16> expected{0xd0,0x07,2,0,2,0,29,0,23,0,59,0,58,0,0xe7,3};
    require(std::memcmp(&authored,expected.data(),16) == 0, "unchanged calendar field byte order");
    const auto before = nativeCalendarFromLocalTime(-1,999);
    require(before.wYear == 1969 && before.wMonth == 12 && before.wDay == 31 &&
        before.wDayOfWeek == 3 && before.wHour == 23 && before.wMinute == 59 &&
        before.wSecond == 59 && before.wMilliseconds == 999, "pre-epoch local conversion in UTC fixture");
    const auto epoch = nativeCalendarFromLocalTime(0,0);
    require(epoch.wYear == 1970 && epoch.wMonth == 1 && epoch.wDay == 1 &&
        epoch.wDayOfWeek == 4 && epoch.wHour == 0 && epoch.wMilliseconds == 0, "epoch conversion");
    require(getUnicodeTimeBuffer(authored) == UnicodeString(L"23:59") &&
            getUnicodeDateBuffer(authored) == UnicodeString(L"02/29/00"), "host C date/minute-resolution time");
    require(validNativeCalendar(nativeCalendarNow()), "native current local calendar");
    auto high = authored; high.wYear = 65535; high.wMonth = 12; high.wDay = 31;
    require(validNativeCalendar(high), "defined unsigned16 calendar upper year");
    const NativeCalendarTime invalids[] = {
        {}, {1900,2,0,29,0,0,0,0}, {2000,0,0,1,0,0,0,0},
        {2000,13,0,1,0,0,0,0}, {2000,4,0,31,0,0,0,0},
        {2000,1,7,1,0,0,0,0}, {2000,1,0,1,24,0,0,0},
        {2000,1,0,1,0,60,0,0}, {2000,1,0,1,0,0,60,0},
        {2000,1,0,1,0,0,0,1000}
    };
    for (const auto& invalid : invalids) {
        require(!validNativeCalendar(invalid), "invalid calendar fields rejected");
        bool rejected=false;
        try { (void)getUnicodeDateBuffer(invalid); } catch (const std::invalid_argument&) { rejected=true; }
        require(rejected, "calendar display admission rejects rather than normalizes");
    }
    bool rejected=false;
    try { (void)nativeCalendarFromLocalTime(0,1000); } catch (const std::invalid_argument&) { rejected=true; }
    require(rejected, "conversion millisecond boundary");
}
void faults() {
    const NativeCalendarTime authored{2000,2,2,29,23,59,58,999};
    // Initialize public libc locale/time machinery before taking allocation baselines.
    (void)nativeCalendarFromLocalTime(0,0);
    (void)getUnicodeDateBuffer(authored); (void)getUnicodeTimeBuffer(authored);
    for (bool date : {false,true}) for (std::size_t ordinal=0; ordinal<=2; ++ordinal) {
        UnicodeString accepted(L"previous accepted display");
        const auto* prior = accepted.str();
        const auto live = AllocationFault::live();
        bool rejected=false;
        AllocationFault::arm(ordinal);
        try { accepted = date ? getUnicodeDateBuffer(authored) : getUnicodeTimeBuffer(authored); }
        catch (const std::bad_alloc&) { rejected=true; }
        catch (...) { AllocationFault::disarm(); throw; }
        AllocationFault::disarm();
        if (ordinal==2) {
            require(!rejected && !AllocationFault::triggered(), "exact two-allocation terminal");
        } else {
            require(rejected && AllocationFault::triggered() && AllocationFault::live()==live &&
                accepted.str()==prior && accepted==UnicodeString(L"previous accepted display"),
                "failed calendar formatting preserves accepted string/backing");
            accepted = date ? getUnicodeDateBuffer(authored) : getUnicodeTimeBuffer(authored);
        }
        require(accepted == UnicodeString(date ? L"02/29/00" : L"23:59"), "corrected formatting retry");
    }
}
void clocks() {
    require(nativeElapsedMilliseconds(4,0xfffffff0u)==20 &&
            nativeElapsedMilliseconds(0x80000001u,0x7fffffffu)==2,
            "millisecond elapsed arithmetic crosses both native32 boundaries");
    for (UnsignedInt delay : {0u,499u,500u,501u,30000u}) {
        const UnsignedInt then=0xfffffff0u,now=then+delay;
        require(nativeElapsedMilliseconds(now,then)==delay, "wrapping timeout thresholds");
        require((nativeElapsedMilliseconds(now,then)>500)==(delay>500), "strict load threshold");
        require((nativeElapsedMilliseconds(now,then)<30000)==(delay<30000), "inclusive timeout completion");
    }
    for (Int fps : {-2147483647-1,-1,0,1000,1001,2147483647})
        require(nativeFrameDelayMilliseconds(fps)==0, "disabled/submillisecond frame cap");
    require(nativeFrameDelayMilliseconds(1)==999 && nativeFrameDelayMilliseconds(30)==32 &&
        nativeFrameDelayMilliseconds(60)==15 && nativeFrameDelayMilliseconds(999)==0,
        "ordinary source float cap conversion");
    (void)nativeMilliseconds();
    nativeSleepMilliseconds(0); // Public native yield; no long timing-based fixture.
}
}
int main(int argc,char** argv) {
    if (argc!=2) return 2;
    try {
        for (int repeat=0;repeat<3;++repeat) {
            initMemoryManager();
            try {
                if (std::string_view(argv[1])=="functional") functional();
                else if (std::string_view(argv[1])=="faults") faults();
                else if (std::string_view(argv[1])=="clocks") clocks();
                else throw std::runtime_error("unknown calendar family");
            } catch (...) { AllocationFault::disarm(); shutdownMemoryManager(); throw; }
            shutdownMemoryManager();
        }
        std::puts("original-calendar-contract: PASS");return 0;
    } catch (const std::exception& error) { std::fprintf(stderr,"%s\n",error.what());return 1; }
}
