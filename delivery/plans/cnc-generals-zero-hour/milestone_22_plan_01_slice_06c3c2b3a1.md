# M22 slice 06C3C2B3A1: stable Linux W3D object allocation

## Outcome and scope

Requires A0 and A0B. On the Linux CPU-only original W3D port, allocate `W3DMPO` objects through normal C++ `new`/`delete` while preserving the polymorphic base/glue layout and all source classes, render paths, and asset/refcount semantics. This removes the legacy fixed per-class pool implementation from this port's allocation path. Windows/original builds retain their class-pool policy. This is a risk-mitigation prerequisite to B3A, not a claim that a specific pool write is proven, nor source-scene acceptance.

## Entry, state, and failure behavior

Original asset loading, rendering, and shutdown continue to construct and release the same W3D classes through their source entry points. Linux allocations use the process's original global allocator. A failed allocation follows ordinary C++ allocation failure rather than returning a partial object. No user authorization, persistent data, or UI behavior applies. Source refcounts, static-sort ownership and device resource retirement must remain unchanged. The `ZH_WW3D_ASAN_DIRECT_NEW` diagnostic switch becomes unnecessary on Linux and must not create divergent translation-unit layouts.

## Evidence and validation

The hypothesis is allocator-policy-dependent corruption: with A0 applied, the pooled Clang ASan+UBSan two-generation source scene failed intermittently at a corrupt material texture owner or stalled on second-generation render; a diagnostic globally-direct-new build passed 10/10 fresh-process two-generation source scenes. No pool-size mismatch was seen under the opt-in runtime check. That evidence does not identify the precise first bad write.

Positive tests: GCC/Clang source CPU graph and renderer focused tests preserve scene, asset lifecycle, static-sort order, retries, and refcounts. Host RTX Vulkan validation-enabled source scene must pass at two extents and both BGRA8/RGBA8 over at least 30 fresh-process, two-generation runs. Negative tests: original source failure injections and stale/missing-session controls still fail closed and recover; ABI/provider-removal and dependency-ledger checks pass. Run all four established compiler/sanitizer presets and milestone-relevant full CPU/host suites. Inspect compile commands for one consistent Linux W3D macro definition, and verify no unintended change to retail symlink content.

Acceptance and commit boundary: all required checks pass without sanitizer fault, timeout, validation error or resource/refcount regression. Commit only the Linux W3DMPO allocation policy, tests if required, this plan and evidence. B3A still owns the normal physical source/static pixel acceptance and may remain pending independently.

Commit: `delivery: M22 slice 06C3C2B3A1 stabilize Linux W3D allocation`.
