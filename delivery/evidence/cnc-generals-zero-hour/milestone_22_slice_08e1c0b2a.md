# M22 08E1C0B2A: map-preview resource provider evidence

A project-owned generated map TGA fixture drives the original
`getMapPreviewImage` path through the published GlobalData, GameState,
MapCache, FileSystem/archive fallback and MappedImageCollection owners. It
proves source-exact portable-name publication, descriptor dimensions, reuse,
missing-input rejection, a cached-existence source-copy failure with no
retained image, retry after restoration, and two clean ownership generations.
Separate archive-removal controls prove MapUtil, GameState, and GlobalData are
required. No retail image, pixel-fidelity, GUI start-marker, layout owner, or
network-service claim is made.

- Focused GCC preview/removal checks: 4/4. Focused Clang coupled
  preview/mapped-image/persona checks: 7/7.
- Registration reconciliation: the exact portable `-LE gpu|lan` set is 241
  tests in every configuration. Clang sanitizer rebuilt after its Ninja
  recovery warning and also registered 241 before execution.
- Fresh portable suites: 241/241 in GCC Debug, GCC Release, Clang Debug,
  Clang Release, GCC sanitizer, and Clang sanitizer. Canonical sanitizer
  suites use `ASAN_OPTIONS=detect_leaks=0` due to the established sandbox
  ptrace limitation.
- Strict host `detect_leaks=1`: GCC coupled preview focus 4/4; Clang coupled
  preview focus 4/4. No leak report.
- Physical Vulkan validation: 9/9. Serial local-socket LAN: 4/4 in each of
  GCC Debug, GCC Release, Clang Debug, Clang Release, GCC sanitizer, and
  Clang sanitizer.
