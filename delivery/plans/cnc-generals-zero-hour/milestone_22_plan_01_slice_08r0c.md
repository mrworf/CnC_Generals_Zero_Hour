# M22 plan 01 slice 08R0C: transactional source terrain-prop frame

Status: complete and accepted; 08R0A and 08R0B accepted. Six corrected complete
builds, exact10 focus on four configurations, strict host LSan exact10 both,
physical four, fresh minimal/Vulkan/LAN and six canonical suites 291/291 pass.
Evidence: [transactional prop frame](../../evidence/cnc-generals-zero-hour/milestone_22_slice_08r0c_prop_frame.md).
Plan transaction parent: `b4fc25b137e94accc1abebd4f0f14bc992c64670`.
Command/queue refinement parent: `c382a699f61a91dbaa992b99c9b4a832918bee50`.

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

### Verified queue and preparation boundary

Public-source audit confirms `W3DPropBuffer::drawProps` performs cull and the real
partition/local-player shroud query, installs native terrain-object lighting, and
calls each admitted render object's `Render`. It does not emit prop triangles.
`MeshClass::Render` enqueues original `PolyRenderTaskClass` and visible/delayed
`MatPassTaskClass` work, skin links, or the source static-sort list. The later
DX8 rigid/skin/category flush and static/sorting flush emit the triangles.
R0B remains only exact shroud-pass semantics; C owns all queue preparation,
registration, frame scheduling, rollback and successful retirement.

Registration audit includes `Register_For_Rendering` / `Register_Mesh_Type`,
rigid and skin containers, vertex split/gap arrays, FVF list growth, 64-way
texture/material/shader booking, polygon renderer construction and both list
memberships, VB/IB append bytes/counters and source buffer mapping generation.
Never allow a failed publication to unregister or rebuild an accepted sibling.
Prepare guarded bounded candidate allocations/list nodes/task ownership units
outside the source frame; preserve native matching, grouping and insertion order.
Publish only after complete admission, with nonthrowing owner-local publication
and reverse exact withdrawal on failure. Narrow Linux friendship/internal
sidecars may access these owner fields without changing generic container
operations, class fields/vtables/layout/serialization or Windows behavior.

Frame capture must retain consumed task/list ownership until device commit:
restoration restores the same nodes, refs, head/tail/link order and source bytes,
not newly allocated reconstructions. Pre-existing populated work and accepted
categories are checkpointed exactly; newly prepared units are canceled exactly.
Include procedural per-polygon APT/dynamic buffers, delayed passes, skin streaming
offsets/scratch storage, source static-sort levels/ref nodes, sorting state/ref
nodes/temp-index storage and all counters. Unsupported NPatches or malformed
source/FVF/category/pass/range providers reject before registration or journal.
All bounds use accepted D0 limits (4096 commands/resources, ordered-view and
64MiB byte bounds), including mixed manifests and exact bound+1 rejection.

Visual-animation admission recursively validates all reachable SINGLE/DOUBLE/
COMBO motion providers before any journal, queue/category registration, animation
time, RNG or C1 mutation. Sound-bearing or indeterminate motion graphs reject
with exact no-mutation residual and deterministic retry. `Animatable3DObjClass`
can dispatch embedded sound Play/Stop during transform update, which is outside
this frame rollback owner; C does not admit that effect or change ordinary
nontransactional sound behavior. Sound-free visual animation, hierarchy/bone/
LOD/cache state remains within C's exact checkpoint.

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

Register `original_w3d_prop_frame` using the established full-probe/Python
generated-map owner, bounded public packet/map and wrapper `--gpu`;
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

