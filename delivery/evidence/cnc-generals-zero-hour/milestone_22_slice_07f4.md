# M22 slice 07F4 evidence: original flat terrain geometry ownership

## Source and ownership result

- Canonical Linux `HeightMapRenderObjClass` now accepts the generated F3
  flat/unblended visual map under production-compatible display and Recording
  edge owners, while the source default extra-pass request resolves to an empty
  list only after every cell proves unblended.
- The derived owner creates the fixed source 6,144-index topology and one
  source-capacity 4,096-vertex tile with a matching CPU backup. Exactly 7x7
  cells (196 vertices) are populated for the 8x8 fixture; the remainder is
  padded capacity, not claimed map geometry.
- Populated vertices preserve source map-space XY, scaled height Z and the F3
  missing-bitmap zero UV/alpha boundary. A bounded one-cell height change
  re-uploads the existing vertex resource without changing buffer count.
- Explicit free releases the map ref and source buffers; edge cache retirement
  returns both Recording buffers. A second init and two fresh processes repeat
  the same ownership lifecycle with zero retained resources.

## Negative and isolation controls

- Null/mismatched/unbound and active visual metadata remain rejected by the F2
  and F3 contracts. Duplicate init and an invalid update rectangle reject
  without changing the valid owner.
- Injected first buffer creation failure rolls back base map/shroud and derived
  CPU buffers, restores the map ref, and publishes no Recording resource.
- Recording sees only one index and one vertex buffer plus uploads; it records
  no draw. Terrain atlas/bitmap, material/shader, `Render`,
  `W3DTerrainVisual::load`, active effects, projection and pixels remain pending.
- The generated map is temporary and read-only; private paths are redacted.
  Retail inputs and retail symlink content are untouched.

## Acceptance gates

- Focused `original_w3d_flat_terrain_geometry`: GCC Debug 1/1. Clang Release
  geometry/visual/ledger/presentation controls: 6/6.
- Exact-tree non-LAN: GCC Debug 195/195, GCC Release 195/195, Clang Release
  195/195, GCC ASan/UBSan/LSan 195/195 and Clang ASan/UBSan/LSan 195/195.
  Leak detection remains enabled in both sanitizer presets.
- LAN label under established local-socket permission: 4/4 in all five
  configurations.
- All dependency-ledger checks and `git diff --check` pass.
