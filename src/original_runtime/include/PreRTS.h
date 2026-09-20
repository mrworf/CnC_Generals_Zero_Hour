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
#include <ctime>
#include <cwctype>
#include <deque>
#include <fstream>
#include <functional>
#include <filesystem>
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
#ifndef _cdecl
#define _cdecl
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
#include <fcntl.h>
#include <arpa/inet.h>
#include <netdb.h>
#ifdef AI_PASSIVE
#undef AI_PASSIVE
#endif
#include <sys/stat.h>
#include <sys/sysinfo.h>
#include <unistd.h>

#define _isnan std::isnan

using DWORD = std::uint32_t;
using WORD = std::uint16_t;
using HWND = void*;
using HINSTANCE = void*;
using HMODULE = void*;
using FARPROC = void (*)();
using HRESULT = long;
constexpr HRESULT S_OK = 0;
constexpr unsigned SEVERITY_ERROR = 1;
constexpr unsigned FACILITY_ITF = 4;
constexpr HRESULT MAKE_HRESULT(unsigned severity, unsigned facility, unsigned code)
{
  return static_cast<HRESULT>((severity << 31) | (facility << 16) | code);
}
constexpr unsigned VK_RETURN = 0x0d;
constexpr std::size_t _MAX_PATH = 4096;

struct WSADATA { WORD wVersion = 0; };
using HOSTENT = struct hostent;
constexpr WORD MAKEWORD(unsigned low, unsigned high) { return static_cast<WORD>((high << 8) | low); }
constexpr unsigned LOBYTE(WORD value) { return value & 0xffU; }
constexpr unsigned HIBYTE(WORD value) { return (value >> 8) & 0xffU; }
inline int WSAStartup(WORD version, WSADATA *data) { if (data) data->wVersion = version; return 0; }
inline int WSACleanup() { return 0; }
inline int WSAGetLastError() { return errno; }
inline char *itoa(int value, char *buffer, int radix)
{
  if (radix == 16) std::snprintf(buffer, 33, "%x", static_cast<unsigned>(value));
  else if (radix == 8) std::snprintf(buffer, 33, "%o", static_cast<unsigned>(value));
  else std::snprintf(buffer, 33, "%d", value);
  return buffer;
}
inline DWORD GetTickCount()
{
  return static_cast<DWORD>(std::chrono::duration_cast<std::chrono::milliseconds>(
      std::chrono::steady_clock::now().time_since_epoch()).count());
}
inline char16_t *wcsncpy(char16_t *destination, const char16_t *source, std::size_t count)
{
  std::size_t index = 0;
  for (; index < count && source[index] != 0; ++index) destination[index] = source[index];
  for (; index < count; ++index) destination[index] = 0;
  return destination;
}

struct MEMORYSTATUS {
  DWORD dwLength = sizeof(MEMORYSTATUS);
  DWORD dwMemoryLoad = 0;
  DWORD dwTotalPhys = 0;
  DWORD dwAvailPhys = 0;
  DWORD dwTotalPageFile = 0;
  DWORD dwAvailPageFile = 0;
  DWORD dwTotalVirtual = 0;
  DWORD dwAvailVirtual = 0;
};

inline void GlobalMemoryStatus(MEMORYSTATUS *status)
{
  struct sysinfo info {};
  if (!status || ::sysinfo(&info) != 0) return;
  const auto scaled = [&info](unsigned long value) {
    return static_cast<DWORD>(value * info.mem_unit);
  };
  status->dwTotalPhys = scaled(info.totalram);
  status->dwAvailPhys = scaled(info.freeram);
  status->dwTotalPageFile = scaled(info.totalswap);
  status->dwAvailPageFile = scaled(info.freeswap);
  status->dwTotalVirtual = status->dwTotalPhys + status->dwTotalPageFile;
  status->dwAvailVirtual = status->dwAvailPhys + status->dwAvailPageFile;
  status->dwMemoryLoad = status->dwTotalPhys == 0 ? 0 :
      100 - static_cast<DWORD>((100ULL * status->dwAvailPhys) / status->dwTotalPhys);
}

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

inline int _access(const char* path, int mode) { return ::access(path, mode); }
inline int CreateDirectory(const char* path, void*)
{
  std::string native(path ? path : "");
  std::replace(native.begin(), native.end(), '\\', '/');
  std::error_code error;
  return std::filesystem::create_directories(native, error) || (!error && std::filesystem::is_directory(native));
}
inline void GetLocalTime(SYSTEMTIME* result)
{
  const auto now = std::chrono::system_clock::now();
  const std::time_t value = std::chrono::system_clock::to_time_t(now);
  std::tm local{};
  localtime_r(&value, &local);
  result->wYear = static_cast<std::uint16_t>(local.tm_year + 1900);
  result->wMonth = static_cast<std::uint16_t>(local.tm_mon + 1);
  result->wDay = static_cast<std::uint16_t>(local.tm_mday);
  result->wDayOfWeek = static_cast<std::uint16_t>(local.tm_wday);
  result->wHour = static_cast<std::uint16_t>(local.tm_hour);
  result->wMinute = static_cast<std::uint16_t>(local.tm_min);
  result->wSecond = static_cast<std::uint16_t>(local.tm_sec);
}
inline HMODULE LoadLibrary(const char*) noexcept { return nullptr; }
inline FARPROC GetProcAddress(HMODULE, const char*) noexcept { return nullptr; }
inline int FreeLibrary(HMODULE) noexcept { return 1; }

#define _open ::open
#define _close ::close
#define _read ::read
#define _write ::write
#define _lseek ::lseek
#define _O_CREAT O_CREAT
#define _O_TRUNC O_TRUNC
#define _O_APPEND O_APPEND
#define _O_TEXT 0
#define _O_BINARY 0
#define _O_RDWR O_RDWR
#define _O_WRONLY O_WRONLY
#define _O_RDONLY O_RDONLY
#define _S_IREAD S_IRUSR
#define _S_IWRITE S_IWUSR

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
