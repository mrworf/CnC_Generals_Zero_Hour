# M22 plan 01 slice 08T0A: bounded native camera startup

Status: implementation-ready; plan-only checkpoint precedes executable work.
Plan transaction parent: `b6f95598fde2d9b8f5b8ec68bcc9b20290ee71cb`.
Plan provenance: exact commit `delivery: M22 plan bounded camera startup`.

## Outcome, dependencies and authority

The public new-game camera sequence uses the original ordinary default angle,
pitch and zoom, constrained nonmoving transform, initial ground/elevated lookAt,
and map-height setup. The accepted camera then drives the existing stationary
terrain/shroud/scene frame without adapter camera replacement. One coherent
corrective leaf is approved, followed by evidence-only 08T0, before active08.
No additional split or owner admission without an architecture checkpoint.

Requires accepted 08S0 (`b6f95598fde2d9b8f5b8ec68bcc9b20290ee71cb`),
07D/07E0C/07FA camera/display/traversal, 08I1/I2 bounded terrain tiles,
08P0C2D0 idle/device rollback, 08P0C2B shroud, 08P0C2D frame rollback,
and 08R0A/C prop lifecycle/checkpoint. These are providers, not reopened slices.
Preserve the same thirteen active08/renderer dirty paths and shared ledger/engine
hunks. Original corpus/symlink are read-only; generated owned inputs alone supply
this leaf's behavior witnesses. Authorization/security changes are not applicable:
this is the existing local source-engine path, with no new external access.

## Fixed discovery and finite public owner graph

FACT: after accepted preload, the first public stack is
GameLogic::startNewGame:2531 → W3DView::setAngleAndPitchToDefault:199. The CPU
method throws the fixed tactical-pending category. A bounded read-only diagnostic
returns within the unchanged120-second wrapper deadline; wrapper teardown and
corpus-unchanged checks pass. This is not retail scene acceptance or an S0/R0
regression. No input metadata, raw output, coordinates or images are retained.

FACT: source order is defaults/zoom → Recorder initialization → initial waypoint
or native fallback lookAt → initHeightForMap → defaults/zoom → PartitionManager
update. Preserve it; do not move shroud upload or frame publication before source
object construction. Recorder/partition and later producers are not opened here.

| Owner | Native success / reached mutation and fallibility |
|---|---|
| GlobalData / View / W3DView | Global cameraHeight/pitch/yaw produce offset Z, -Z/tan(pitch), -Y*tan(yaw). View defaults supply angle/pitch/max height; default zoom uses maximum of center/four ±40 ground samples plus maxHeight divided by offset Z, without ordinary setZoom clamping. initHeightForMap caps ground above120 only. |
| LinuxTerrainLogic | Actual runtime provider owns logical height/extent reads; it is not native W3DTerrainLogic. Preflight finite/representable query coordinates before its Real→Int conversion. Preserve its accepted sampling/border/extent semantics. |
| Camera transform/constraints | Ordinary offset×zoom; native pitch then angle rotation, ground factor, Look_At, near MAP_XY_FACTOR and native far-plane rule. Constraint center/95%-height rays use source clamp order, including narrow extents. Debug/internal constraint/FOV conditions remain source-defined. |
| Elevated lookAt | Native camera center ray is translated to the elevated target, clipped/refined against the heightfield, then tests the source ordered cell triangles with clipped border sampling. Ground lookAt bypasses this; both publish position Z=0. Current CPU Cast_Ray is a false stub. Use a narrowly bounded native heightfield helper, not general picking admission. |
| CameraClass / RenderObj | Transform/identity/bounds bits, clip/view plane and lazy world/view frustum, near-clip box, inverse/projection state change. Existing camera copy/assignment invalidates caches and is not an exact rollback primitive. |
| HeightMap / Edge / props | setCameraTransform creates a lights iterator and calls updateCenter after camera mutation. updateCenter may mutate prop full-update state, terrain backup/VB bytes, full-update flag and source→native mappings; upload/allocation can throw. Existing display frame checkpoint captures camera userdata only and starts later. |
| Reset/teardown | Preserve published view/camera ownership and existing initialized reset behavior; tear views down before scenes/device. No retained attempt/pin or cross-generation retry after teardown. Timed resetCamera/motion remains unavailable. |

