#include "PreRTS.h"
#include "Common/version.h"
#include "Common/STLTypedefs.h"
#include "zh/original_process.h"

#include <cstdio>
#include <cstring>

extern "C" AsciiString* m26_make_ascii_string();
extern "C" UnicodeString* m26_make_unicode_string();

namespace {

int fail(const char* message)
{
  std::fprintf(stderr, "m26 string/service failure: %s\n", message);
  return 1;
}

bool all_services_released()
{
  const auto counts = zh::original_process::service_counts();
  return counts.synchronization == 0 && counts.logging == 0 &&
         counts.version == 0 && counts.workers == 0 && !TheVersion;
}

}  // namespace

int main()
{
  static_assert(sizeof(WideChar) == 2, "original Unicode ABI must use UTF-16 code units");
  char diagnostic[96]{};

  for (int failure_stage = 1; failure_stage <= 3; ++failure_stage) {
    if (zh::original_process::initialize_services(failure_stage, diagnostic, sizeof(diagnostic)))
      return fail("injected construction failure unexpectedly succeeded");
    if (!all_services_released())
      return fail("construction failure did not unwind every initialized service");
    zh::original_process::shutdown_services();
    if (!all_services_released())
      return fail("repeated shutdown was not idempotent");
  }

  if (!zh::original_process::initialize_services(0, diagnostic, sizeof(diagnostic)))
    return fail(diagnostic);
  const auto live = zh::original_process::service_counts();
  if (live.synchronization != 4 || live.logging != 1 || live.version != 1 || live.workers != 0)
    return fail("unexpected live process-service counts");

  AsciiString ascii("alpha");
  AsciiString ascii_copy(ascii);
  ascii_copy.concat("-beta");
  if (std::strcmp(ascii.str(), "alpha") || std::strcmp(ascii_copy.str(), "alpha-beta"))
    return fail("ASCII copy-on-write semantics changed");
  AsciiString independently_allocated("alpha");
  if (rts::hash<AsciiString>{}(ascii) != rts::hash<AsciiString>{}(independently_allocated))
    return fail("equal ASCII values did not produce an identity-stable content hash");
  ascii_copy.format("%s:%d", "value", 7);
  if (std::strcmp(ascii_copy.str(), "value:7"))
    return fail("ASCII formatting failed");

  UnicodeString unicode(u"A\U0001f680Z");
  if (unicode.getLength() != 4 || unicode.getCharAt(1) != 0xd83d || unicode.getCharAt(2) != 0xde80)
    return fail("UTF-16 surrogate pair was not preserved");
  UnicodeString unicode_copy(unicode);
  unicode_copy.concat(u"!");
  if (unicode.getLength() != 4 || unicode_copy.getLength() != 5)
    return fail("Unicode copy-on-write semantics changed");
  UnicodeString formatted;
  formatted.format(u"%s:%d", u"wide", 9);
  if (formatted.compare(u"wide:9") != 0)
    return fail("bounded UTF-16 formatting failed");

  AsciiString* provider_ascii = m26_make_ascii_string();
  UnicodeString* provider_unicode = m26_make_unicode_string();
  if (std::strcmp(provider_ascii->str(), "provider-owned") || provider_unicode->getLength() != 11)
    return fail("cross-target original string construction failed");
  delete provider_ascii;
  delete provider_unicode;

  TheVersion->setVersion(1, 4, 2, 0, "builder", "linux", "12:00", "2026-09-20");
  if (TheVersion->getVersionNumber() != 0x00010004U ||
      std::strcmp(TheVersion->getAsciiVersion().str(), "1.4"))
    return fail("original Version behavior changed");
  if (TheVersion->getUnicodeVersion().compare(u"1.4") != 0)
    return fail("original Version UTF-16 adapter failed");

  static WideChar oversized[UnicodeString::MAX_LEN + 2];
  for (auto& unit : oversized) unit = u'x';
  oversized[UnicodeString::MAX_LEN + 1] = 0;
  bool rejected = false;
  try { UnicodeString too_long(oversized); }
  catch (...) { rejected = true; }
  if (!rejected) return fail("oversized UnicodeString was not rejected");

  zh::original_process::shutdown_services();
  if (!all_services_released())
    return fail("normal shutdown did not release services exactly once");
  zh::original_process::shutdown_services();
  if (!all_services_released())
    return fail("second normal shutdown changed released state");

  std::printf("original-process strings: ok providers=AsciiString.cpp,UnicodeString.cpp,version.cpp utf16=2 services=0 workers=0\n");
  return 0;
}
