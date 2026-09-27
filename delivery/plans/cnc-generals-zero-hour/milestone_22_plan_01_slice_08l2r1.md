# M22 plan 01 slice 08L2R1: symmetric construction binding rollback

Status: ready; dependency-first corrective owner approved at the slice08 architecture checkpoint.
Parent checkpoint: `72751dcb55bbdddc914b8187ceedaba2c5ccd42c`.

## Outcome, dependencies and scope

Depends on accepted 08L2. Object/Drawable construction either publishes one
coherent pair or withdraws both sides and all construction registries before
Object constructor cleanup. A callback exception must not leave a registered
Drawable pointing at freed Object storage; reset and retry remain safe.
This is a Linux construction/binding correction, not recoloring, a new draw
provider, gameplay destruction, shadow admission or retail acceptance. Existing
slice08 trial edits and the unrelated renderer diagnostic remain unstaged.
No authorization change applies to generated process-local construction.

## Source evidence and state contract

`GameLogic::sendObjectCreated` owns the newly registered Drawable.
`bindObjectAndDrawable` currently invokes `Drawable::friend_bindToObject`
before `Object::friend_bindToDrawable`. The former installs `m_object` before
indicator-color and draw-module callbacks; the latter installs `m_drawable`
before model-condition and behavior callbacks. A throw in the first half leaves
Object construction rollback unable to find the Drawable. Subsequent client
reset dereferences the stale Object through ordinary unbinding. This is a
public-source lifetime defect independent of the callback's recolor failure.

Preserve successful callback order. Snapshot both exact prior bindings before
the attempt; on any callback failure restore both pointers directly, without
setters, notification callbacks or allocation. Do not add serialized fields,
virtual slots or a Windows ABI change. The `sendObjectCreated` owner then
withdraws the newly constructed Drawable/client registry/module ownership
before rethrowing to Object's existing reverse construction cleanup. Generic
binding failure restores caller-owned prior identities without deleting them.
Rollback must not run behavior binding callbacks on a partially constructed
or already-cleared Object. Reject foreign/inconsistent pair inputs before
mutation; retain the existing successful source callbacks and destruction rules.

## Implementation and controls

Expected surfaces: GameLogic, Object and Drawable headers/sources; the existing
generated construction witness and narrowly scoped generated-only callback
fault seams; ledger, this plan/index and adjacent evidence. Do not stage any
slice08 trial source/test hunk. Use exact owned hunks if a shared file overlaps.

Cover exceptions before/after indicator-color notification, each drawable
binding notification, model-condition notification and behavior binding
notification, plus existing module `onObjectCreated`, construction and registry
faults. Include callbacks that actually throw on owned generated data, not only
an after-success injection. Assert both pointers, intrusive list heads, lookup
IDs, object/client/partition/radar/team/module ownership and Recording resources
at the immediate rollback boundary before reset, then total-zero teardown.
Prove clean retry after every fault, two source generations, no duplicate
notification on cleanup, no-model regression, and required-provider removal.

Configure all affected overlays before building. Focus build:
`cmake --build build/<preset> --target zh_original_w3d_full_probe zh_original_main -j4`.
Exact native and sanitizer focus:
`ASAN_OPTIONS=detect_leaks=0 ctest --test-dir build/<preset> -R '^(original_w3d_generated_construction|original_w3d_generated_scene_boundary|original_w3d_borrowed_file_owner|original_w3d_full_draw_identity|original_w3d_full_draw_provider_removal|original_headless_update|original_simulation_map|original_simulation_reentry)$' --output-on-failure -j1`.
Verify registration and audit complete sanitizer output. Strict host LSan uses
`ASAN_OPTIONS=detect_leaks=1` and the same selection on both sanitizer toolchains,
without a UBSan override. Before commit run six complete builds and six serial
canonical nonretail suites `-LE 'gpu|lan|retail'`, established native physical
Vulkan controls, serial LAN4/4 all six, and ledger/header/diff review. These
gates include the frozen unstaged trial composition and do not accept slice08.

## Acceptance and commit boundary

Every admitted success and every callback rejection preserves exact ownership;
no stale dereference, sanitizer finding or rollback residual remains. R1 is one
independent implementation/evidence commit:
`delivery: M22 08L2R1 roll back symmetric drawable binding`.
Q0 and retail continuation remain closed until R1 is accepted.
