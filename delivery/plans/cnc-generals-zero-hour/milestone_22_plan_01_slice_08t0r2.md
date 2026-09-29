# M22 plan 01 slice 08T0R2: deferred exact logical-terrain publication

Status: accepted in this independently validated implementation commit.
Plan transaction parent: `8e13acdc410ef0cf6b4bd5ab3c0fb9758739c698`.
Plan provenance: exact commit `delivery: M22 plan deferred camera terrain publication`.

## Outcome, authority and dependencies

The real native early-view/later-logic initialization order can perform the
accepted bounded ordinary camera startup without freezing a null terrain-logic
identity. Missing, foreign, removed or stale providers still reject before
dereference or source/device/RNG mutation. This is exactly one approved narrow
corrective leaf after accepted [08T0R1](milestone_22_plan_01_slice_08t0r1.md)
`cbb65e0924ba3c1b6660e6c3193717e9e9a4a123` and
[08T0A](milestone_22_plan_01_slice_08t0a.md)
`c62160a8964291a2e218b915bbc7ccf7cb0af0e6`, before refreshed evidence-only
[08T0](milestone_22_plan_01_slice_08t0.md), active08 and09. Historical accepted
commits/evidence are not rewritten. No further split without architecture authority.

Existing GameLogic/TerrainLogic lifecycle, typed camera/terrain idle transaction,
view/display/map registration, accepted allocator and minimal/full-probe test
owners are the providers. No new subsystem, package, credential, service,
external authority or retail producer is admitted. Authorization is unchanged
local generated validation plus existing host gates; original inputs stay private
and read-only and cannot substitute for generated acceptance.

## Audited public lifecycle and defect

FACT: GameEngine::init publishes/initializes GameClient before GameLogic
(`GameEngine.cpp:537/549`). InGameUI creates, initializes and attaches the native
W3DView (`InGameUI.cpp:1166–1168`) while TheTerrainLogic is still null.
GameLogic::init then creates the provider, initializes it and names it
(`GameLogic.cpp:414–416`). T0A's W3DView::init provider sidecar captures that
earlier null identity permanently. The generated camera fixture initializes its
secondary view after logical terrain already exists and misses this source order.

FACT: the unchanged bounded redacted continuation completes init3/logic5/mapINI4
and requested-stage1, then rejects before a Recording frame. Public stack:
GameLogic::startNewGame:2531 → setAngleAndPitchToDefault → applyCameraStartup
→ CameraStartupAttempt::admit:144 → cameraRequire:46. Fixed identity predicates
owner/camera/display/scene/Edge/generation match; only logical terrain differs,
with captured-null and current-present true. Clean teardown and no sanitizer
category are observed. No input metadata or raw process/debugger output persists.

FACT: resetAll visits subsystems in reverse order. GameLogic resets the same
TerrainLogic object before GameClient/display/view reset; it does not recreate
the provider. shutdownAll and init-failure cleanup delete GameLogic/TerrainLogic
before GameClient → terrain visual → display → views. GameLogic's destructor
clears logical terrain after deleting it. Base TerrainLogic::init is empty;
LinuxTerrainLogic preserves its existing logical-map/reset ownership.

Existing Linux reset/terrain/preload capabilities use raw owner identity and
monotonic exact tokens without class fields or WW3D linkage. None currently
observes logical-terrain publication. An allocation-free source publication and
callback-free peek are sufficient; no full-client callback belongs in GameLogic.

## Exact publication, admission and rollback contract

Add Linux-only GameLogic-owned sidecar publication of the exact initialized
TerrainLogic pointer, raw GameLogic owner and nonzero monotonic publication token.
Publish only after successful provider init under the real owning source path;
clear the exact publication before provider deletion, including init-unwind.
Reject duplicate/foreign/mismatched publication/removal and token exhaustion before
changing accepted metadata. No ownership ref, allocation, diagnostic formatting,
callback or WW3D/full-client dependency occurs in publication/peek/withdrawal.
No fields, virtuals, layout/serialization or Windows behavior change.

The callback-free peek compares raw owner/provider/global identities before any
typed conversion/dereference. Fresh/uninitialized/removed state returns no admitted
publication, not a fabricated provider. A null/headless consumer remains no-op;
ordinary GameLogic init/reset/shutdown successful semantics remain unchanged.
Pointer address reuse cannot alias a retired token. Publication token is a source
lifetime identity; keep it distinct from and require the existing Edge generation.