Empty CameraShakerSystem has no random/provider effect; active shaking consumes
RNG and mutates/deletes nodes, so reject it before work. No independent owner was
identified in the approved ordinary nonmoving graph.

## Exact admission, state and rollback contract

Require exact initialized singleton display/view/cameras, registered terrain/map,
logical terrain, GlobalData, scene and matching live Edge/generation. Reject
foreign/detached/stale/removed providers, malformed bounds or metadata, absent
map at map-only entry, selected-stage attempts, pending/active source frames or
rendering before dereference/mutation; guard/poison an already-live source attempt
as required by accepted Edge semantics. Reject nonfinite inputs/intermediates,
nonrepresentable ground-query coordinates, invalid viewport/clip values,
zero/degenerate normalization or trigonometric divisors before publication.

Keep bounded value state explicitly initialized, including position Z, camera
offset and every newly read motion flag; legacy empty vector constructors and
implicit padding must not be treated as initialization. No fields, virtuals,
layout/serialization, generic container or Windows success-path changes.

Build the admitted camera result offside and capture typed state before mutation.
Use Linux-only owner-local friendship/access where needed, not opaque class-byte
copies or synchronizing readers. Capture exactly reachable View/W3DView fields,
CameraClass/RenderObj transform/bounds/clip/viewplane/projection/frustum/cache,
terrain full-update/backup/VB bytes and identities, Edge mapping identity and
prop full-update/checkpoint state. Allocate all checkpoint storage before change;
enforce accepted terrain/resource bounds and a64MiB aggregate checkpoint ceiling.
The heightfield helper checks every extent/border/index/ray arithmetic and bounds
the complete native loop work to4194304 cell visits before collision work.
Exact maximum/bound+1 rejection is a generated control, with no partial output.

Compose with the accepted idle resource transaction for any source-buffer
create/upload/rebind; no live frame journal or present is retained. Iterator
ownership is guarded on every exit. Success publishes the admitted camera and
terrain result together; any allocation/iterator/query/lock/upload/mapping/commit
failure restores exact accepted source state and live resources, preserving
siblings, shroud content/epoch and retry identity. Restore is allocation-free,
noexcept and invokes no producer callbacks/setters. Respect existing monotonic
device diagnostic/tombstone counts with exact bounded deltas separately; total
device/Edge/process teardown returns the initialized allocator baseline.

Existing zero-position generated fixtures and no-map update remain compatible.
After successful startup, updateView admits the exact ordinary stationary camera
identity/finite state at nonzero position; it must not advance camera animation,
C1, project/audio RNG or FX, and keeps source shroud→terrain-center→scene order.
No camera change after accepted immutable phase publication is admitted.

Excluded: active shake/oscillation, object follow/lock, scripted/waypoint movement,
slave/bone cameras, real zoom, automatic animated height/scroll update, advanced
filters/wireframe/letterbox, multiple views, timed resetCamera and general picking.
Reject these before mutation; do not substitute a generic camera or no-op pass.

## Implementation and generated controls

Expected owned surfaces: W3DView source/header, narrowly typed CameraClass and
HeightMap/BaseHeightMap checkpoint/helper surfaces, reached prop checkpoint state
only if required, OriginalGpuEdge existing idle composition, CMake registration,
full-probe engine extern/dispatch hunks, new camera_startup_probe.cpp and
test_w3d_camera_startup.py, owned ledger rows, plan/evidence/index. No new selector
on the shipping camera path; generated profile dispatch only invokes that path.
Use the existing full-probe/Python generated-map process/device owner, not a new
standalone fixture. Keep two external/two internal generations and explicit
TimeoutExpired; bounded wrapper workload is measured before any bound adjustment.

- Run the exact source startup method sequence on flat, varied-height/border,
  shifted/nonzero and narrow-boundary maps; compare independent native formula,
  defaults, zoom, constraints, transform/clip/frustum and height-cap results.
- Ground and elevated lookAt: hit/miss, clipped borders, native triangle order,
  degenerate/nonfinite/bound+1 inputs; exact failed-output and accepted sibling
  identity. No initial zero-Z or successful elevated branch may be skipped.
