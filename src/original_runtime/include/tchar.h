#pragma once

#include <cstddef>
#include <cstring>
#include <cwchar>
#include <cstdlib>
#include <cstdio>
#include <strings.h>

using TCHAR = char;
using WCHAR = wchar_t;
using HANDLE = void *;
using HFONT = void *;
using HBITMAP = void *;
using HDC = void *;

#ifndef _cdecl
#define _cdecl
#endif
#ifndef stricmp
inline int stricmp(const char *lhs, const char *rhs) { return ::strcasecmp(lhs, rhs); }
#endif
#ifndef strnicmp
inline int strnicmp(const char *lhs, const char *rhs, std::size_t count) { return ::strncasecmp(lhs, rhs, count); }
#endif
inline int lstrcmpi(const char *lhs, const char *rhs) { return ::strcasecmp(lhs, rhs); }
inline int _wcsicmp(const wchar_t *lhs, const wchar_t *rhs) { return ::wcscasecmp(lhs, rhs); }
inline char *_strlwr(char *value)
{
	for (char *p = value; *p != '\0'; ++p) {
		if (*p >= 'A' && *p <= 'Z') *p = char(*p - 'A' + 'a');
	}
	return value;
}
inline char *_strdup(const char *value) { return ::strdup(value); }
inline char *lstrcpyn(char *dst, const char *src, int count)
{
	if (count <= 0) return dst;
	std::strncpy(dst, src, static_cast<std::size_t>(count - 1));
	dst[count - 1] = '\0';
	return dst;
}
inline char *lstrcat(char *dst, const char *src) { return std::strcat(dst, src); }
inline char *lstrcpy(char *dst, const char *src) { return std::strcpy(dst, src); }
inline int lstrlen(const char *value) { return static_cast<int>(std::strlen(value)); }
inline long _lrotl(long value, int shift)
{
	const unsigned bits = sizeof(unsigned long) * 8U;
	const unsigned amount = static_cast<unsigned>(shift) % bits;
	const unsigned long v = static_cast<unsigned long>(value);
	return static_cast<long>((v << amount) | (v >> ((bits - amount) % bits)));
}

#ifndef MAX_PATH
#define MAX_PATH 260
#endif

#define _T(value) value
#define _tcslen std::strlen
#define _tcsclen std::strlen
#define _tcscpy std::strcpy
#define _tcscmp std::strcmp
#define _tcsicmp ::strcasecmp
#define _vsnprintf std::vsnprintf
