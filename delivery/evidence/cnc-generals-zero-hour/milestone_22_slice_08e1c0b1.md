# M22 08E1C0B1: persona and MapCache metadata provider evidence

The project-owned generated INI fixture invokes original `INI::load` dispatch
to populate a published `ChallengeGenerals` and `MapCache` pair. It proves
campaign, general-name and player-template lookup, canonicalized map metadata
and start-waypoint lookup, same-name replacement, missing/malformed rejection,
and zero stale global publication over two generations. Separate archive
removal controls prove the `ChallengeGenerals.cpp` and `INIMapCache.cpp`
providers are required. No retail persona, audio/image content, map preview,
live GUI start-marker, layout owner, network service, or pixel claim is made.

- Focused runtime/removal plus configuration-identity and dependency-ledger
  gates: 7/7 in GCC and Clang Debug/Release/canonical sanitizer configurations.
- Registration reconciliation: the exact portable `-LE gpu|lan` set is 237
  tests in every configuration. Raw registration is 250 in the five normal
  and GCC-sanitized configurations and 271 in Clang sanitizer because its
  additional 21 physical tests are GPU-labelled.
- Fresh portable suites: 237/237 in GCC Debug, GCC Release, Clang Debug,
  Clang Release, GCC sanitizer, and Clang sanitizer. Canonical sanitizer
  suites use `ASAN_OPTIONS=detect_leaks=0` because of the established sandbox
  ptrace limitation.
- Strict host `detect_leaks=1`: GCC C0A1/A2 focused providers 2/2 plus C0B1
  runtime/removal 3/3; Clang coupled C0A1/A2/C0B1 focus 5/5. No leak report.
- Physical Vulkan validation: 9/9. Serial local-socket LAN: 4/4 in each of
  GCC Debug, GCC Release, Clang Debug, Clang Release, GCC sanitizer, and
  Clang sanitizer.
