# M22 plan 01 slice 08T0R3: bounded camera-startup capacity

Status: accepted; final-source executable and host acceptance complete.
Parent: `cf4ce431adc6aba98a07f2083badb6b905e1501b`.
Plan provenance: exact plan-only commit identified by
`delivery: M22 plan bounded camera startup capacity`.

## Outcome, authority and dependencies

The accepted ordinary camera-startup transaction supports the complete public
bounded HeightMap capacity, including its first full terrain update, without
weakening ordinary device transactions or individual upload limits. This is one
explicitly approved corrective leaf after accepted
[08T0](milestone_22_plan_01_slice_08t0.md) `cf4ce431` (R1/A/R2 and their accepted
terrain, prop, source-stage and device providers), before active08 and09. Keep
accepted history closed. No additional split without architecture checkpoint.

The existing D0 Recording/bgfx idle transaction and T0 typed source rollback own
the behavior. No targeted checkpoint, COW redesign, new terrain producer, native
bgfx patch, general picking/motion/shaker mode, frame admission, asset import,
allocator or source-success semantic change is authorized. Authorization remains
local generated validation and established host gates. Original inputs remain
read-only/private and cannot replace generated acceptance.

## Public audit and exact ceilings

FACT: CPU HeightMap::initHeightData limits each tile axis to32 and each draw
dimension to1025; tile length32, four vertices per cell, and XYZDUV2 stride32
give `tile_bytes = 32*32*4*32 = 131072`. Maximum1024tiles carry128MiB. The camera
currently copies both source backup and CPU bytes for every tile even when no
update is required. Its64MiB check admits255 but rejects256 before mutation.
Initial HeightMap publication retains `m_needFullUpdate=TRUE`, so merely skipping
unchanged buffers cannot close initial source startup.

Source ceiling is a checked compile-time formula, not a magic raised constant:
`1024 * (2*tile_bytes + sizeof(CameraStartupAttempt::Tile)) + sizeof(CameraStartupAttempt)`.
Current Linux64 sizes Tile72/Attempt2080 yield268,511,264bytes;255requires
66,867,160 and256requires67,129,376. Use actual compiled sizes on each target,
cross-check the source stride/tile-axis contract, and validate count/dimensions/
tile-grid consistency before array dereference or allocation. Count1025,
inconsistent dimensions/count/grid, negative/overflow and nonrepresentable
arithmetic reject before backing access. No implicit promotion beyond32x32.

FACT: Recording charges retained baseline payload/labels and subsequent uploads;
bgfx charges twice its complete retained baseline plus reservation and uploads.
Each bgfx terrain buffer has payload-sized `shadow` and `written` arrays. Thus
the worst terrain component is two checkpoint copies of both arrays (4x128MiB),
plus one complete terrain upload (128MiB). Retain an explicitly charged64MiB
allowance for ALL auxiliary resource payloads, metadata, reservation and uploads:

`camera_idle_bytes = ordinary_transaction_bytes + 5*maximum_terrain_payload`
`= 64MiB + 5*128MiB = 704MiB = 738197504bytes`.

This is a ceiling, not a promise arbitrary auxiliary resources fit. Existing
actual accounting must reject when baseline+reservation+operations exceeds the
descriptor or profile limit; unused terrain allowance never exempts auxiliary
charges. Recording keeps its existing accounting (not a manufactured bgfx-sized
charge). Use widened checked/subtraction arithmetic before allocation/replay.
Admission includes retirement/resource-table reservations and exact retained
ownership under the existing device formulas. No native operation can bypass
its existing single-upload64MiB validation.

## Named opt-in capacity and state contract

Add a named `camera_startup` capacity class to the project-owned transaction
descriptor, defaulting to ordinary for every existing aggregate/caller. This
profile is valid only for exact idle-preparation, nonzero generation, zero views
and existing command/resource bounds4096. Unknown class, camera+frame mode,
malformed bounds and cap+1 reject before checkpoint allocation or mutation and
cannot poison/replace an already active owner. Ordinary/default/frame classes
retain64MiB admission and all existing transaction behavior. SDL/unsupported
devices remain fail-closed through their existing capability boundary.

