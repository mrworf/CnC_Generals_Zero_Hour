# M22 slice 07F5 evidence: original terrain source bitmap ownership

## Source and ownership result

- Canonical Linux `WorldHeightMap::readTexClass` resolves the generated `Flat`
  class through the original `TerrainTypeCollection`, opens the generated
  read-only `Art/Terrain/Flat.tga` through `TheFileSystem`, and accepts exactly
  one uncompressed 64x64 32-bit source tile.
- Canonical `TileData` is compiled with its required `PreRTS.h` first include,
  owns the decoded bytes, and generates the 64/32/16/8/4/2/1 mip chain. The
  focused probe witnesses deterministic BGRA bytes at every width.
- Two owners in each process and two fresh processes release the map-held tile
  before filesystem/service teardown and return to the allocation baseline.

## Negative and dependency controls

- Missing, compressed, 24-bit, wrong-dimension, oversized and truncated images
  fail before publishing a source tile. A duplicate authored `Flat` name keeps
  the original terrain-definition replacement behavior and remains usable.
- The accepted F3 metadata-only fixture now uses an explicitly unresolved
  four-byte terrain class. This removes an accidental dependency on baseline
  `Terrain.ini` while preserving its native no-bitmap zero-UV/alpha boundary.
  Its truncation fixture now removes twelve bytes so the parser itself, rather
  than an unrelated missing texture file, witnesses the failure. The F3 visual
  and F4 flat-geometry focused regressions pass beside F5.
- The original presentation and W3D provider-removal checks pass after both
  consumers of the shared full-W3D object target regenerate their link maps.
  The initial stale secondary map was diagnostic only and is not acceptance
  evidence.
- Atlas `TextureClass` resources, terrain materials/shaders, draw submission,
  `W3DTerrainVisual::load`, active effects and pixels remain pending. Generated
  inputs are temporary and read-only; retail inputs and symlink content remain
  untouched.

## Acceptance gates

- Focused GCC Debug F3/F4/F5 regressions: 3/3. Clang Debug F5, dependency
  ledgers and regenerated presentation/W3D identity/provider controls: 7/7.
- Exact-tree non-LAN: GCC Debug 196/196, GCC Release 196/196, Clang Release
  196/196, GCC ASan/UBSan/LSan 196/196 and Clang ASan/UBSan/LSan 196/196.
  Leak detection remains enabled in both sanitizer presets.
- LAN label under the established local-socket permission: 4/4 in all five
  configurations.
- All dependency-ledger checks and `git diff --check` pass.
