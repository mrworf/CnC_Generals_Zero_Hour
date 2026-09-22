# M22 plan 01 slice 07A: canonical status-circle Linux compile frontier

## Outcome and boundary

Requires accepted 06C. The original `W3DStatusCircle` source is the first authored 2D object constructed by `RTS2DScene`, but the current Linux CPU-only scene constructor rejects before creating it and the object is absent from the full W3D draw target. Compile the unchanged source behavior under the existing CPU ABI, correcting only case-sensitive include/source portability blockers proven by the canonical GCC/Clang commands. Keep the original no-device rejection and production factory unchanged. This is a compile/link prerequisite, not a physical 2D scene or 07 acceptance.

## Positive, negative and gate

Add the canonical `W3DStatusCircle.cpp` to the full draw object target once it compiles under GCC/Clang Debug and sanitizers. Verify the object and its original constructor/`Render`/`initData` symbols in each build, and that the production schema-only binary still does not link reached physical status-circle behavior. Preserve the test's typed no-device `RTS2DScene` rejection, with no silent scene publication. Do not stub original draw operations, change class layout, alter retail content or activate `W3DDisplay::init`. Run focused original GameClient identity/provider tests and four full asset-free suites with ledger freshness. One plan/code/test/evidence commit; 07B will own construction and physical 2D pixels before any production switch.