- Every reachable allocation/query/iterator/VB lock/create/upload/rebind/commit
  fault: exact prior camera caches, full-update flags, backup/VB/mapping bytes,
  prop/shroud state, refs/live native resources; deterministic same-owner retry.
- Wrong/stale/removed providers/map/device generation, pending/active journal,
  live selected-stage attempt, empty-map versus map-required calls, checkpoint/
  ray capacity max/bound+1, malformed state and every excluded mode reject before
  source/RNG/GPU/registry/phase mutation. Minimal/headless remains independent.
- Stationary repeated frames at accepted nonzero camera position preserve state,
  native shroud/terrain/scene order and sibling identities. Recording output and
  physical Vulkan pixels demonstrate the same source camera, not manual fixture
  camera replacement; fault then retry yields identical commands/pixels.
- Reset/free/provider removal/recreation, two generations, exact once-only release
  and raw/DMA/native total teardown. No stale retry or callback after teardown.

## Exact acceptance commands and privacy gate

New registered output ID: original_w3d_camera_startup. Exact12 union:

```text
^(original_w3d_camera_startup|original_w3d_view_scene|original_w3d_terrain_visual_map|original_w3d_terrain_shroud_projection|original_w3d_terrain_map_frame|original_w3d_prop_frame|original_w3d_tree_preparation|original_w3d_tree_draw|original_w3d_tree_module|original_w3d_asset_preload|original_w3d_full_draw_identity|original_w3d_full_draw_provider_removal)$
```

Verify all12 registrations after implementation. GCC/Clang debug and both
sanitizers: ASAN_OPTIONS=detect_leaks=0 ctest --test-dir build/<preset>
-R '<exact12>' --output-on-failure -j1 -V. Strict host LSan: same exact12,
exactly ASAN_OPTIONS=detect_leaks=1, no strict UBSan override, established host
escalation. Serialize heavy sanitizer runs and audit complete category logs.
Generated physical all four focused configurations:
`ASAN_OPTIONS=detect_leaks=0 VK_INSTANCE_LAYERS=VK_LAYER_KHRONOS_validation python3 tools/run_validation_clean.py python3 tests/original_rendering/test_w3d_camera_startup.py --source-root . --executable build/<preset>/zh_original_w3d_full_probe --asset-producer build/<preset>/original_w3d_cpu_graph_tests --gpu`.

Refresh configure-time overlays with cmake -S . -B build/<preset>, complete all
six builds using cmake --build build/<preset> -j4, freeze owned source/test and
selected executable hashes. Six fresh serial canonical suites:
ASAN_OPTIONS=detect_leaks=0 ctest --test-dir build/<preset> -LE 'gpu|lan|retail'
--output-on-failure -j1 -V. Preserve exact established native Vulkan GCC3/Clang2,
minimal8 both and LAN4 all six commands in accepted S0's common acceptance;
freshly execute them, not infer success. Run dependency-ledger, ABI/linkage,
whitespace/privacy/preserved-hash/exact-staged checks. No acceptance claims from
this plan-only checkpoint.

After A commit and separate evidence-only T0 closure, rerun only the existing
read-only categorized scene-once retail wrapper with runtime-private roots and
unchanged120-second bound. Corpus-unchanged and teardown contracts must hold;
no roots/selections/names/bytes/hashes/raw logs/images enter artifacts. Crossing
camera startup is not full retail acceptance. A new owner stops for read-only
classification and architecture approval; do not extend this leaf implicitly.

## Preconditions, readiness and commit boundary

M22-T0-01 accepted providers/S0 are verified ancestors; M22-T0-02 camera code/
witness is consuming-slice output, not entry prerequisite. M22-T0-03 existing
toolchains/full-probe/generators and eleven prior focus registrations are verified;
the new twelfth ID is implementation work. M22-T0-04 existing host facilities are
fresh later gates; M22-T0-05 private inputs are only later active08 continuation.
No service/package/credential/M0 or unresolved architecture prerequisite.
Readiness checks: exact S0/R1 ancestry, cmake --list-presets/tool resolution,
parsed six CTest manifests and public source/typed-owner inspection. Stage only
owned implementation/ledger/plan/evidence/index hunks after clean final gates.
Implementation subject: `delivery: M22 08T0A own bounded native camera startup`.
