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
#ifndef _vsnprintf
#define _vsnprintf vsnprintf
#endif

#include "Lib/BaseType.h"
#include "Common/Errors.h"
#include "Common/Debug.h"
#include "Common/GameMemory.h"
