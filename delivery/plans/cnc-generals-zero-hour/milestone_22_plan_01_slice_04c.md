# M22 plan 01 slice 04C: original mesh entry and source-local WW3D state

## Outcome and dependencies

Requires 04B. Original `MeshClass::Render` executes its visibility, frustum,
static-sort, alpha/shadow, base/additional-pass, skin/decal and category-queue
decisions. Original `ww3d.cpp` owns its own static state and render/flush
entry order. Category/device calls still fail typed at their first physical
operation; this slice cannot claim completed passes or frames.

## Source closure and ownership

Compile canonical `ww3d.cpp` for CPU entry/state, remove duplicated static
defaults from `ww3d_cpu_state.cpp`, and link canonical static-sort ownership
for reached sort routing. Retain original `hlod.cpp`, `mesh.cpp`,
`meshmdl.cpp`, `dx8renderer.cpp`, `dx8polygonrenderer.cpp` and shader/material
owners. Source-local physical guards are allowed only where original code
first needs the absent device; never precompute or reorder subsequent
material/pass decisions. Characterize `USE_WWSHADE` build selection and
read-only retail shader-family encounters; do not assert `SHD_FLUSH` in a
configuration where the authored macro compiles it out. Physical commands
begin in 05 and remaining interleaved source scheduling closes in 06.

## Test and failure contracts

Use owned W3D fixtures to prove hidden and rigid-frustum negatives; positive
skin and camera-visible rigid registration; selected sort, alpha/shadow,
additional-pass and decal queues where reachable; explicit typed first
physical operation, source-owned state, teardown/retry, no mixed ABI,
provider removal, GCC/Clang full suites, focused sanitizers and ledger
freshness. No claim about a complete pass or WWShade flush. Original
interleaved pass ordering is mandatory 06 before GameClient/retail consumers.

## Commit boundary

One independently validated commit: `delivery: M22 slice 04C restore original mesh entry`.

## Completion

Complete. See `delivery/evidence/cnc-generals-zero-hour/milestone_22_slice_04c.md`.