W3DView may remain unbound when initialized before logical terrain. The first
camera attempt may admit only the explicit source publication, never an arbitrary
first-use global or dynamically guessed class. All existing display/view/camera/
terrain/map/scene/Edge-generation and mode guards remain mandatory. Build the
candidate logical-provider binding/token offside and publish it nonthrowingly only
after the existing complete camera/terrain idle transaction commits successfully.
Any preflight/query/allocation/upload/late-commit failure leaves the prior binding
or unbound state unchanged; retry uses the same source publication exactly once.

After binding, every camera entry validates exact pointer/owner/publication token
and Edge generation before dereference. Provider removal or replacement cannot
silently rebind an accepted view. Reset retains the same source provider/token
while clearing accepted camera state through existing reset semantics; a destroyed
or recreated source lifetime requires fresh view lifecycle, not cross-generation
retry. Teardown withdrawal precedes logical-provider destruction and remains safe
while W3DView outlives GameLogic. View removal retires only its own binding state.

Preserve T0A formulas, lazy camera cache validity, constraints, ground/elevated ray,
typed rollback and all motion/shaker/picking/filter/UI exclusions. No camera math,
sampler/backend, generic registry, destructor notification or allocator redesign.

## Implementation surfaces and generated controls

Expected owned surfaces: Linux-only declarations/source-local publication in
GameLogic.h/GameLogic.cpp; W3DView's private startup provider/attempt sidecars;
existing camera_startup_probe.cpp/test_w3d_camera_startup.py and narrowly required
minimal lifecycle controls; corresponding source-ledger rows and evidence/index.
Keep all active08 and unrelated test_bgfx_device.cpp hunks unstaged. Shared ledger
and engine hunks must be isolated exactly; no trial bookkeeping is absorbed.

- Genuine native initial view before GameLogic/TerrainLogic publication: peek is
  empty, early camera admission rejects without mutation, successful initialized
  late publication admits the ordinary full startup sequence. Keep the existing
  view-after-logic fixture as an independent success regression.
- Failed provider init/publication and every first-camera fallible boundary:
  no partial binding, exact source/cache/terrain/Edge/ref/resource/RNG baseline,
  clean same-owner retry; successful binding occurs once after full commit.
- Missing/null/foreign/malformed owner/provider/global, wrong/stale publication
  token, Edge generation, duplicate publication/removal and token overflow:
  callback-free zero-mutation rejection before dereference; exact accepted sibling.
- Provider removal before view destruction, fresh address reuse with a new token,
  rejected stale camera attempt, exact withdrawal and retry after legitimate fresh
  lifecycle; failed teardown preconditions preserve owner identity.
- Reset preserves the provider token, clears the declared camera accepted baseline
  and allows the same source sequence; full source teardown clears publication
  before provider destruction. Two complete generations and total initialized
  allocator baseline. No orphan sidecar or cross-generation retry.
- Minimal/headless absence/no-op and compile/link removal prove core GameLogic
  does not link WW3D/full-client symbols. Native camera/terrain physical pixels
  and typed late-fault identical retry remain unchanged under validation.

## Exact commands, freeze and acceptance

Reuse the existing full-probe/Python generated-map owner and registration, not a
new standalone fixture or selector. Exact12, verified registered on all six:

```text
^(original_w3d_camera_startup|original_w3d_view_scene|original_w3d_terrain_visual_map|original_w3d_terrain_shroud_projection|original_w3d_terrain_map_frame|original_w3d_prop_frame|original_w3d_tree_preparation|original_w3d_tree_draw|original_w3d_tree_module|original_w3d_asset_preload|original_w3d_full_draw_identity|original_w3d_full_draw_provider_removal)$
```

Refresh each of six configurations (GCC/Clang Debug/Release/sanitized) with
`cmake -S . -B build/<preset>` and complete `cmake --build build/<preset> -j4`.
During implementation build the existing full-probe/producer and affected minimal
targets and use the narrow camera test first. Freeze exact source/test/selected
binary hashes before final gates; changed bytes invalidate affected gates.
Run exact12 four focused configurations with
`ASAN_OPTIONS=detect_leaks=0 ctest --test-dir build/<preset> -R '<exact12>' --output-on-failure -j1 -V`.
Strict host same12 both sanitizers uses exactly `ASAN_OPTIONS=detect_leaks=1`,
no strict UBSan override. All heavy tests/suites run serially with category audits.

Generated camera physical route on both native and both sanitizer configurations:
`ASAN_OPTIONS=detect_leaks=0 VK_INSTANCE_LAYERS=VK_LAYER_KHRONOS_validation python3 tools/run_validation_clean.py python3 tests/original_rendering/test_w3d_camera_startup.py --source-root . --executable build/<preset>/zh_original_w3d_full_probe --asset-producer build/<preset>/original_w3d_cpu_graph_tests --gpu`.
Use established authorized graphical host escalation, no alternate driver/camera.

