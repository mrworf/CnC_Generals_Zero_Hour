#include "PreRTS.h"

#include <cctype>
#include <cstdarg>
#include <cstdio>

namespace {

bool contains(const WideChar* set, WideChar value) noexcept
{
  for (; *set; ++set)
    if (*set == value) return true;
  return false;
}

bool append(WideChar*& out, std::size_t& left, const WideChar* text) noexcept
{
  while (*text) {
    if (left <= 1) return false;
    *out++ = *text++;
    --left;
  }
  return true;
}

bool append_ascii(WideChar*& out, std::size_t& left, const char* text) noexcept
{
  while (*text) {
    if (left <= 1) return false;
    *out++ = static_cast<unsigned char>(*text++);
    --left;
  }
  return true;
}

}  // namespace

std::size_t wcslen(const WideChar* value) noexcept
{
  const WideChar* end = value;
  while (*end) ++end;
  return static_cast<std::size_t>(end - value);
}

WideChar* wcscpy(WideChar* destination, const WideChar* source) noexcept
{
  WideChar* result = destination;
  while ((*destination++ = *source++)) {}
  return result;
}

WideChar* wcscat(WideChar* destination, const WideChar* source) noexcept
{
  wcscpy(destination + wcslen(destination), source);
  return destination;
}

int wcscmp(const WideChar* lhs, const WideChar* rhs) noexcept
{
  while (*lhs && *lhs == *rhs) { ++lhs; ++rhs; }
  return static_cast<int>(*lhs) - static_cast<int>(*rhs);
}

int _wcsicmp(const WideChar* lhs, const WideChar* rhs) noexcept
{
  while (*lhs && *rhs) {
    const auto l = static_cast<WideChar>(std::tolower(static_cast<unsigned char>(*lhs)));
    const auto r = static_cast<WideChar>(std::tolower(static_cast<unsigned char>(*rhs)));
    if (l != r) return static_cast<int>(l) - static_cast<int>(r);
    ++lhs; ++rhs;
  }
  return static_cast<int>(*lhs) - static_cast<int>(*rhs);
}

std::size_t wcsspn(const WideChar* value, const WideChar* accept) noexcept
{
  const WideChar* start = value;
  while (*value && contains(accept, *value)) ++value;
  return static_cast<std::size_t>(value - start);
}

std::size_t wcscspn(const WideChar* value, const WideChar* reject) noexcept
{
  const WideChar* start = value;
  while (*value && !contains(reject, *value)) ++value;
  return static_cast<std::size_t>(value - start);
}

int _vsnwprintf(WideChar* destination, std::size_t capacity, const WideChar* format, va_list args) noexcept
{
  if (!capacity) return -1;
  WideChar* out = destination;
  std::size_t left = capacity;
  for (const WideChar* p = format; *p; ++p) {
    if (*p != u'%') {
      if (left <= 1) return -1;
      *out++ = *p;
      --left;
      continue;
    }
    ++p;
    if (*p == u'%') {
      if (left <= 1) return -1;
      *out++ = u'%';
      --left;
    } else if (*p == u'h' && p[1] == u's') {
      ++p;
      const char* value = va_arg(args, const char*);
      if (!append_ascii(out, left, value ? value : "(null)")) return -1;
    } else if (*p == u'l' && p[1] == u's') {
      ++p;
      const WideChar* value = va_arg(args, const WideChar*);
      if (!append(out, left, value ? value : u"(null)")) return -1;
    } else if (*p == u'S') {
      const char* value = va_arg(args, const char*);
      if (!append_ascii(out, left, value ? value : "(null)")) return -1;
    } else if (*p == u's') {
      if (!append(out, left, va_arg(args, const WideChar*))) return -1;
    } else if (*p == u'c') {
      if (left <= 1) return -1;
      *out++ = static_cast<WideChar>(va_arg(args, int));
      --left;
    } else if (*p == u'd' || *p == u'u') {
      char number[32];
      if (*p == u'd') std::snprintf(number, sizeof(number), "%d", va_arg(args, int));
      else std::snprintf(number, sizeof(number), "%u", va_arg(args, unsigned));
      if (!append_ascii(out, left, number)) return -1;
    } else {
      return -1;
    }
  }
  *out = 0;
  return static_cast<int>(out - destination);
}
