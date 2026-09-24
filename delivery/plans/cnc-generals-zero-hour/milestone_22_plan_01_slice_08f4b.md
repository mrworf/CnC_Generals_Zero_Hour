# M22 plan 01 slice 08F4B: display shroud refresh dispatch

## Outcome and source route

Implement the original `W3DDisplay::clearShroud` and `setShroudLevel` CPU
methods reached by `PartitionManager::refreshShroudForLocalPlayer` after
radar map setup. Native clear performs no mutation. Native per-cell dispatch
maps `CELLSHROUD_SHROUDED`, `CELLSHROUD_FOGGED`, or a clear cell to configured
shroud/fog/clear alpha, writes the existing `W3DShroud` grid, then calls the
accepted 08F4A terrain notification. Preserve source order; do not synthesize
a replacement grid or a frame.

## Scope and state

Require exact initialized display, GPU edge, primary scene, map-owned terrain
and shroud provider before even the clear no-op. For per-cell calls require a
valid logical category and in-range grid coordinate; reject malformed or
foreign/missing providers before mutation. Existing `W3DShroud` bounds and
alpha clamps remain authoritative. Verify repeated clear, category mapping,
provider removal, invalid cell/category, failed-resource retry where relevant,
two fresh generated generations and zero Recording resources on teardown.
No user authorization applies to this internal method; exact generated owner
identity is its admission boundary.

Do not admit 08F construction selector, implement border shroud setter,
active prop buffer, pixel render, retail provider traversal, or any private
retail data. Default/08D/08F0 unchanged.

## Dependency, validation and commit

Requires accepted 08F4A and 08F3. Use generated read-only map/shroud
fixtures, source/link identity and provider-removal controls. Run focused
GCC/Clang, six configured builds and canonical `-LE 'gpu|lan|retail'` suites,
canonical sanitizer `detect_leaks=0`, strict host LSan both, physical Vulkan,
serial LAN all six, ledger and diff gates. One separate 08F4B
production/test/evidence commit after acceptance; only then resume 08F.

## Source-order implementation detail

The accepted 08F4A notification is deliberately a no-op for its valid
no-prop owner, but throws for a malformed non-null prop. Calling it only
after the shroud write could therefore leave a partial cell mutation.
Add a CPU-only side-effect-free readiness predicate to the terrain owner;
both the accepted notification and display validation use it. The display
still calls `notifyShroudChanged` exactly once, after the native-ordered
grid write. Do not pre-call notification or attempt to reconstruct the
shroud's private current/final state for rollback. Test forced malformed
prop and other broken owners leave an existing cell unchanged.

## Result

Complete. Source-ordered display clear/per-cell shroud dispatch, generated
negative/retry/two-generation ownership tests, source ledger updates and all
required acceptance gates are recorded in
[08F4B evidence](../../evidence/cnc-generals-zero-hour/milestone_22_slice_08f4b.md).
The 08F construction selector remains unadmitted.
