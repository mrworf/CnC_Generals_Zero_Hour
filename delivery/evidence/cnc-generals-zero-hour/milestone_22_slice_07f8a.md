# M22 slice 07F8A evidence: terrain scene-registration prerequisite

## Delivered boundary

This slice closes the reusable terrain-owner prerequisite found while starting
07F9. The CPU original `BaseHeightMapRenderObjClass` now derives its native
map bounds and performs source-equivalent `SceneClass` update registration on
attach, with symmetric unregistration before detach. The CPU-only declaration
keeps the native build declaration set and class layout unchanged.

The generated probe covers a bound 8x8 map, actual original `RTS3DScene`
attach/detach, duplicate and mismatched add/remove negatives, counted
`ON_FRAME_UPDATE` registration/unregistration in a source-compatible scene,
second attach/detach, two independent generations, zero retained Recording
resources, and no draw record. It does not load retail data, render a
map-frame, or claim physical pixels.

## Validation

- Focused scene-registration test: GCC Debug and Clang Debug, 1/1 each.
- Coupled base-terrain/F8/F8A controls: GCC Debug and Clang Debug, 3/3 each.
- Presentation identity, provider removal, dependency-ledger validation and
  `git diff --check`: pass.
- Exact non-GPU/non-LAN trees: GCC Debug, GCC Release, Clang Release, GCC
  ASan/UBSan/LSan and Clang ASan/UBSan/LSan, 198/198 each. Sanitizers used
  `ASAN_OPTIONS=detect_leaks=1` and `UBSAN_OPTIONS=print_stacktrace=1`.
  The direct Clang CTest child launcher first reported a host permission error
  before test code ran; the same suite passed through `cmake -E env`, and the
  focused scripts also pass when invoked directly under those same settings.
- Serial local-loopback LAN suites: 4/4 each in GCC Debug, GCC Release, Clang
  Release, GCC sanitizer and Clang sanitizer.

## Remaining route

07F9 remains pending and now depends on this accepted prerequisite. Its scope
is only transactional generated `W3DTerrainVisual::load`; shroud/view,
tracks, water, shadows, particles/smudges, the integration join and production
factory publication remain separately ordered 07 children before retail 08.
