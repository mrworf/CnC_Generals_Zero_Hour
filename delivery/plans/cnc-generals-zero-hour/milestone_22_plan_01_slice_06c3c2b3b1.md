# M22 plan 01 slice 06C3C2B3B1: original dynamic pool growth with retained snapshots

## Outcome and boundary

The Linux source DX8 and sorting dynamic VB/IB pools may grow while the original GPU edge or a source-state snapshot still owns the previous generation. Growth retires only the pool reference; the old buffer remains valid through its remaining owners. In-use access, 16-bit index/offset bounds and shutdown with live owners still fail closed. Class layout, Windows behavior, source geometry, retail assets and renderer defaults are unchanged.

## Entry, state and errors

Allocate a small original dynamic access, submit/bind and retain a source buffer reference, then request a larger access. The pool allocates a distinct buffer and preserves the old contents until the edge releases its cache. A simultaneous allocation while the access is in use must reject; deinit while external owners remain must reject, then succeed after all refs retire. B3B subsequently exercises the default 32768-vertex sorting growth with decal/skin/rigid source draws and multiple public-bgfx device generations.

## Validation and commit

Add focused positive/negative original pool ownership tests for DX8 and sorting VB/IB, plus Recording edge cache retention. Run GCC/Clang Debug/Release builds, GCC/Clang ASan+UBSan, original graph/provider-removal/ABI tests and the canonical four-preset asset-free suites. The B3B Vulkan mixed-scene default-capacity run is the physical integration witness; it remains pending if unstable. Evidence `delivery/evidence/cnc-generals-zero-hour/milestone_22_slice_06c3c2b3b1.md`. Commit independently from B3B fixture/shutdown edits.
