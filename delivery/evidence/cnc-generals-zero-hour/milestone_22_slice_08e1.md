# M22 08E1: generated mode-exclusive load-screen aggregate evidence

This test-only aggregate joins the accepted 08E1A presentation spine, 08E1B
`SinglePlayerLoadScreen` owner, and 08E1C `MultiPlayerLoadScreen` owner. It
uses their existing project-owned WND, INI, mapped-image, mission, slot, and
map-preview fixtures; it does not read retail content or claim layout, pixel,
audio, network, or gameplay fidelity.

The source-backed owner target completes two single-player generations first.
Before constructing multiplayer state it requires the shared `WindowManager`
tree and `DisplayStringManager` baseline to be empty, Campaign/GameInfo/
template/settings/map/GameState globals to be unpublished, and the base
mapped-image collection to be restored. It then completes two multiplayer
generations and emits a terminal marker only after final source cleanup. The
new aggregate CTest rejects missing owner files, nonzero execution, missing
markers, and any ordering other than single-player before multiplayer.

Acceptance after the final tree was stable:

- focused GCC and Clang aggregate/owner/provider-removal/identity/ledger
  checks: 11/11 in each compiler;
- fresh exact non-GPU/non-LAN registration: 256 tests in each build. The full
  suites passed GCC Debug 256/256 (218.63s), GCC Release 256/256 (127.75s),
  Clang Debug 256/256 (202.17s), Clang Release 256/256 (77.98s), GCC
  ASAN/UBSAN 256/256 (569.81s), and Clang ASAN/UBSAN 256/256 (443.90s);
  the canonical sandbox sanitizer wrappers use `detect_leaks=0` only for the
  known ptrace limitation;
- strict host `detect_leaks=1` focus passed with no leak report: GCC 20/20
  (88.46s) and Clang 20/20 (64.26s), both including the aggregate;
- proportional physical Vulkan validation passed 9/9; serial local-socket LAN
  passed 4/4 in GCC Debug, GCC Release, Clang Debug, Clang Release, GCC
  sanitizer, and Clang sanitizer.

The aggregate adds no production dispatch. 08E remains responsible for source
GameLogic dispatch and later retail/redacted advancement.
