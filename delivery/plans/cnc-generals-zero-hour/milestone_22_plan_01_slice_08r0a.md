# M22 plan 01 slice 08R0A: strong composite and terrain-prop ownership

Status: planned; ready for implementation after plan-only checkpoint.
Plan transaction parent: `b4fc25b137e94accc1abebd4f0f14bc992c64670`.

## Outcome, dependencies and scope

Depends on accepted 01, 05B2B2B1, 08P0B1, 08Q0R1 and 08Q0; lifecycle composition
consumes accepted 08L2R1 and 08P0C4. See the complete transitive branch inventory
and common acceptance contract in [08R0](milestone_22_plan_01_slice_08r0.md).
Provide strongly exception-safe HLOD/HTree, HModel, Collection and legacy DistLOD
construction/copy/conversion plus exact W3DPropBuffer type/instance ownership.
No prop rendering, ST_SHROUD_TEXTURE lowering, frame acceptance, W3DPropDraw
factory, retail selector or retail admission. R0B then R0C provide those separate
required source owners before active08 continuation.

## Strong graph construction contract

Validate the entire reachable registered prototype graph before construction:
bounded checked counts/byte products, existing 64MiB owner budget, finite geometry,
valid names/bones/parents, maximum 65536 reachable nodes/256 recursive levels,
exact store membership and provider
identity; reject recursive cycles before acquiring refs. Shared DAG references
remain valid with exact multiplicity. Null optional children retain authored
meaning, required missing children fail without publication. Exact prototype kind
must distinguish Aggregate despite its HLOD class ID. Allow Mesh/HLOD/HModel,
Collection, DistLOD, Null and Box only; reject Aggregate/Ring/Sphere/Dazzle and
foreign/unknown kinds before their constructors, including nested kinds. Never
turn a rejected graph into a substitute model/type0.

HLOD/Animatable/HTree guards cover hierarchy pivots/parent fixup, LOD/cost/value
arrays, child clones and container ownership, additional models, proxies, snap
points and name storage. Definition constructors and copy constructors share
the bounded strong contract; don't publish LodCount/raw pointers before complete
storage. Preserve current-LOD selection/clamping, matching flags, LOD bias, child
bone/order, base-pose/reset semantics and native bounds. Constructors must unwind
without relying on an unconstructed most-derived destructor or generic Resize.

Collection builds SubObjects/proxy/name/snap-point and child refs offside, exact
order/identity, then publishes nonthrowingly. DistLOD guards temporary object,
duplicated name, initialized child-ref array, every acquired child and converted
HLOD/HTree until complete success; preserve reversal and containerless conversion
semantics and delete the temporary pointer array. Definition/copy paths retain
all local refs under guards. No fields, virtuals, class layout, serialization or
Windows successful behavior change; no generic container redesign. Existing
ordinary assignment behavior is not broadened unless directly mandatory here.

## Prop admission and lifecycle contract

Explicitly initialize members/arrays/flags; never memset strings or a vptr.
Build light/pass resources under reverse guards. Publish a lazy map-owned buffer
only after complete constructor success. Type admission is case-insensitive,
bounded at exactly 64; bound+1 rejects before mutation and never aliases type0.
Instance capacity is exactly 4000, including native tombstones; bound+1 rejects
with accepted identities/counts unchanged. Missing native optional model yields
no prop publication, not a substitute. New type/name/clone/transform/bounds/ref
publication is atomic, preserving siblings on any interior fault.

Preserve first authored model module, weather/time choice, RotateZ→Scale→translation,
zero and finite-negative scales, finite angle/position, native translated type
sphere (no invented scale-radius correction), ID multiplicity, initial visibility
and INVALID shroud status. Existing type/instance order and type refs are exact.
Update/remove/construction clearing preflight first; remove all matching IDs,
preserve tombstone positions/type cleanup semantics and invalidate source cull
state exactly. Real PartitionManager invalidation remains source-owned.

clear/reset/destructor release every instance ref before every type ref, and
light/pass resources exactly once. Whole-operation phase/provider/generation and
mapping-retirement preflight precedes all destructive terrain/map mutations;
active frame/stale provider rejects owner-intact for idle retry. Destructor work
is nonthrowing and reachable only after preflight or proven-idle construction
rollback. No reset shortcut that destroys unrelated terrain/tree layers. Device
release/recreation preserves logical owner identities and never replays mappings
into a stale generation; A makes no draw/residency claim. Provider removal and
two generations end at exact initialized allocator baseline.

## Generated witnesses and exact commands

Register `original_w3d_prop_owner`, `original_w3d_prop_owner_identity` and
`original_w3d_prop_owner_provider_removal`; independent generated helper/templates,
not retail or a new public selector. Cover direct Mesh, multi-LOD HLOD/HModel,
Collection and converted DistLOD, nested/shared children, hierarchy parent rebinding,
optional arrays and each rejected prototype kind. Inject every allocation/clone/
name/Add/ref publication boundary with immediate exact source/ref/owner residuals,
clean retry and sibling order/identity survival. Cover empty/duplicate/cyclic/
foreign graphs, null providers, checked-byte and exact limits, all 64 types and
type65 rejection, 4000 instances and instance4001 rejection, duplicate IDs,
tombstone/update/clear/construction geometry, snow/night states, zero/negative/
nonfinite scale, bounds/cull/shroud invalidation, provider recreation/removal,
reset/destruction and total teardown. Keep physical prop factory closed.

Build focus:
`cmake --build build/<preset> --target zh_original_w3d_full_probe original_w3d_prop_owner_tests original_w3d_cpu_graph_tests original_w3d_clone_graph_tests -j4`.
Exact ten-control focus:
`ASAN_OPTIONS=detect_leaks=0 ctest --test-dir build/<preset> -R '^(original_w3d_prop_owner|original_w3d_prop_owner_identity|original_w3d_prop_owner_provider_removal|original_w3d_clone_graph|original_w3d_clone_graph_identity|original_w3d_clone_graph_provider_removal|original_w3d_cpu_graph|original_w3d_house_color|original_w3d_house_color_identity|original_w3d_tree_module)$' --output-on-failure -j1`.
Run four focused configurations and both strict host LSan plus six complete
build/canonical, established native Vulkan, serial LAN and ledger/diff gates
defined in R0. A has no new physical prop drawing claim.

## Preconditions, readiness and commit

M22-R0-01 accepted source providers inspected; M22-R0-02 graph/lifecycle code and
three registered tests are A outputs, not entry requirements. PRE-002 tools and
four native presets resolve; existing sanitizers already configured. PRE-012/016
host access is freshly verified at later acceptance; PRE-008 retail not used.
Clean-checkout order providers→A→B→C has no forward dependency or cycle. No
additional mandatory clone owner remains unresolved; checkpoint any new one.

Commit only A sources/helpers/tests/registration, exact ledger rows and A evidence/
plan/index status: `delivery: M22 08R0A own terrain prop graphs strongly`.
Preserve every active08 source/test/ledger hunk and renderer diagnostic unstaged;
when overlap exists, stage only demonstrably separable A-owned hunks.
