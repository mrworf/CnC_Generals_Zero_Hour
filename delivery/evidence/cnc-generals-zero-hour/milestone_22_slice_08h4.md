# M22 slice 08H4: authored terrain blend and cliff queries

The canonical `WorldHeightMap` CPU query owner now consumes the accepted
authored metadata and atlas placement. It resolves base quadrant and full-tile
UVs, primary and extra blend alpha orientation, forced triangle flips, custom
edge-class ranges, authored cliff UV remapping and the height-diagonal flip
decision. Every public query validates its cell, class and record boundary
before indexing fixed arrays.

Project-owned coverage supplies nine primary blend records and eight extra
blend references spanning horizontal, vertical, both diagonals, inverted,
long, forced-flip and custom-edge cases. It checks exact alpha/flip tables,
all four base quadrants, full-tile coordinates, edge ranges and authored cliff
coordinates/state. Invalid cells/classes and an unpublished edge atlas fail
closed, and two generations release all resources. No retail input was used.

Acceptance on the final tree:

- Focused query gates passed with GCC, Clang and both sanitizers.
- All six builds and all six canonical non-GPU/non-LAN/non-retail suites pass
  261/261; sanitizer suites used `detect_leaks=0` in the sandbox.
- Strict host LeakSanitizer passed 1/1 with GCC and 1/1 with Clang.
- Physical public Vulkan validation-layer display/factory controls pass 2/2.
  Serial host-loopback LAN passes 4/4 in all six configurations.
- All three dependency-ledger gates and `git diff --check` pass.

Generated authored composition and redacted retail-boundary revalidation
remain the 08H aggregate owner.