The wrapper is `tests/original_rendering/test_w3d_prop_frame.py`, invoking
`zh_original_w3d_full_probe` through the existing generated mission/map fixture
and `original_w3d_cpu_graph_tests` asset producer. Do not add a standalone prop
process/device owner, retail selector or alternate draw route. The generated
profile dispatch is isolated from the preserved active08 trial hunks.
The fresh equivalent generated operation sweep reaches exactly 1382 rejected
props-only and 1386 rejected combined ordinals followed by clean success. Its
unchanged two external native GCC processes measured 137.385/137.275 seconds
under an isolated, diagnostic-only 240-second ceiling. The inherited 30-second
helper default cannot represent that workload; only this wrapper uses a bounded
180-second per-process native limit (~31% headroom), retaining TimeoutExpired
and all workload/assertions. After the lighting correction, isolated category-clean
GCC sanitizer processes measured 565.788/566.341 seconds and Clang measured
492.544/492.894 seconds, with unchanged 1382/1386 ordinal counts, retry and
zero-resource teardown. Diagnostic-only runs used the existing helper timeout
parameter and are not acceptance. The approved wrapper-only final bound is
750 seconds per process (~32% margin over the slower sanitizer), with unchanged
TimeoutExpired failure, workload, inputs and assertions; no production/CMake
change or rebuild. Corrected native ten-control results at the stricter 180-second
bound and the corrected six complete builds are retained as proportional evidence:
their executable/helper bytes and selected behavior assertions are unchanged.
Refresh the wrapper hash; rerun registered sanitizer exact ten controls, strict
ten controls on both sanitizers, generated physical four configurations and all
six fresh canonical suites, plus the common fresh minimal/Vulkan/LAN controls.
DynamicSorting VB/IB access binding is admitted only when both the pending
source-frame journal and mesh checkpoint are active. Preserve the legacy guard
elsewhere; prove outside rejection and inside failure/retry with exact source
buffer identity/type/offset restoration and original SortingRenderer lowering.
Prop lifecycle preflight and destructor invariants reject both a pending source
frame checkpoint and an active pass; unchanged Edge retirement semantics remain
outside C. Reject add before constructing a private candidate while pending,
so constructor cleanup retains its proven idle contract. Generated pending and
active reset/free/add controls preserve exact owner/frame/resource identities;
journal close restores idle preparation and deterministic draw/removal retry.
Sanitizer localization reached native MeshClass::Set_Lighting_Environment's full
member assignment: inactive InputLight boolean slots were uninitialized by the
LightEnvironment constructor. Initialize every InputLight/OutputLight/Fill field
to explicit portable neutral values at construction, including local directional
InputLight values whose unused point fields otherwise remain indeterminate.
Preserve layout and active-light semantics; no rollback workaround or sanitizer
suppression. Add fresh, populated/reset and copy regression plus the reached
late-fault sanitizer proof. Initial native/build gates and the 560.325-second
category-rejected sanitizer diagnostic are superseded, not acceptance. Measure
sanitizer timing only after its category audit is clean.
Build focus:
`cmake --build build/<preset> --target zh_original_w3d_full_probe original_w3d_cpu_graph_tests original_w3d_prop_owner_tests original_w3d_tree_program_tests original_w3d_stage_transaction_tests original_w3d_source_reference_tests -j4`.
Exact ten-control focus:
`ASAN_OPTIONS=detect_leaks=0 ctest --test-dir build/<preset> -R '^(original_w3d_prop_frame|original_w3d_prop_owner|original_w3d_prop_owner_identity|original_w3d_prop_owner_provider_removal|original_w3d_prop_shroud|original_w3d_terrain_map_frame|original_w3d_tree_draw|original_w3d_tree_decal|original_w3d_stage_transaction|original_w3d_source_reference)$' --output-on-failure -j1`.
Generated physical command on GCC/Clang native and both sanitizers:
`python3 tools/run_validation_clean.py python3 tests/original_rendering/test_w3d_prop_frame.py --executable build/<preset>/zh_original_w3d_full_probe --asset-producer build/<preset>/original_w3d_cpu_graph_tests --source-root . --gpu`.
Strict host LSan uses exactly `ASAN_OPTIONS=detect_leaks=1` with the same ten-control
CTest expression, host escalation and no strict UBSan override. Native/generated
controls cover every conditional branch above; a sound-bearing/indeterminate
motion negative proves no dispatch, frame, queue, animation-time or RNG changes.
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
