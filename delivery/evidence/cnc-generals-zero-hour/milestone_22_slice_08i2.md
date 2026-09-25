# M22 slice 08I2: edge-aware multi-tile terrain submission

`HeightMapRenderObjClass::Render` now preflights the complete vertex-buffer and
backup grid plus exact edge metadata before source draw state changes. Each of
the two accepted base passes traverses tiles in row-major order. Full-width
tiles submit one exact height-bounded range; partial-width tiles submit only
their valid shared-index rows, excluding all fixed-capacity padding. The
accepted extra-blend and terrain-track consumers remain after the base shader
reset.

Generated coverage renders partial 1x1, X-only 2x1, Y-only 1x2, exact 2x2 and
partial 2x2 maps. It proves exact draw/range counts, first/last shared-index
ranges, base-before-extra ordering, incomplete owner and invalid edge-metadata
rejection, first-tile physical bind failure, draw failure at both boundaries
of both passes, deterministic retry, two process generations and zero retained
resources. No retail input was used.

Acceptance on the final slice tree:

- Focused and related authored-terrain gates pass with GCC and Clang.
- Focused ASan+UBSan passes with `detect_leaks=0`; strict host LeakSanitizer
  passes with GCC and Clang.
- All six canonical configurations build and their non-GPU/non-LAN/non-retail
  suites pass 261/261; sanitizer suites use `detect_leaks=0` in the sandbox.
- Physical public Vulkan controls pass 2/2; serial host LAN passes 4/4 in all
  six configurations.
- All three dependency-ledger checks and `git diff --check` pass.
