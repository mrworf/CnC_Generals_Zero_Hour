# M22 08E1C0A1: generated GameInfo/GameSlot lifecycle evidence

The project-owned probe executes the original `GameInfo`/`GameSlot` source
with caller-owned slots. It publishes those slots before `setSlot`, proves a
human local-IP slot and an AI slot, rejects invalid indexes, clears both slots,
and unpublishes GameInfo/GameText/global data across two generations. It does
not claim template/settings parsing, map-cache selection, multiplayer layout,
retail data, pixels, media, or network service behavior.

- Focused runtime plus GameInfo.cpp provider-removal controls: 2/2 in GCC and
  Clang Debug, Release, and canonical sanitizer configurations.
- Registration reconciliation: non-sanitizer configurations register 244 raw
  tests; Clang sanitizer registers 265 because its additional 21 tests are
  GPU-labelled physical coverage. The exact portable `-LE gpu|lan` set is
  231 tests in all six configurations.
- Fresh portable suites: 231/231 in GCC Debug, GCC Release, Clang Debug,
  Clang Release, GCC sanitizer, and Clang sanitizer. The canonical sanitizer
  wrapper uses `ASAN_OPTIONS=detect_leaks=0` due its established ptrace limit.
- Strict host `detect_leaks=1` focus: 2/2 in GCC sanitizer and 2/2 in Clang
  sanitizer.
- Physical Vulkan validation: 9/9. Serial LAN: 4/4 in each of GCC Debug, GCC
  Release, Clang Debug, Clang Release, GCC sanitizer, and Clang sanitizer.
