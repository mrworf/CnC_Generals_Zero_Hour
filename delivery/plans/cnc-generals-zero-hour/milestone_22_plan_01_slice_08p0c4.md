# M22 plan 01 slice 08P0C4: exact physical tree module admission

## Goal and boundary

After C1–C3, register the exact `W3DTreeDraw` create proc and module data
only in the full-instance `W3DModuleFactory`. The M20/M21 schema-only path
and the other eight unavailable draw providers stay closed. Follow native
first nonzero transform admission to the terrain tree buffer, later move,
destructor removal and owner/device-epoch checks; zero-XY placement defers.
Its source `doDrawModule` is empty, so the C2/C3 terrain pass remains the
only draw. Require matching ready map/terrain, accepted source resources,
exact Drawable ID and test-only Recording provider before publication.

Stage module/tree/terrain publication atomically with Drawable construction.
On create, transform, registration, resource or frame failure, unwind in
reverse without gameplay destroy hooks or stale partition/Drawable/tree/decal
owner. Replacement and provider removal preserve accepted owner identity;
clean retry succeeds without world reset. Authorization is not applicable;
retail admission remains slice 08.

## Validation and commit

Generated full-instance fixtures cover first/zero/repeated transform, move,
remove, multiple types, shadow-enabled/disabled frame, matching and mismatched
terrain, provider removal, two generations and each owned failure boundary.
Assert immediate pre-teardown residual zero and no half-publication, then
retry. Run six complete builds and canonical nonretail suites, focused strict
host LSan, physical Vulkan controls, serial LAN 4/4 all six, ledger and diff
checks on final source. Commit one slice:
`delivery: M22 08P0C4 admit physical W3DTreeDraw`.
