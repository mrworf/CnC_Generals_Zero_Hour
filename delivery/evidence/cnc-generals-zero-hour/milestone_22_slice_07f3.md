# M22 slice 07F3 evidence: original visual terrain tile metadata

## Source and ownership result

- The canonical Linux `WorldHeightMap` visual constructor parses matching v4
  `HeightMapData` and bounded v8 `BlendTileData`, owns the source tile, blend,
  extra-blend, cliff-index, flip-bit and cliff-bit arrays, and publishes only
  after both chunks validate.
- A generated 8x8 flat/unblended map resolves every cell to its single authored
  texture class and reports false flip, cliff and cliff-mapped state through
  the original query surface.
- No texture bitmap is loaded in this child. Consequently zero UV and alpha is
  the source missing-texture result for this flat/unblended fixture only; it is
  not claimed as general visual-map behavior. Texture file, atlas and GPU
  resource calls remain explicit failures.
- Two in-process generations and two fresh processes retire every owned array;
  the existing F0 logical-only constructor still accepts the same height chunk
  while retaining its independent visual-construction negative control.

## Negative and isolation controls

- Missing and duplicate visual chunks, truncation, height/tile shape mismatch,
  excessive class counts, invalid active blend metadata and active cliff bits
  reject before a usable map is returned.
- Out-of-range alpha/UV and texture-class queries reject or return their bounded
  source sentinel without reading outside the arrays.
- The generated inputs live under a temporary read-only root and failures
  redact private paths. Retail inputs and retail symlink content are untouched.
- Derived vertex/index buffers, `W3DTerrainVisual::load`, atlas resources,
  active shroud projection and terrain pixels remain pending.

## Acceptance gates

- Focused `original_w3d_visual_height_map`: GCC Debug 1/1. Clang Release visual,
  logical/base-map, ledger and presentation controls: 6/6.
- Exact-tree non-LAN: GCC Debug 194/194, GCC Release 194/194, Clang Release
  194/194, GCC ASan/UBSan/LSan 194/194 and Clang ASan/UBSan/LSan 194/194.
  Leak detection remains enabled in both sanitizer presets.
- LAN label under established local-socket permission: 4/4 in all five
  configurations.
- All three dependency-ledger checks and `git diff --check` pass.
