# M22 08E1C0A2: parser-backed multiplayer provider evidence

The generated project-owned fixture uses the original `INI::load` dispatch for
`PlayerTemplate`, `MultiplayerSettings`, `MultiplayerColor`, and
`MultiplayerStartingMoneyChoice`. It publishes a caller-owned template store,
lets the source settings parser allocate its singleton, proves valid fields and
same-name updates, rejects malformed template fields and missing/out-of-range
lookups, then releases both providers across two generations. No persona, map
cache, layout owner, retail data, media, pixels, or network service claim is
made.

- Focused runtime plus separate PlayerTemplate.cpp and INIMultiplayer.cpp
  provider-removal controls: 3/3 in GCC and Clang Debug, Release, and canonical
  sanitizer configurations.
- Registration reconciliation: five configurations register 247 raw tests;
  Clang sanitizer registers 268 because of its additional 21 GPU-labelled
  physical tests. The exact portable `-LE gpu|lan` set is 234 in all six.
- Fresh portable suites: 234/234 in GCC Debug, GCC Release, Clang Debug,
  Clang Release, GCC sanitizer, and Clang sanitizer. Canonical sanitizer suite
  runs use `ASAN_OPTIONS=detect_leaks=0` for the established sandbox ptrace
  limitation.
- Strict host `detect_leaks=1` focus: 3/3 in GCC sanitizer and 3/3 in Clang
  sanitizer. Physical Vulkan: 9/9. Serial LAN: 4/4 in each of the six builds.
