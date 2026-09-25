# M22 plan 01 slice 08I2: edge-aware multi-tile terrain submission

## Goal and observable outcome

Requires accepted slice 08I1. Restore the original pass-major traversal of
every source-owned terrain vertex-buffer tile through the accepted base-terrain
material route. Generated Recording evidence observes every full and partial
edge tile exactly once per source pass, then the already accepted extra-blend
and terrain-track consumers in their existing order.

## Scope and non-scope

Included: validation of the complete tile array, row-major tile traversal
inside each base pass, per-tile valid cell and vertex bounds, edge-aware shared
index ranges, source shader begin/reset ordering, failure unwind, retry, and
single-/multi-tile parity. Because the shared source index buffer has a
32-cell row stride, partial-width edge tiles must submit only their valid rows
or an equivalently exact source-derived bounded index traversal; unused fixed
capacity must never be drawn.

Excluded: allocation/update ownership from 08I1, new materials or generated
geometry, frustum culling, dynamic lights, camera recentering, retail scene
admission, private labels/commands, and final Vulkan pixels. No authorization
or external mutation applies.

## Entry, behavior, and recovery

A generated authored map initialized through 08I1 enters canonical
`HeightMapRenderObjClass::Render`. For each of the two accepted base shader
passes, the original owner binds tiles in source row-major order and submits
only the cells valid for that tile. Exact-boundary tiles retain the efficient
full-tile draw; partial-width rows use bounded first-index/count ranges so no
uninitialized geometry is consumed. The base shader is reset exactly once
before extra blends and tracks. Hidden or incomplete ownership emits no draw or
fails before mutation according to the existing source contract.

If any bind/draw/material operation fails, the active shader is reset when
possible, the exception remains visible to the scene transaction, resource
ownership is retained for deterministic retry, and teardown remains complete.
No failed frame is reported as recorded.

## Implementation surfaces

- Canonical CPU `HeightMapRenderObjClass::Render` in `HeightMap.cpp`.
- The 08I generated original-rendering probe/test and CMake registration.
- `delivery/evidence/cnc-generals-zero-hour/milestone_22_slice_08i2.md`.

## Validation and error handling

Positive coverage includes 1x1, 2x1, 1x2 and 2x2 tile grids with exact and
partial final edges. Recording asserts pass-major ordering, tile row-major
ordering, exact first-index/triangle/vertex bounds for every draw, the expected
authored payload at seams and corners, base-before-extra-before-tracks order,
two device generations, and zero teardown resources.

Negative coverage removes or stales an interior/edge tile, hides the owner,
uses invalid edge metadata, and injects bind/draw failure at first, middle and
last tile in both passes. Assert no out-of-range draw, shader reset, propagated
failure, stable ownership, clean retry, and unchanged single-tile semantics.
Run focused GCC/Clang and sanitizer tests, strict host LSan, canonical
nonretail filtered suites, source/provider/ABI and ledger checks, and
`git diff --check`; aggregate 08I owns the full six-config, physical Vulkan,
serial LAN, and redacted continuation gates.

## Acceptance and commit boundary

All valid generated terrain cells, and no fixed-capacity padding, traverse the
same accepted base material route in source pass/tile order with deterministic
failure recovery. One independently reviewable commit:
`delivery: M22 08I2 submit multi-tile terrain`.

## Result

Complete. The CPU terrain owner validates its full tile grid before mutating
draw state, then traverses every tile row-major inside each base shader pass.
Full-width tiles use one exact bounded range; partial-width tiles use one
shared-index row range per valid row, so no padded cell is submitted. Generated
1x1, 2x1, 1x2 and exact/partial 2x2 coverage proves range counts and ordering,
owner/metadata rejection, physical bind and first/middle/last draw failures,
shader/frame unwind, retry, base-before-extra ordering and teardown.