Only the accepted camera attempt opts into this profile in production. Do not
raise RendererLimits::maximum_upload_bytes, B0 selected-stage defaults, frame
journal/native manifest limits, source frame bounds or other consumers. The
source checkpoint uses its separate derived1024-tile ceiling. No engine fields,
virtuals, serialized state, Windows source behavior or original ABI change.
Project renderer descriptor consumers must all rebuild against the same definition.

Preserve exact T0 admission, early-view/later-logic publication, camera math,
lazy cache validity and typed source rollback. Build checkpoints offside, begin
the exact idle device transaction, perform native updateCenter/full tile upload,
commit device, then publish camera/binding. Failed source allocation, device
checkpoint/reservation/upload or late commit restores camera/cache, all tile
backup/CPU bytes, full-update and prop-cull flags, mapping/native identities,
logical token, resources and project RNG. No frame/present is emitted. Same-owner
retry performs the same native update once. Monotonic device diagnostic/tombstone
behavior remains the accepted D0 contract; total initialized teardown is exact.

No-update camera attempts still admit the complete bounded checkpoint/ownership;
they do not upload or clear an already-false full-update flag. Do not discard the
checkpoint or omit type/provider validation to pass the no-update control.
Reset, provider withdrawal/recreation and two complete source/device generations
retain existing token/epoch semantics; no stale-generation retry or raw-provider
dereference is introduced.

## Surfaces and generated acceptance

Expected implementation surfaces: include/zh/renderer/contract.h, Recording/bgfx
device admission, private bgfx transaction-state admission, camera-startup CPU
attempt; existing test_recording_transaction.cpp, test_bgfx_transaction.cpp,
test_bgfx_transaction_resource.cpp and camera_startup_probe.cpp/generated camera
wrapper/map helper as narrowly needed; affected ledger rows, evidence and index.
Use existing registered full-probe/Python map and renderer fixture owners. No
standalone fixture, private mutation, hidden selector or native patch change.
Never stage active08/renderer diagnostic hunks or their ledger bookkeeping.

- Real generated255/256/1024-tile maps: first full update succeeds at native
  source capacity; verify exact CPU/backup/native identity and all-tile content.
  Matching no-update repeats preserve identities/pixels and emit zero uploads.
  Count1025 and mismatched axis/dimension/grid reject before dereference, RNG,
  device or camera mutation. Include partial edge tiles, not just square maps.
- Recording and bgfx capacity-class admission: ordinary64MiB max/+1 unchanged;
  camera704MiB max/+1, zero/unknown/mode/generation/command/resource bounds;
  camera+frame rejection and ordinary frame preservation. Exact combined
  retained-baseline/reservation/upload budget and budget+1 controls independently
  prove actual accounting, including auxiliary/sibling resources. Arithmetic
  boundary controls need not allocate a synthetic704MiB array; real1024-tile
  success and backend combined-charge controls must still reach native owners.
- Full/no-update and early/middle/late checkpoint/upload/commit faults on large
  maps: exact preattempt source/cache/pointer/resource/RNG baseline, no frame,
  accepted siblings unchanged, bounded candidate diagnostic deltas, clean retry.
  Preserve exhaustive existing small-map faults; use deterministic representative
  large-map boundary ordinals, not an unbounded quadratic workload.
- Nested/foreign token, pending/active source frame and provider-removal/stale
  generation reject without mutating the original owner. Reset and fresh second
  generation exercise255/256/1024 bounds and exact initialized teardown.
  Establish each generated allocation baseline after real logical map load and
  before device/camera construction. LinuxTerrainLogic::reset retains height
  vector capacity; native SidesList validation owns its default-player strings.
  Neither is a camera leak. Require zero live camera/terrain/device resources,
  then exact initialized allocation equality after device destruction and
  logical reset, including retirement of device diagnostics/tombstones.
