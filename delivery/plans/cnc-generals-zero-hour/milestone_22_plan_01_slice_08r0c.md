# M22 plan 01 slice 08R0C: transactional source terrain-prop frame

Status: planned; dependency-blocked until 08R0B acceptance.
Plan transaction parent: `b4fc25b137e94accc1abebd4f0f14bc992c64670`.

## Outcome, dependencies and boundary

Depends on [08R0A](milestone_22_plan_01_slice_08r0a.md),
[08R0B](milestone_22_plan_01_slice_08r0b.md), accepted 01/05B2B2B1 mesh/material,
06C4D original mixed/static/sorted frame, 08P0C2D0 device transactions, 08P0C2B exact shroud,
08P0C2C/D immutable tree/frame and 08P0C3 projected tree decals. Full transitive
inventory and final-source gates are in [R0](milestone_22_plan_01_slice_08r0.md).
Provide native prop scheduling and bounded props-only/combined terrain frame
through source mesh renderer, Recording and physical Vulkan. No direct adapter
replacement, W3DPropDraw factory, retail admission, new shader/owner mode or
advancement of already-published C1/RNG/FX on render retry.

## Complete frame admission and rollback inventory

Prepare every fallible immutable prop/model/material/texture/FVF/category/VB/IB
requirement outside source frame, with exact generation/resource pins and bounded
command/resource/byte/view manifest. Validate supported rigid/skin, per-polygon
material, additional/delayed material pass, LOD/animation and static-sort branches
before mutation; unsupported malformed/provider branches reject explicitly, never
skip geometry. Source camera/local player/partition/shroud queries remain real.
Props retain native visible/status predicate, including CLEAR pass comparison.

Capture exact owner-specific bounded state before Begin_Render: prop visibility/
shroud/change/cull flags and render light, HLOD/HTree transforms/bones/hidden/bounds/
LOD/animation time; mesh light environment/alpha/emissive/base offsets/skin links;
registered/FVF/category/polygon-renderer lists and identities/capacities, visible
category/material/delayed/skin/decal tasks and ordering/refs, all affected counters;
static/sorting queues; source DX8 maps/caches/pending material/stage state; dynamic
and registered VB/IB identity/capacity/offset/bytes; camera/scene lights/queues;
WW3D counters/statistics; track/particle/smudge/shadow/terrain transients and exact
texture loader/expiry metadata already captured by C2D. Accepted siblings remain
identical. Allocate/guard candidate task/category/storage graph offside, publish
only within complete admitted checkpoint; no global Invalidate/reset shortcut.
All commit replay is allocation-free within D0 admitted API-thread bounds; worker/
backend allocation remains existing device-loss handling, not reversible claim.

Preserve source HeightMap ordering: terrain/shore/extra, authored edging/roads,
prop Render task generation, scorches/bridge/tracks/additional terrain shroud;
actual mesh triangles occur at the existing source scene flush, followed by
occluded/shader flush and then projected/tree/shadow/water/particle families as
native dictates. Preserve scene material-pass stacking and tree decal adjacency.
Do not relocate prop triangles into the task-generation slot. Exact command
order and populated sibling draw proof are required, not marker-only evidence.

Display owns journal before Begin_Render for props even without any tree phase.
Exactly one present/CompleteFrame is the last command. Success commits frame,
drains tasks/retirements once and retires any accepted tree phase exactly once.
Any begin/pass/stage/draw/flush/end/present/commit failure aborts device journal,
restores complete source snapshot and accepted prior pixels/commands, retains
immutable retry identity and tree phase. Retry does not re-run C1/RNG/FX. No live
journal spans irreversible callbacks or old-resource retirement. Removal/reset/
provider loss preflight before destruction, cancel owned pending work exactly,
and never access stale registry/mappings during replay.

## Generated witnesses and exact commands

Register `original_w3d_prop_frame` with bounded generated packet/map and --gpu;
exercise public map add and display.draw, not direct draw substitution. Cover
props-only and combined tree/decal/shadow/water frame; mixed direct Mesh and
HLOD/HModel/Collection/converted DistLOD, shared type/texture refs, animated/LOD,
rigid/skin/static-sort and additional/delayed passes when authored and supported.
Prove native source order, cull/shroud CLEAR/PARTIAL/FOGGED/suppressed behavior,
real reveal/undo query, nonuniform shroud, lighting/scale and physical pixels.
Assert populated sibling command/owner identities unchanged.

Sweep every device/source publication ordinal from fresh equivalent generation;
each instance retains same-phase failure→identical retry, exact prior pixel/frame/
counter/list/bytes baseline, no duplicated FX or RNG changes. Include late present/
commit faults, unsupported overlap, exact manifest maxima/bound+1, tombstone
capacity rejection, allocation faults, provider loss/recreation, active-frame
removal rejection/idle retry, reset/cancel/hidden/empty and two-generation teardown.
Immediate owner/live-resource residual and exact monotonic diagnostic delta are
separate; final initialized raw/DMA baseline must match after complete teardown.

Build focus:
`cmake --build build/<preset> --target zh_original_w3d_full_probe original_w3d_prop_owner_tests original_w3d_prop_shroud_tests original_w3d_prop_frame_tests original_w3d_tree_program_tests original_w3d_stage_transaction_tests -j4`.
Exact ten-control focus:
`ASAN_OPTIONS=detect_leaks=0 ctest --test-dir build/<preset> -R '^(original_w3d_prop_frame|original_w3d_prop_owner|original_w3d_prop_owner_identity|original_w3d_prop_owner_provider_removal|original_w3d_prop_shroud|original_w3d_terrain_map_frame|original_w3d_tree_draw|original_w3d_tree_decal|original_w3d_stage_transaction|original_w3d_source_reference)$' --output-on-failure -j1`.
Generated physical command on GCC/Clang native and both sanitizers:
`python3 tools/run_validation_clean.py build/<preset>/original_w3d_prop_frame_tests --gpu`.
Freeze source after native audit/focus, then run all R0 common final-source gates
including strict LSan, six serial canonical suites and fresh physical/LAN controls.

## Readiness and exact closure

M22-R0-04 frame code/witness are C outputs; A/B and listed accepted providers gate
entry. PRE-002 already available, PRE-012/016 checked at physical acceptance;
retail is not fixture data. No unresolved rollback owner is left as a later
investigation gate; checkpoint any independent ownership gap before broadening.
Commit C only: `delivery: M22 08R0C render terrain props transactionally`.
Immediately close separate evidence-only R0 aggregate using unchanged C gates,
then rebase preserved active08 trial against accepted owners and resume only its
bounded redacted probe. Do not reopen prior accepted slices or stage renderer.
