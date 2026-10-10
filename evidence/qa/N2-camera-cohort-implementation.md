# N2 camera frame cohort — checkpoint evidence

Parent: `31ccae86`. Initial worktree clean. Existing N2 plan01/slice03 refined
before production changes. This artifact is iteration recovery, not milestone,
root, production model decoding or visual acceptance. No framework/retail/recovery changes.

## Current implemented surfaces

- NativeCamera explicit world pose, general-inverse projection separate from raw
  world unprojection, source Look_At basis and ordered local XYZ rotations.
- Actual native terrain extent constraint query and source low-then-high XY
  clipping, odd viewport/origin truncation and retained ground-Z border.
- Root-owned shared NativeCameraShaker C3 records: original six admission/nine
  in-range secondary visual RNG draws, source degree arithmetic, sinusoid/time
  decay and newest-first accumulation. Stage capacity before RNG admission;
  expire without fallible temporary list. No discarded source clamp redesign.
- Positive/negative/retry/zero-allocation tests, original Matrix3D column oracle,
  analytic scaled/sheared inverse, actual Object/terrain parent composition, C3
  formula/RNG and whole-owner capacity retirement tests.

NativeFrameCameraView now implements the original camera phase: follow/tether/
snap/chase, invalidation and one/two-build cardinality, terrain-height settlement,
legacy shake, shared C3 ownership and first-interface slave-bone resolution.
Const spatial queries read accepted backing without model/RNG/time callbacks.
Physical pick/iterate/draw remain abstract adapter obligations, not empty success.
The generated world fixture composes actual parsed GameData, terrain, Object,
Drawable, ScriptEngine and original pooled draw modules. Generated model metadata
tests selection and transform semantics, not proprietary model decoding.

Frozen source recipe: changed regular source paths from `9e5c0e4f`, plus
untracked regular source paths, sorted/deduplicated; SHA256 over each path, NUL,
and binary file SHA256. Canonical includes CMakeLists.txt, GeneralsMD/Code,
tests/original and cmake; inclusive also includes tests/toolchain.
Canonical651 `b74aeb56f781fc11e3f7f2ddf9afc58d4335dce283f9262e5c3bdc737c0447b5`;
inclusive658 `fe029a0a5b927324faed9edd18a14260f26691dab4fc69f0834faa3f574d2b7f`.
374 tests configured. Focused17 includes all13 script_camera families, both
logical_world camera families and native_client camera-lifetime/view-ownership.
Related184 and full374 are distinct gates; full374 remains unrun.

## Retained development / environment runs

- Pose-only all3 focused11 PASS: GCC15998/5.35s, GCC sanitizer19262/25.14s,
  Clang sanitizer19408/14.52s, all terminal0. Earlier source, not final acceptance.
- GCC4596 actual-extent/pose focused12 PASS8.55s, terminal0 before C3 addition.
- GCC sanitizer20017 failed compiling: assembler temporarily unavailable.
- Clang initial spatial run exited127: LLVM22 library unavailable during host
  update. Installed Clang/llvm-libs now23.1.1; compiler later verified executable.
- Both `*-spatial-environment-retry-build.log` runs failed because old configured
  SDL library `.0.4.16` disappeared. No test execution or source defect claim.
- Regenerated with `cmake --fresh`, rendererOFF, Debug, sanitizer settings intact,
  and clean-rebuilt all original targets. GCC21966 and Clang61314 stopped at
  fixture linking after C3 was added while their old graph was executing: that
  graph lacked NativeCameraShaker.cpp. New source graph includes it. Retain these
  incomplete clean rebuilds, not a full-build PASS or assertion failure.
- GCC sanitizer92380 stopped on the same pre-provider graph link failure.
- Regenerated provider graph: GCC33981 full build/focused14 PASS9.54s terminal0,
  before final amortized capacity/push/reverse-iteration refinement. Clang44407
  is the corresponding pre-final-owner iteration; poll exact handle.
- Clang44407 completed full build/focused14 PASS24.84s terminal0 before final
  owner refinement.
