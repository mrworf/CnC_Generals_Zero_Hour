#pragma once

#include <algorithm>
#include <atomic>
#include <bitset>
#include <cassert>
#include <chrono>
#include <cctype>
#include <cerrno>
#include <cfenv>
#include <climits>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cwctype>
#include <deque>
#include <fstream>
#include <functional>
#include <ext/hash_map>
#include <list>
#include <map>
#include <mutex>
#include <new>
#include <queue>
#include <set>
#include <stack>
#include <string>
#include <string_view>
#include <strings.h>
#include <thread>
#include <unordered_map>
#include <utility>
#include <vector>

// The original STLPort exposed hash_map in std. libstdc++ keeps the compatible
// container in __gnu_cxx; retain the source-facing name until its consumers are
// migrated without changing ordering/hash behavior.
namespace std { using __gnu_cxx::hash_map; }

#ifndef __forceinline
#define __forceinline inline __attribute__((always_inline))
#endif
#ifndef __cdecl
#define __cdecl
#endif
#ifndef stricmp
#define stricmp strcasecmp
#endif
#ifndef strnicmp
#define strnicmp strncasecmp
#endif
#ifndef _stricmp
#define _stricmp strcasecmp
#endif
#ifndef _vsnprintf
#define _vsnprintf vsnprintf
#endif

#include "Lib/BaseType.h"
#ifdef NULL
#undef NULL
#endif
#define NULL 0
#include "Common/Errors.h"
#include "Common/Debug.h"
#include "Common/GameMemory.h"

#if !defined(_WIN32)
using DWORD = std::uint32_t;
using HWND = void*;
using HINSTANCE = void*;

struct SYSTEMTIME {
  std::uint16_t wYear = 0;
  std::uint16_t wMonth = 0;
  std::uint16_t wDayOfWeek = 0;
  std::uint16_t wDay = 0;
  std::uint16_t wHour = 0;
  std::uint16_t wMinute = 0;
  std::uint16_t wSecond = 0;
  std::uint16_t wMilliseconds = 0;
};

class CComModule {
 public:
  void Init(void*, HINSTANCE) noexcept {}
  void Term() noexcept {}
};

inline void timeBeginPeriod(unsigned) noexcept {}
inline void timeEndPeriod(unsigned) noexcept {}
inline DWORD timeGetTime() noexcept
{
  using namespace std::chrono;
  return static_cast<DWORD>(duration_cast<milliseconds>(steady_clock::now().time_since_epoch()).count());
}
inline void Sleep(DWORD milliseconds) noexcept
{
  if (milliseconds) std::this_thread::sleep_for(std::chrono::milliseconds(milliseconds));
  else std::this_thread::yield();
}

inline long InterlockedIncrement(long* value) { return __atomic_add_fetch(value, 1L, __ATOMIC_ACQ_REL); }
inline long InterlockedDecrement(long* value) { return __atomic_sub_fetch(value, 1L, __ATOMIC_ACQ_REL); }

std::size_t wcslen(const WideChar* value) noexcept;
WideChar* wcscpy(WideChar* destination, const WideChar* source) noexcept;
WideChar* wcscat(WideChar* destination, const WideChar* source) noexcept;
int wcscmp(const WideChar* lhs, const WideChar* rhs) noexcept;
int _wcsicmp(const WideChar* lhs, const WideChar* rhs) noexcept;
std::size_t wcsspn(const WideChar* value, const WideChar* accept) noexcept;
std::size_t wcscspn(const WideChar* value, const WideChar* reject) noexcept;
int _vsnwprintf(WideChar* destination, std::size_t capacity, const WideChar* format, va_list args) noexcept;
#endif

#include "Common/AsciiString.h"
#include "Common/UnicodeString.h"
