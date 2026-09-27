# M22 slice 08L2R1: symmetric binding exception rollback

Status: complete; accepted final-source gates and exact slice commit below.
Plan checkpoint: `a734c4448ec87f6b120ce64a5798f3a368032c37`.
Commit boundary: `delivery: M22 08L2R1 roll back symmetric drawable binding`.

## Delivered boundary

The public source publishes Drawable.m_object before indicator-color/draw
notifications, then Object.m_drawable before model-condition/behavior callbacks.
The former failure previously left a registered Drawable pointing at Object
storage released by the construction transaction, and reset dereferenced it.
Linux GameLogic now snapshots both prior pointers, preserves successful native
notification order and restores both directly on any exception. Its creation
owner withdraws the newly created Drawable/client registry/modules before Object
cleanup. Drawable construction rollback no longer invokes binding notifications
while clearing the exact reverse link. Generic failed binding does not delete
caller-owned prior pairs. Null and foreign-owner inputs reject before mutation.

Private Linux friendship adds no fields, public setter, virtual slot or serialized
state, and leaves Windows declarations unchanged. All six consumers were rebuilt.
The narrow general callback-publication lesson is recorded in AGENTS.md.
Recoloring is not implemented here: 08Q0 remains the separate required owner,
and neither these gates nor the current trial establish retail scene acceptance.

## Generated proof

The registered construction wrapper now supplies a real default draw module,
plus a separate fake-structure input for terrain-decal binding notification.
It checks 52 failure boundaries, including every indexed object-created/draw/
behavior callback reached by the declared fixture and both sides of indicator,
model-condition and fake-decal notification. Failed binding reports restored
links before cleanup; the existing immediate list-head/registry residual witness
remains zero before teardown. Every failure has a clean construction retry.
Mission/skirmish two-generation positives, required-provider removal and total
Recording resource-zero teardown remain asserted.

The completed generated route also exercises the public binding entry against
accepted pairs: null-object/null-draw and foreign-owner attempts reject, while a
callback throw during same-pair rebinding preserves both pairs and exact object/
client list heads. Cleanup never repeats a binding callback. Captured generated
child output is explicitly scanned for ASan/LSan/UBSan findings before discard;
successful exit alone is not sanitizer evidence.

The initial before-condition seam was attached to another similarly named call;
the failed targeted run localized that witness placement, and it was moved to
the actual binding method before final acceptance. All earlier targeted runs
are superseded. The later Python-only diagnostic scan changed no executable
source; GCC focus was refreshed and every remaining gate uses its final hash.
Frozen source/test SHA-256 values are listed in the
[slice plan](../../plans/cnc-generals-zero-hour/milestone_22_plan_01_slice_08l2r1.md).

## Final-source gates

All six complete builds pass. Exact registered 8-control focus passes:

| Configuration | Result | Seconds |
| --- | --- | --- |
| GCC Debug, final wrapper refresh |8/8|10.49|
| Clang Debug |8/8|10.59|
| GCC Release |8/8|7.19|
| Clang Release |8/8|6.42|
| GCC sanitizer |8/8|42.63|
| Clang sanitizer |8/8|25.75|

Exact focus command:
`ASAN_OPTIONS=detect_leaks=0 ctest --test-dir build/<preset> -R '^(original_w3d_generated_construction|original_w3d_generated_scene_boundary|original_w3d_borrowed_file_owner|original_w3d_full_draw_identity|original_w3d_full_draw_provider_removal|original_headless_update|original_simulation_map|original_simulation_reentry)$' --output-on-failure -j1`.
Strict host LSan uses precisely `ASAN_OPTIONS=detect_leaks=1`, no UBSan override,
with that same selection: GCC 8/8 in 54.32s and Clang 8/8 in 31.51s. Host escalation
is recorded; complete verbose logs and generated child diagnostics are clean.

Established host Vulkan source controls pass GCC 3/3 in 5.07s (display owner,
factory bootstrap and map), Clang 2/2 in 4.15s (display owner and map), with the
configured Khronos validation layer and no validation category. Serial host
LAN passes 4/4 all six in 1.71–1.82s, with no sanitizer finding.

Canonical command:
`ASAN_OPTIONS=detect_leaks=0 ctest --test-dir build/<preset> -LE 'gpu|lan|retail' --output-on-failure -j1 -V`.
Suites run serially; complete logs are audited, not only return status.

| Canonical configuration | Result | Seconds |
| --- | --- | --- |
| GCC Debug |280/280;category-clean|331.84|
| Clang Debug |280/280;category-clean|322.46|
| GCC Release |280/280;category-clean|173.24|
| Clang Release |280/280;category-clean|124.37|
| GCC sanitizer |280/280;category-clean|1133.89|
| Clang sanitizer |280/280;category-clean|907.97|

Generated/public project gate logs are local `/tmp/m22-r1-*`; no raw retail log,
name, path, byte, hash or image is retained in these artifacts.

## Composition and exact ownership

Validation covers the frozen R1 executable/test sources plus the unchanged
unstaged active08 trial and unrelated renderer diagnostic. This does not accept
or commit either. The ledger validates the actual worktree composition; only its
three R1 source rows belong to this commit. Four trial hash-only row refreshes
remain unstaged with their trial sources. All six frozen source/test hashes
match the plan. Ledger and whitespace validation pass; exact staging excludes
every active08 trial path/hunk and the unrelated renderer diagnostic.
