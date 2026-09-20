#pragma once

#include <algorithm>
#include <atomic>
#include <cassert>
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
#include <mutex>
#include <new>
#include <strings.h>
#include <utility>

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
#include "Common/Errors.h"
#include "Common/Debug.h"
#include "Common/GameMemory.h"

#if !defined(_WIN32)
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
