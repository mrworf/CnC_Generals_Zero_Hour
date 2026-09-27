# M22 plan 01 slice 08R0: modeled terrain-prop aggregate

Status: planned; implementation-ready in dependency order, not accepted.
Plan transaction parent: `b4fc25b137e94accc1abebd4f0f14bc992c64670`.

## Outcome and dependency graph

Accepted [08Q0](milestone_22_plan_01_slice_08q0.md), original mesh/material,
shroud and transactional frame providers precede
[08R0A](milestone_22_plan_01_slice_08r0a.md) strong composite graph/prop ownership
→ [08R0B](milestone_22_plan_01_slice_08r0b.md) exact shroud material pass
→ [08R0C](milestone_22_plan_01_slice_08r0c.md) transactional source prop frame
→ 08R0 evidence-only closure → [08](milestone_22_plan_01_slice_08.md) → 09.
This is the approved reached modeled terrain-prop correction, not retail admission.
R0 adds no factory selector or replacement renderer. All original retail inputs
remain read-only/private; only fixed redacted categories may persist.

R0 closes in a separate evidence-only commit immediately after C, reusing C's
unchanged frozen executable gates. No executable change or duplicate rerun.
Keep active08 trial source/tests/ledger and the unrelated renderer diagnostic
unstaged throughout each exact plan/behavior transaction.

## Complete public-source branch and ownership audit

FACT: inspected GameLogic::loadMapObjects, W3DTerrainVisual, BaseHeightMap,
HeightMap, W3DPropBuffer, W3DPropDraw, W3DModelDraw model-state selection,
WW3DAssetManager/W3DAssetManager, all registered prototype Create/Clone paths,
RenderObj/Composite/Animatable/HLOD/HTree/Collection/DistLOD, mesh/material/texture
providers, PartitionManager, W3DShroud/W3DShaderManager, DX8 mesh renderer,
WW3D/DX8Wrapper/DynamicVB/IB, RTS3DScene and W3DDisplay frame checkpoints.
The audit used public source only; no retail identifiers or contents are inputs.

| Source boundary / authored branches | Publication and fallibility | Approved owner / preserved semantics |
|---|---|---|
| GameLogic KINDOF_PROP; forced fluff with zero fence width | Weather/time model selected after position/angle adjustment; terrain add before Object construction | A: native optimized terrain route, map ID multiplicity, no synthetic Drawable; shrubs/tree/bridge routes remain separate |
| W3DTerrainVisual first model-draw module | Snow/night model selection; empty authored model is no-op; finite position/angle/scale | A: actual model-state choice; no alternate selector, fallback object or pre-render shroud upload |
| BaseHeightMap buffer creation/reset/free/remove/shroud/view update | CPU currently has no prop buffer; no-prop readiness predicates and empty device methods must compose with exact new owner | A: strong lazy publication and preflight before destructive lifecycle mutation; C: exact provider recreation/frame readiness |
| W3DPropBuffer constructor/type/instance | Reversed zero-length memset leaves fields uninitialized; raw type publication precedes name assignment; 64-type overflow returns type0; clone/transform may throw | A: explicit member initialization (never memset nontrivial strings/vptr), offside type+instance publication, exact 64/4000 admission bounds and reverse unwind |
| clearAllProps/destructor/remove/construction clearing | Existing clear zeros count without releasing instance clones; duplicate IDs remove all matching owners; removed entries are tombstones | A: exact instance then type cleanup; preserve native duplicate-ID/tombstone/type-retention semantics, translate-only sphere bounds and authored geometry clearing |
| Plain Mesh / MeshModel/material graph | Mesh Clone allocates then shares Model by Add_Ref; recolor Make_Unique is not the plain prop clone route | A consumes accepted Q0R1/Q0 ownership; C handles actual renderer registration, not house-color cache reopening |
| HLOD/HModel definition and copy / Animatable / HTree | Raw LOD/cost/value arrays, hierarchy copies, child refs, aggregate/proxy/snap-point storage; parent/container publication precedes later allocations | A: guard entire bounded hierarchy/composite graph and publish nonthrowingly; preserve child/LOD/bone order, copied base-pose behavior, LOD matching and bounds |
| Collection definition and copy | Child Create/Clone refs, SubObjects storage, proxy-list/name storage, snap-point refs can outlive constructor throw | A: offside exact-order graph, guarded local and inserted refs, nonthrowing publication; no generic container change |
| Legacy DistLOD definition/copy/conversion | Temporary DistLOD, strdup name, child-ref raw array, reversed child order and HLOD/HTree conversion; original conversion does not delete the child array | A: complete local guards and exact successful conversion, all interior allocation/ref faults, no leaked conversion array |
| Remaining registered prototypes | Null/Box scalar-only clone paths safe after bounded-name/finite geometry admission; Aggregate base/attach refs unguarded and class ID may alias HLOD; Ring/Sphere shared mesh/material creation unguarded; Dazzle type lookup may index missing type | A: allow proven-safe Mesh/HLOD/HModel/Collection/DistLOD/Null/Box; reject exact Aggregate, Ring, Sphere, Dazzle kinds before construction, including nested kinds. Never use class ID alone for Aggregate discrimination |
| Asset lookup/load-on-demand | Accepted loader/provider may publish resident assets before prop clone; missing asset returns null by native optional path | A validates exact registered prototype graph before Create, bounds recursive depth/nodes and detects cycles; rejected/failed prop work leaves prior cache/prototype/ref identities intact. Accepted loader cache effects remain that provider's declared semantics, not a new parser transaction |
| Weather/time/scale/transform/visibility | RotateZ then finite authored Scale then translation; native type sphere translated only; cull and invalid shroud status before render | A preserves zero/finite-negative scale semantics, exact source state and real partition query; no corrected-radius invention |
| W3DPropDraw optional producer | First origin check, m_propAdded publication and Drawable-ID producer differ from map route; destructor has no native removal | Not admitted by R0: physical create proc remains closed; explicit unsupported-module negative, no new factory owner |
| Prop shroud eligibility | OBJECT_INVALID/CLEAR/PARTIAL_CLEAR/FOGGED differ numerically from CELL_CLEAR; native comparison pushes pass even for admitted OBJECT_CLEAR | B/C preserve the authored comparison; no enum-value 'cleanup'. Native missing partition/player worldbuilder-clear branch remains bounded/generated, with exact provider validation |
| ST_SHROUD_TEXTURE install/uninstall | CPU currently binds texture only; native sets shader, EQUAL depth, camera-space generated coordinates, COUNT2 transform, multiplicative or debug-alpha blend | B: exact program/state/texture/filter/transform transaction and source reset, accepted current content required before pass admission |
| HeightMap/scene scheduling | Terrain/shore/extra → optional edging/roads → props → scorches/bridge/tracks/shroud as authored; prop Render queues mesh tasks, actual triangles emitted at later source flush | C: no direct prop draw relocation; preserve scene material-pass stacking and adjacent mesh/occluded/shader flush then shadow/tree/water/particle order |
| Mesh/FVF/category/task registration and rendering | New VB/IB/category allocation, task/ref queues, skin/lighting/animation/bounds/static-sort state mutate incrementally; global Invalidate would destroy accepted siblings | C: bounded owner-specific offside preparation/checkpoint and exact rollback; never rebuild siblings from globals or use Invalidate as rollback |
| Display-owned frame/present | Existing tree-only journal leaves props-only frame uncovered; source End_Render presents exactly once | C: props-only or combined frame journal before Begin_Render, one final present/CompleteFrame, immutable tree phase retained on failure, no C1/RNG/FX re-advance |
| Release/reset/removal/recreation | Exact source mappings/native generations/refs must retire before owner destruction; stale or active frame cannot safely destroy them | A/C preflight entire operation before mutation, exact idle retry and once-only release; no throwing destructor, deferred admission or stale-generation replay |

