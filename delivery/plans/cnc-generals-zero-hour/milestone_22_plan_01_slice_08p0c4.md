# M22 plan 01 slice 08P0C4: exact physical tree module admission

## Goal and boundary

Status: implementation-ready after the approved factory/unwind architecture
checkpoint; persist this plan-only correction before executable edits.
Transaction parent is accepted C3 `b2a613b6b69b3bc1c33e4a3043381e4a5a84082f`.
Preserve the unrelated renderer diagnostic unstaged. No additional owner/split
may be introduced without an explicit architecture checkpoint.

After C1–C3, register the exact `W3DTreeDraw` create proc and module data
only in the full-instance `W3DModuleFactory`. The M20/M21 schema-only path
and the other eight unavailable draw providers stay closed. Follow native
first nonzero transform admission to the terrain tree buffer, later move,
destructor removal and owner/device-epoch checks; zero-XY placement defers.
Its source `doDrawModule` is empty, so the C2/C3 terrain pass remains the
only draw. Require matching ready map/terrain, accepted source resources,
exact Drawable ID and the accepted generated Recording/bgfx capability before
publication. This opens only the full-instance source create proc, not a retail
selector or retail scene admission.

Stage module/tree/terrain publication atomically with Drawable construction.
On create, transform, registration, resource or frame failure, unwind in
reverse without gameplay destroy hooks or stale partition/Drawable/tree/decal
owner. Replacement and provider removal preserve accepted owner identity;
clean retry succeeds without world reset. Authorization is not applicable;
retail admission remains slice 08.

## Resolved factory, transform and removal contracts

Only the full-instance factory selects `addModule(W3DTreeDraw)`; schema/headless
registration remains data-only and the other eight unavailable providers remain
closed. Validate the typed module-data provider before using its fields. All
translation units that instantiate or own the physical class must see identical
`ZH_WW3D_CPU_ONLY` definitions. The audited full factory currently lacks that
definition while its inline pooled create proc computes class size; correct it
before opening the proc and prove exact size/alignment and owner-field layout
agreement between factory, constructor and generated consumer. Do not change
Windows declarations, serialized fields or native non-CPU class layout.

Preserve zero-XY deferred tree addition and first nonzero transform admission.
Capture exact terrain identity, registry epoch and device identity/generation;
reject stale/provider-replaced movement before any accepted registry mutation.
Repeated transforms update the same Drawable ID, not a second tree. The native
empty `doDrawModule` remains empty; tree/decal drawing belongs only to terrain.

The existing Thing setters mutate before notifying modules, so a rejected tree
callback otherwise leaves source transform and terrain position inconsistent.
Give the existing Linux mutation/notification boundary strong exception safety:
before any mutation snapshot `m_transform`, `m_cachedPos`, `m_cachedAngle`,
`m_cachedDirVector`, both cached terrain/terrain-or-water altitudes and
`m_cacheFlags`. On callback rejection restore those exact fields directly,
without setters, notification, new class fields or vtable/serialization changes.
Cover setPositionZ, setPosition, setOrientation and setTransform, including
nested slope alignment; successful unrelated callback behavior is unchanged.

Add a Linux-only DrawModule removal-preflight virtual with a default no-op and
tree override, plus a bounded Drawable module scan. Reject active source-frame
or otherwise nonretirable exact owners before any side effect, including module
ownership destruction. Check before GameClient destroyDrawable/list/UI/binding
mutation, whole-list GameClient reset, GameLogic destroyObject/onDestroy/status,
whole-list destroyAllObjectsImmediate/reset, pending processDestroyList mutation,
Object/Drawable construction rollback flags/unregistration and their existing
friend deletion wrappers. Engine/subsystem reset admission must happen before
window/subsystem mutation and outside resetAll's catch-and-shutdown boundary.
Construction failure unwind remains reverse and does not run gameplay hooks.
An unapplied/zero-XY module owns no terrain work and its default cleanup is empty.

Caller-visible terrain reset/freeMapResources/removeAllTrees and visual reset
must likewise reject before clearing a live phase/map/scene owner. The supported
shutdown sequence first aborts/completes the source frame through its existing
Edge owner, then deletes modules/terrain and process services; a destructor is
never the rejection boundary. Raw protected destructors and bypassing an owned
Drawable via a base MemoryPoolObject pointer are not supported deletion routes.
Audit actual pooled deletion calls and prove the shipping/generated shutdown
ordering rather than throwing from a destructor or silently abandoning resources.