- Final owner/source iteration: GCC79967 full build/focused14 PASS9.54s terminal0;
  GCC sanitizer60081 and Clang sanitizer55202 also terminal0 full build/focused14
  PASS. Clang focused24.82s; GCC sanitizer focused44.17s. No live jobs.

All regenerated toolchain metadata identifies GCC16.2.1 and Clang23.1.1. No
mixed older-compiler build is claimed as final acceptance. Final source runs
remain iteration evidence until concrete View cohort adoption/broader checks.

Log prefixes `/tmp/n2-camera-cohort-<configuration>-{pose,spatial,
spatial-environment-retry,host-refresh,provider}-{configure,build,focused}.log`.
All live CTest processes retain unsuppressed ASan/UBSan/LSan and existing family
timeouts. No coverage waiver or timeout increase. CMake/compiler changes touch
only generated local build state, never upstream source.

## Concrete frame development and corrected source oracle

- GCC69549: compile failure from a helper colliding with the original global
  normalizeAngle; corrected local helper naming.
- GCC3194: generated model fixture compile failure (three missing abstract
  methods and fixed enum assignment); GCC85204: link failure for explicit-pool
  out-of-line destructor. Corrected fixture contracts, not production behavior.
- GCC34831: two selected tests ran; camera passed and camera-frame failed a
  missing-bone test oracle that assumed normal eye Z could not equal 300.
  Corrected to compare complete normal pose; no relaxation of bone selection.
- GCC65601: focused15 PASS12.69s after surrounding frame/lifecycle batch review.
- Final17 GCC84908, GCC sanitizer48227 and Clang sanitizer13878 each failed the
  same old spatial-negative oracle (16/17 passed; 16.97/74.02/41.91s). Original
  Matrix3D::Look_At explicitly supports coincident eye/target; finite ground300
  was incorrectly classified as malformed. Replaced that negative input with
  NaN ground and added positive source-fallback and coplanar-ray tests. Original
  Find_*_At_Z divides by zero for coplanar rays; return the ray's plane-origin
  point as a defined original bug repair, retaining noncoplanar parallel rejection.
- Corrected focused17: GCC89009 PASS16.90s, GCC sanitizer56756 PASS70.77s and
  Clang sanitizer24249 PASS41.74s, all terminal0.

All failure/retry pairs above remain recorded. No timeout changes, sanitizer
suppression or omitted failing family. Source is frozen for full configured
builds and related184 checks. Log prefixes `/tmp/n2-camera-cohort-<configuration>
-frame-{final,source}-{build,focused}.log` and `-frozen-{build,related}.log`.

## Remaining acceptance limits

Actual native UI/Display/root factory adoption and client frame execution remain
pending, as do physical rendering and production CPU model metadata providers.
Paused-frame tests require the source Mouse/Audio parents used by setGamePaused;
airborne tests require parsed AIUpdate/Locomotor parents. Do not fake those flags
or infer branch acceptance from generated ground actors. N2/slice03 remain open.
The initialized parsed configuration precedes the frame fixture's partition grid;
repeated roots verify View count, original model pool used units and whole memory
manager retirement. ModuleFactory owns ModuleData before returning it, while
ModuleInfo borrows it; fixture literal conversions are staged before admission.
Production LTA literal callers share that registry ownership, so a later string
conversion failure does not create an unowned raw ModuleData candidate.

## Frozen validation

| Configuration | Frozen build / related job | Focused17 | Related184 |
| --- | --- | --- | --- |
| GCC16.2.1 | 48765 terminal0; build PASS | PASS16.90s | PASS180.17s |
| Clang23.1.1 ASan/UBSan/LSan | 28898 terminal0; build PASS | PASS41.74s | PASS478.14s |
| GCC16.2.1 ASan/UBSan/LSan | 36703 terminal0; build PASS | PASS70.77s | PASS773.60s |

Sanitizers remain unsuppressed; all family timeouts and failure/retry coverage
are unchanged. Full374 is not run or implied by this camera checkpoint.
