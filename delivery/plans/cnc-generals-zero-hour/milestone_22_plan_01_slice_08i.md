# M22 plan 01 slice 08I: general multi-tile terrain aggregate

## Outcome and dependency order

Requires accepted slices 08I1 then 08I2. Couple checked source tile-grid
allocation, per-tile authored payload/update ownership, exact partial-edge
geometry and pass-major row-ordered submission through one generated canonical
`HeightMapRenderObjClass` owner. This prerequisite must complete before slice
08 may resume retail scene admission.

## Scope and acceptance

The aggregate generated fixture exercises X-only, Y-only and both-axis tile
crossings, exact and partial final edges, authored base/primary and extra-blend
state, two device generations, every child rollback boundary, and clean
teardown. Recording proves that all accepted static terrain tiles are uploaded
and submitted, fixed-capacity padding is never drawn, and base passes still
precede extra blends and tracks. No retail input or selector is used.

Run focused GCC and Clang tests; ASan+UBSan with `detect_leaks=0`; strict host
GCC and Clang LSan; all six canonical nonretail suites using the established
`-LE 'gpu|lan|retail'` filter; physical Vulkan with validation output scanned;
serial LAN; source/provider/ABI and dependency-ledger checks; and
`git diff --check`. Record only public generated evidence in
`delivery/evidence/cnc-generals-zero-hour/milestone_22_slice_08i.md`.

One aggregate commit records only coupling/evidence changes:
`delivery: M22 08I close multi-tile terrain`.