Module teardown uses an allocation-free exact-ID/epoch detach, not removeTree's
allocating candidate-vector copies. Compact existing bounded instance/type
storage in place; retain surviving IDs, type references and partition buckets,
cancel only the accepted owner's phase, retire GPU/atlas/model references in
reverse order and erase an empty registry. Prove alias multiplicity and last-user
cleanup, no allocating/throwing destructor work, and exactly-once release after
reset/provider removal. Stale epochs must not delete a replacement registry.
Frame rejection preserves phase/resource identity and permits idle retry.

## Implementation surfaces and exact focused commands

Expected surfaces: full W3DModuleFactory/CMake definitions and identity witness,
W3DTreeDraw header/source, DrawModule/Drawable removal admission, Thing source
mutation guard, GameClient/GameLogic/Object and engine/subsystem reset callers,
CPU terrain/visual exact detach/reset boundaries, generated module probe/wrapper,
full probe wiring, CMake registrations and dependency ledger. No unrelated
shadow mode, shader, allocator, retail admission or source rendering redesign.

`cmake --build build/<preset> --target zh_original_w3d_full_probe original_w3d_cpu_graph_tests original_w3d_tree_program_tests original_w3d_source_reference_tests original_w3d_stage_transaction_tests renderer_recording_transaction_tests renderer_bgfx_transaction_resource_tests renderer_bgfx_transaction_tests original_w3d_schema_tests original_w3d_abi_schema original_w3d_abi_full -j4`

`ctest --test-dir build/<preset> -R '^(original_w3d_tree_module|original_w3d_tree_decal|original_w3d_tree_draw|original_w3d_tree_preparation|original_w3d_tree_program|original_w3d_terrain_shroud_projection|original_w3d_terrain_map_frame|original_w3d_source_reference|original_w3d_stage_transaction|renderer_recording_transaction|renderer_bgfx_transaction_resource|renderer_bgfx_transaction|original_w3d_schema|original_w3d_schema_identity|original_w3d_schema_provider_removal|original_w3d_abi|original_w3d_full_draw_identity|original_w3d_full_draw_provider_removal|original_w3d_generated_construction)$' --output-on-failure -j1`

Run this exact nineteen-control union on GCC/Clang debug and both sanitizers;
sanitizer focus uses `ASAN_OPTIONS=detect_leaks=0`. Strict host LSan uses the
same union with exactly `ASAN_OPTIONS=detect_leaks=1`, no UBSan override.
Retain the asset-free existing full draw scenario separately:
`python3 tests/original_rendering/test_w3d_full_draw_scenario.py --source-root . --executable build/<preset>/zh_original_w3d_full_probe --asset-producer build/<preset>/original_w3d_cpu_graph_tests`
(no retail-archive argument). Its tree-without-ready-terrain rejection remains
an explicit missing-owner control, not a claim that the physical proc is closed.

Generated module physical route:
`VK_INSTANCE_LAYERS=VK_LAYER_KHRONOS_validation ASAN_OPTIONS=detect_leaks=0 python3 tests/original_rendering/test_w3d_tree_module.py --source-root . --executable build/<preset>/zh_original_w3d_full_probe --asset-producer build/<preset>/original_w3d_cpu_graph_tests --gpu`.
Run all four configurations under the established escalated graphical host
environment; retain generated tree/decal/program/shroud and established Vulkan
controls. Audit complete logs rather than exit codes alone.

## Validation and commit

Generated full-instance fixtures cover first/zero/repeated transform, move,
remove, multiple types, shadow-enabled/disabled frame, matching and mismatched
terrain, provider removal, two generations and each owned failure boundary.
Assert immediate pre-teardown residual zero and no half-publication, then
retry. Run six complete builds and canonical nonretail suites, focused strict
host LSan, physical Vulkan controls, serial LAN 4/4 all six, ledger and diff
checks on final source. Commit one slice:
`delivery: M22 08P0C4 admit physical W3DTreeDraw`.

Additional exact controls: factory/constructor/consumer size/layout agreement;
wrong/null module data; missing map/terrain/edge/assets and stale device/registry
epochs; each transform rejection restores all Thing cache bytes/flags and the
accepted tree position; successful non-tree callbacks preserve prior behavior.
Active-frame destroy/reset/rollback must reject before list/lookup/binding,
partition, status, hooks, flags, resource or queue changes, then idle retry
detaches exactly once. Prove multiple same-type aliases, distinct-type index
compaction, last-tree registry retirement, stale-owner no-retarget, constructor
unwind and two-generation shutdown. Retain C1/RNG/FX and C2/C3 identical frame
retry assertions. Record focused, strict, physical, six complete build/canonical,
LAN, ledger and exact-owned diff provenance before the one implementation commit.