Registered unsupported kinds are explicit fail-closed profile boundaries, not
unresolved prerequisites. No independent owner outside A/B/C was found. Any new
mandatory kind/provider or wider transaction stops at an architecture checkpoint.
No ABI/layout/virtual/serialization change, generic container redesign or Windows
success-path change is authorized. Narrow Linux-only friendship/internal sidecars
may expose exact owned state without replacing native source behavior.

## Common final-source acceptance contract

Each behavior child records frozen public source/test hashes and its exact focus
selection/count before gates. No retail files, names, paths, hashes or logs enter
evidence. Register generated inputs and fault seams; seams are owner-local,
once-only generated-friend capabilities with no retail/environment selector.

Configurations: `linux-gcc-debug`, `linux-gcc-release`, `linux-clang-debug`,
`linux-clang-release`, and existing `linux-gcc-sanitized`, `linux-clang-sanitized`
build directories. These six directories were verified by the read-only audit;
never infer a pass from a missing test. Build each complete tree with
`cmake --build build/<preset> -j4`, then run six serial canonical suites:
`ASAN_OPTIONS=detect_leaks=0 ctest --test-dir build/<preset> -LE 'gpu|lan|retail' --output-on-failure -j1`.
Audit complete logs for sanitizer/runtime/validation categories, not exit alone.
Serialize heavyweight sanitizer workloads; test-only timeout changes require
measured isolated timings and documented bounded margin, assertions unchanged.

Child exact focus runs on GCC/Clang debug and both sanitizers; strict host LSan
runs use exactly `ASAN_OPTIONS=detect_leaks=1` and the same complete focus, no
strict UBSan override. Physical routes use existing graphical host escalation,
`python3 tools/run_validation_clean.py <executable> <arguments>` and validation
enabled. A retains established native Vulkan controls; B/C add their generated
physical route on native GCC/Clang and both sanitizers. Fresh established native
Vulkan controls and serial LAN4/4 in all six configurations remain mandatory.
Use established selected commands/inputs, never weaken or substitute host gates.

Immediate rollback proves exact source refs/bytes/cache/prototype/owner order,
Edge mappings and live device resources. Accepted diagnostic/tombstone counters
remain monotonic and bounded; assert their exact candidate delta separately.
Fresh-equivalent device/Edge teardown and allocator shutdown must return the
initialized public-service raw/DMA baseline exactly. Two-generation reset,
provider removal, no stale-generation replay and deterministic retry are required.
Verify ledger hashes, complete owned/staged diffs, plan status/evidence and exact
commit paths. Unrelated renderer diagnostic and active08 trial stay unstaged.

## Readiness and closure

M22-R0-01–06 in the scoped readiness supplement define existing providers,
child-owned implementation outputs, tooling and later host gates. No new tool,
package, service, credential or M0. Public generated fixtures suffice on a clean
checkout; physical availability is freshly checked at acceptance, not inferred.
Aggregate verifies A/B/C accepted commits and unchanged C executable hashes,
then commits only plans/index/evidence as `delivery: M22 08R0 close terrain props`.
