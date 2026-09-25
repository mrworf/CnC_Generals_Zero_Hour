# M22 slice 08P0A: atomic source tree registry

The CPU terrain now admits bounded source tree type/instance records only for
its exact published scene and loaded-map owner. Admission preserves
`DrawableID`, model/texture type, position, scale, angle and the source
area-partition policy. Invalid, duplicate, stale, over-capacity and injected
type/instance failures leave earlier entries intact; the first transform
commits `W3DTreeDraw::m_treeAdded` only after admission. Relocation, reverse
removal, `removeAllTrees`, reset and terrain teardown invalidate stale owner
epochs. The physical `W3DTreeDraw` factory create proc remains closed, so no
tree frame or asset-readiness claim is made here.

The first 08P0A implementation placed CPU vectors in the shared
`BaseHeightMapRenderObjClass` layout. An additional physical factory-bootstrap
control caught a changed source-allocation delta. The corrected implementation
uses a lazy registry keyed to the exact terrain owner and erased on reset or
teardown; the shared class has no new fields, and empty graphics startup
allocates no tree registry. The physical display/bootstrap pair passes on the
corrected source. The generated rigid physical control also passes.

An additional generated factory-map physical control still fails at
`W3DTerrainVisual::load` preflight with the fixed category "construction water
owner unavailable." A temporary category-only diagnostic established a
detached water owner with disabled water and no active-water reset state.
That preflight was introduced by 08J commit `ae61258`; it runs before
`TerrainVisual::load`, map binding or any 08P0A tree method. The diagnostic was
removed. This is a separate pre-existing disabled-water reattachment defect,
not an 08P0A regression or a passing control. The governing plan now requires
a plan-only 08J1 corrective split after 08P0A and physical generated-map
revalidation before retail slice 08 or slice 09 acceptance.

Generated two-generation owner tests cover atomic type/instance failures,
identity preservation, duplicate/stale IDs, partition relocation, 64-type
and 4000-instance limits, reset/shroud unready behavior and zero Recording
residual. A generated full-draw fixture confirms the physical tree create
proc remains closed before scenario setup. No private retail input or raw
output was used or retained.

Final-source validation: six complete GCC/Clang Debug, Release and ASan+UBSan
builds and six canonical nonretail suites passed 267/267 each
(`-LE gpu|lan|retail`; sanitizer suites used `ASAN_OPTIONS=detect_leaks=0`).
Both focused strict host LSan groups passed 2/2, including the source tree
registry and closed-factory witnesses. The required non-sanitized physical
Vulkan display/factory-bootstrap controls passed 2/2 with validation; the
additional rigid factory control passed 1/1. Serial host LAN passed 4/4 in
all six configurations. The original dependency ledger and `git diff --check`
pass. The unrelated renderer diagnostic remains unstaged.
