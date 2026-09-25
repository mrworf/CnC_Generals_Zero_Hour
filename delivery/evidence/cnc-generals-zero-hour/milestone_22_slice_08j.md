# M22 slice 08J: detached active-water scene continuation

`W3DTerrainVisual` now derives the pre-map water transition from the complete
live owner state instead of a generated or retail selector. The exact
reset-detached active-water state accepts repeated updates without mutation.
Terrain load attaches terrain first and the existing water owner second,
disables its grid, applies the accepted map override, and unwinds only scene
links created by the failed attempt before a deterministic retry.

Generated coverage proves every state predicate, pending-resource rejection
and reacquisition, failure after the first physical allocation, retry, two
device generations, and zero retained resources. No retail input was used.

Acceptance on the final slice tree:

- Focused terrain-water, scene-boundary and construction routes pass with GCC
  and Clang.
- All six canonical configurations build and their non-GPU/non-LAN/non-retail
  suites pass 261/261; sanitizer suites use `detect_leaks=0`.
- Strict host LeakSanitizer passes the three focused lifecycle routes with GCC
  and Clang.
- Physical public Vulkan controls pass 2/2; serial host LAN passes 4/4 in all
  six configurations.
- All three dependency-ledger checks and `git diff --check` pass.
