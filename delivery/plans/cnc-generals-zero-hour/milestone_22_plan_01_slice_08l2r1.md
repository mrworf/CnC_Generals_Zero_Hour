# M22 plan 01 slice 08L2R1: symmetric construction binding rollback

Status: complete; accepted corrective owner in `delivery: M22 08L2R1 roll back symmetric drawable binding`.
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
or already-cleared Object. Reject null and foreign-owner inputs before
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

## Implementation and source-freeze checkpoint

Plan-only dependency packet `a734c444` preceded executable changes. Linux-only
GameLogic/Drawable friendship permits direct exact pointer restoration; no
public setter, class field, virtual slot or serialized/Windows declaration was
added. Successful source callback order is unchanged. GameLogic's creation owner
withdraws its new Drawable on binding rejection; Drawable construction cleanup
clears the exact reverse link directly instead of notifying behavior modules.
The generated construction fixture now contains a real default draw module and
separate fake-structure input. Its 52 faults cover indexed object-created/draw/
behavior callbacks and indicator/model-condition/decal boundaries; every binding
fault observes restored pointers before registry/module cleanup. Completed
generated maps also prove null/foreign rejection and prior-pair rollback through
the public GameLogic binding entry without changing either accepted registry head.
Two-mode/two-generation success, immediate residual-zero rollback and clean
per-fault retry pass. Exact focus passes8/8 all six configurations, strict host
LSan passes8/8 both sanitizers, established native Vulkan passes GCC3/3 and
Clang2/2, serial LAN passes4/4 all six, and canonical nonretail suites pass
280/280 all six with complete sanitizer logs category-clean.

Frozen R1 executable/test source identities (SHA-256, public project source only):

- Drawable.h: `44436e7ccbbb0970a3fcff179f0999e4d2a86369465fc7a91a0a27bbab789fce`
- Object.h: `e6af240b93ba9483fe072b60d4f0848c6bf4b3a9b125bd21c08bd36563bbdbdc`
- Drawable.cpp: `ea6b745b9a54f86d3308a2da7aefef85e022495e861f04e5c9b36f2239a9b315`
- Object.cpp: `82fbb6aa16d4954d1f0bdae42e5f0df7cc379562a4dd38cc7a2db8983ab1f031`
- GameLogic.cpp: `0f2e2ca38863fe01266cd94091254cd9278bbb3e60a2f4e828a39114a6829357`
- generated construction wrapper: `7bad6caff045bed71981b293f7a229ec7d6b29677906033e503e728db1499451`

The final wrapper explicitly rejects captured ASan/LSan/UBSan diagnostics before
discarding generated subprocess output, including recoverable findings on an
otherwise successful exit. Executable identities above did not change; GCC focus
is refreshed and all remaining gates consume this final test identity.

Six complete builds/focus and later canonical/host gates include the unchanged
unstaged active08 executable/test composition; they do not accept that trial.
Only the three R1 source ledger rows are owned by this corrective commit; the
four trial-only hash refreshes required for worktree ledger validation remain
unstaged together with their source edits. Preserve the unrelated renderer test.

Final acceptance: [binding rollback evidence](../../evidence/cnc-generals-zero-hour/milestone_22_slice_08l2r1_binding_rollback.md).