- Validation-enabled physical Recording/bgfx comparison: authored nonblack terrain,
  exact constrained source camera (no override), ordinary native draw order,
  full update and no-update pixel identity, early/late fault unchanged prior pixels
  and identical retry. Test partial-chain mip semantics through accepted R1.
  Keep the existing small-map physical pixel proof;255/256/1024 native maps
  validate the idle camera transaction with unchanged native frame count and
  zero staged frame commands. No max-map full-display admission is claimed.
  An exploratory255-tile continuation passed camera preparation then rejected
  W3DShroud's ordinary64MiB transaction (`original shroud upload transaction
  rejected`). This is a separate downstream active08 boundary; do not promote
  shroud, selected-stage or frame capacity in R3.

## Exact commands and source freeze

Six configurations are linux-gcc-debug, linux-clang-debug, linux-gcc-release,
linux-clang-release, linux-gcc-sanitized and linux-clang-sanitized. Configure each
with `cmake -S . -B build/<configuration>` and run the complete
`cmake --build build/<configuration> -j4`. During edits use existing affected
targets and the narrow camera/renderer controls first. Freeze owned source/test
and selected-binary hashes before final acceptance. No timeout increase without
an evidence-backed architecture checkpoint; preserve workload and TimeoutExpired.

Exact17 CPU controls, verified registered on all six at plan entry:

```text
^(original_w3d_camera_startup|original_w3d_view_scene|original_w3d_terrain_visual_map|original_w3d_terrain_shroud_projection|original_w3d_terrain_map_frame|original_w3d_prop_frame|original_w3d_tree_preparation|original_w3d_tree_draw|original_w3d_tree_module|original_w3d_asset_preload|original_w3d_full_draw_identity|original_w3d_full_draw_provider_removal|renderer_recording_transaction|renderer_bgfx_transaction|renderer_bgfx_transaction_shader_scope|renderer_bgfx_transaction_resource|original_w3d_stage_transaction)$
```

Approved wrapper-only workload correction: the existing30s mission-child bound
cannot cover the added real maximum-capacity camera workload. Diagnostic-only
drivers used the existing public `run(..., timeout_seconds=...)` parameter,
unchanged executable/generated inputs/workload/assertions, isolated serial load,
120s CPU/native-physical and240s sanitizer-physical ceilings. All completed
processes had exact172 typed-fault and255/256/1024/1025/full/no-update/retry/
two-generation/zero-resource markers; physical also had16 small-map pixel-fault
boundaries. All sanitizer and Vulkan validation category counts were zero.
The drivers remain temporary/uncommitted and are not final acceptance evidence.

| Configuration | CPU child seconds (two processes) | Physical child seconds (two processes) |
| --- | --- | --- |
| GCC native | registered44.17s total; separate child times not measured |44.924 /44.872 |
| Clang native |22.497 /22.558 |45.633 /45.667 |
| GCC sanitizer |82.209 /82.402 |184.872 /185.951 |
| Clang sanitizer |77.457 /78.433 |177.737 /176.725 |

Set only the camera wrapper mission-child override to250s:34.44% headroom over
the slowest clean185.951s process. Keep TimeoutExpired behavior, both generation
loops, every assertion and all workload unchanged; do not raise the shared
helper/default/bootstrap timeout. The original30s physical timeout and stopped
incomplete120s diagnostic are superseded non-evidence. The temporary driver's
first successful GCC physical output printed a literal validation-category name
with count zero, triggering the outer substring classifier; raw child category
counts and wrapper checks were zero, and subsequent safe-label runs were clean.
Freeze the final wrapper hash and run all planned final gates under250s.

All four focused configurations use
`ASAN_OPTIONS=detect_leaks=0 ctest --test-dir build/<configuration> -R '<exact17>' --output-on-failure -j1 -V`.
Strict host exact17 on both sanitizer configurations uses exactly
`ASAN_OPTIONS=detect_leaks=1`, no strict UBSan override. Audit sanitizer/validation
categories, not exit status alone; run heavy gates serially under controlled load.

Generated camera physical on both native and both sanitizer configurations:
`ASAN_OPTIONS=detect_leaks=0 VK_INSTANCE_LAYERS=VK_LAYER_KHRONOS_validation python3 tools/run_validation_clean.py python3 tests/original_rendering/test_w3d_camera_startup.py --source-root . --executable build/<configuration>/zh_original_w3d_full_probe --asset-producer build/<configuration>/original_w3d_cpu_graph_tests --gpu`.
Same four run existing renderer resource/journal physical tests with
`ASAN_OPTIONS=detect_leaks=0 VK_INSTANCE_LAYERS=VK_LAYER_KHRONOS_validation ctest --test-dir build/<configuration> -R '^(renderer_bgfx_transaction_gpu|renderer_bgfx_transaction_resource_gpu)$' --output-on-failure -j1 -V`;
their registered commands already invoke tools/run_validation_clean.py.
Use established authorized graphical host escalation; no substitute driver.

Minimal8 both native toolchains:
`ctest --test-dir build/<configuration> -R '^(original_headless_update|original_headless_update_identity|original_headless_update_provider_removal|original_lifecycle_subsystem|original_lifecycle_subsystem_identity|original_lifecycle_spine_objects|original_lifecycle_enum_abi|original_lifecycle_provider_removal)$' --output-on-failure -j1 -V`.
Established validation Vulkan controls: GCC selects
`^(original_w3d_display_owner_bgfx|original_w3d_factory_bootstrap|original_w3d_factory_map)$`;
Clang selects `^(original_w3d_display_owner_bgfx|original_w3d_factory_map)$`.
Serial LAN4 all six:
`ASAN_OPTIONS=detect_leaks=0 ctest --test-dir build/<configuration> -L lan --output-on-failure -j1 -V`.
Six fresh serial canonicals:
`ASAN_OPTIONS=detect_leaks=0 ctest --test-dir build/<configuration> -LE 'gpu|lan|retail' --output-on-failure -j1 -V`.
Check selected counts against CTest JSON; no empty/misquoted selection acceptance.
Run `python3 tools/check_original_dependency_ledger.py --root . --ledger docs/original-runtime-dependency-ledger.tsv`, owned/whole/staged diff, final hash,
ABI/default-descriptor and privacy audits. No reuse of R2 executable gates after
this production change; historical acceptance remains evidence, not fresh R3 proof.

Only after generated focus/physical gates pass, run the existing unchanged
120-second redacted scene-once wrapper with privately recovered provisioning.
Require the camera checkpoint capacity guard is crossed; classify any next owner
read-only before behavioral changes. Fixed public categories only; no original
names/paths/bytes/hashes/raw logs/images. This is guard closure, not retail acceptance.

## Preconditions, readiness and commit boundary

M22-T0R3-01 accepted T0/D0/terrain/prop providers are verified ancestors; -02
named capacity/source controls are this leaf's implementation output; -03 tools,
generated fixture and exact17 registrations are entry-ready; -04 established
host gates are fresh acceptance prerequisites; -05 preserved privacy/worktree
is mandatory before staging/continuation. Readiness checks: ancestor/index/source
formula/default audit, tool/preset and six CTest JSON verification, fourteen-path
hash/empty-index review. No new M0, package, external authority or unresolved owner.

First exact-stage/review/commit only new R3 plan, governing index, prerequisite
manifest and readiness report as `delivery: M22 plan bounded camera startup capacity`.
Then implement/validate one independent leaf and commit only owned paths/hunks as
`delivery: M22 08T0R3 admit bounded camera startup capacity`. Acceptance requires
all above exact positive/negative/fault/reset/provider/two-generation/physical
controls and clean final gates. Only then resume active08. No extra aggregate or
historical T0 rewrite is required for this single post-T0 correction.
