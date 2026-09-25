# M22 slice 08H2: authored terrain source tile sets

The canonical `WorldHeightMap` terrain image owner now decodes a texture
class's complete square base or edge tile set. It accepts bounded 24/32-bit
true-color TGA in uncompressed or RLE form, skips the authored identifier,
normalizes both origin flags into canonical tile order, generates every
`TileData` mip and publishes the class only after decoding succeeds. Partial
classes are released on every failure, and later map-construction failure
releases all previously published class tiles.

Project-owned coverage supplies an oriented four-tile base class, an RLE
single-tile base class and a 24-bit edge class. Exact per-tile colors persist
through all seven mip levels over two in-process and two fresh-process
generations. Missing base/edge images, class/image dimension mismatch,
truncation and incomplete RLE packets fail closed. The prior one-tile fixture
and its malformed controls remain covered. No retail input was used.

Acceptance on the final tree:

- Focused source tile-set gates passed with GCC, Clang and both sanitizers.
- All six builds and all six canonical non-GPU/non-LAN/non-retail suites pass
  261/261; sanitizer suites used `detect_leaks=0` in the sandbox.
- Strict host LeakSanitizer passed 1/1 with GCC and 1/1 with Clang.
- Physical public Vulkan validation-layer display/factory controls pass 2/2.
  Serial host-loopback LAN passes 4/4 in all six configurations.
- All three dependency-ledger gates and `git diff --check` pass.

General base/edge atlas placement remains 08H3; active blend/cliff query
semantics remain 08H4.
