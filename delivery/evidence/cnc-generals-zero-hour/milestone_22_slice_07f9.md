# M22 slice 07F9 evidence: transactional generated-map terrain visual

## Delivered boundary

`W3DTerrainVisual::load` now follows the original owner order for the bounded
generated-map route: `TerrainVisual::load`, `CachedFileInputStream`,
`WorldHeightMap`, existing `HeightMapRenderObjClass::initHeightData`, then
primary `RTS3DScene` attachment.  Missing stream, malformed map and injected
buffer allocation failures reverse the visual filename/map/scene publication.
Owner teardown detaches the terrain, frees its map resources and releases the
logical map before a second generation.

The read-only generated fixture proves the primary attachment, 6144-index
two-pass Recording submission inherited from 07F8, no-frame rejection,
transaction rollback, unload and two in-process reentries. It runs twice in
independent owned source trees. There is no retail input, map-object/light
activation, active shroud/tracks/water/shadows/particles/smudges/tactical
frame, physical pixel, or Vulkan claim.

Texture-atlas creation is lazy in the accepted 07F8 render boundary, not eager
map loading; its injected failure contract remains 07F8 rather than being
misattributed to this slice.

## Validation

- Focused F9 generated-map transaction: GCC Debug, Clang Debug, GCC
  ASan/UBSan/LSan and Clang ASan/UBSan/LSan pass. Sanitizers used
  `ASAN_OPTIONS=detect_leaks=1` and `UBSAN_OPTIONS=print_stacktrace=1`.
- Coupled F8/F8A/base-terrain controls: GCC Debug and Clang Debug, 4/4 each.
- Exact non-GPU/non-LAN suites: 199/199 each in GCC Debug, GCC Release, Clang
  Release, GCC ASan/UBSan/LSan and Clang ASan/UBSan/LSan.
- Dependency ledger, presentation identity and provider-removal checks pass.
- Serial local-loopback LAN suites: 4/4 each in GCC Debug, GCC Release, Clang
  Release, GCC ASan/UBSan/LSan and Clang ASan/UBSan/LSan.

## Remaining route

07FA–07FE remain independent inactive-effect branches, joined only by 07FF;
07FG remains the production map-factory gate before read-only retail recording
in 08. This slice does not close any of those obligations.
