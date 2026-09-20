#include "PreRTS.h"

extern "C" AsciiString* m26_make_ascii_string()
{
  return new AsciiString("provider-owned");
}

extern "C" UnicodeString* m26_make_unicode_string()
{
  return new UnicodeString(u"provider-\U0001f680");
}