Minimal8 both native toolchains:
`ctest --test-dir build/<preset> -R '^(original_headless_update|original_headless_update_identity|original_headless_update_provider_removal|original_lifecycle_subsystem|original_lifecycle_subsystem_identity|original_lifecycle_spine_objects|original_lifecycle_enum_abi|original_lifecycle_provider_removal)$' --output-on-failure -j1 -V`.
Established Vulkan controls under the same validation wrapper: GCC selects
`^(original_w3d_display_owner_bgfx|original_w3d_factory_bootstrap|original_w3d_factory_map)$`;
Clang selects `^(original_w3d_display_owner_bgfx|original_w3d_factory_map)$`.
Serial LAN4 all six uses `ASAN_OPTIONS=detect_leaks=0 ctest --test-dir build/<preset> -L lan --output-on-failure -j1 -V`.
Six fresh serial canonicals use `ASAN_OPTIONS=detect_leaks=0 ctest --test-dir build/<preset> -LE 'gpu|lan|retail' --output-on-failure -j1 -V`.
Run `python3 tools/check_original_dependency_ledger.py --root . --ledger docs/original-runtime-dependency-ledger.tsv`, exact owned/source/staged/diff/ABI/privacy
audits and final hash checks. Exit status alone is not sanitizer/validation proof.

After generated focus/physical gates are green, run only the existing unchanged
120-second redacted scene-once wrapper with private runtime roots. Require the
logical-provider camera guard is crossed; a separately classified next source
stop is discovery, not retail acceptance. No timeout increase, private metadata,
raw log, image or retail names/paths/bytes/hashes may persist. A new owner stops
read-only for architecture checkpoint before behavioral changes.

## Preconditions, readiness and commit boundary

M22-T0R2-01 accepted R1/A/core providers are verified ancestors; -02 publication/
binding is this leaf's output; -03 tools/generator/test registration is entry-ready;
-04 existing authorized host facilities are fresh later acceptance gates; -05
preserved active08/private boundary is exact-staging/continuation prerequisite.
Readiness: inspect public lifecycle/order, ancestry, raw identity contracts,
six test manifests, tools/presets and worktree ownership. No unresolved owner or
M0 is required. Acceptance is exact generated positive/negative/fault/retry/reset/
removal and physical controls, clean categories and total teardown plus all gates.

Exact-stage/review/commit this plan-only packet before production work:
`delivery: M22 plan deferred camera terrain publication`.
Then one independently validated implementation commit:
`delivery: M22 08T0R2 publish deferred camera terrain identity`.
Refresh T0 in a separate evidence-only aggregate using unchanged accepted R2
bytes/gates; only then resume active08. Do not rewrite accepted R1/A/T0 history.

## Implementation checkpoint

Plan packet committed as `075c827f4b1f81ad36430b2b2c1a84833a81557d` before
production work. Finite owner/ABI audit remains inside the nine planned source/test
surfaces; core publication is BSS constant-initialized with no guard symbol and no
WW3D linkage. The existing camera registration additionally drives genuine native
early-view/later-logic bootstrap through existing generated profiles. Headless
runtime/identity/provider-removal3/3 and corrected camera1/1 pass. Frozen native
exact12 is12/12 GCC/Clang,316.68/320.60s, category-clean; retain it after complete
builds only through exact selected binary identity, otherwise rerun. All six
complete builds pass and both native focus binary pairs are unchanged. Fresh
minimal8 passes8/8 on both native toolchains; GCC sanitizer exact12 passes12/12
in1359.97s and Clang sanitizer exact12 passes12/12 in1182.03s without sanitizer/
validation categories. Frozen source/selected binary identities still match.
Strict GCC/Clang pass12/12 in1362.56/1183.28s without sanitizer/leak categories.
Generated validation physical4, established Vulkan GCC3/Clang2 and serial LAN4
all six pass with clean category audits. All six fresh canonical suites pass
297/297 with complete clean category audits and unchanged frozen source/binary
hashes. Ledger, finite ABI/source, preserved-worktree and exact staged audits pass.
The unchanged bounded redacted continuation crosses the logical-publication guard
and stops at the existing T0A camera terrain-checkpoint 64MiB capacity predicate,
before a Recording frame. Public stack classification proves admission returned;
this is a separate active08 capacity boundary, not an R2 publication defect.
Only fixed categories/public owners are retained, with clean teardown. No capacity,
allocation, timeout or behavior change is authorized here. See the [final evidence](../../evidence/cnc-generals-zero-hour/milestone_22_slice_08t0r2.md).
No prior provider acceptance substitutes for R2 final gates. Refresh T0 separately
with these unchanged accepted bytes/gates before active08 architecture work.
