# M22 plan 01 slice 08T0R1: exact declared sampled-mip range

Status: complete; accepted in this slice commit after exact final-source gates.
Plan transaction parent: `9eefa8ec75a8c810717fd983377f188d2fc01931`.
Plan provenance: exact commit `delivery: M22 plan declared sampled-mip range`.
Implementation provenance: this slice commit, exact subject
`delivery: M22 08T0R1 bound declared sampled mip range`.
Acceptance: [08T0R1 evidence](../../evidence/cnc-generals-zero-hour/milestone_22_slice_08t0r1.md).

## Outcome, authority and dependency order

Every ordinary or deferred bgfx draw samples only the original texture's declared
mip range, including partial chains. Preserve authored mip bytes and source
camera/filter/sampler choices. This is exactly one approved backend corrective
leaf before [08T0A](milestone_22_plan_01_slice_08t0a.md), then evidence-only
[08T0](milestone_22_plan_01_slice_08t0.md), then the unchanged active08 trial.
No further split or owner admission without an architecture checkpoint.

Requires accepted05B2B1 source mip upload/lifetime, M30 public bgfx, and accepted
08P0C2D0B1 candidate/COW resource ownership, 08P0C2D0B2A bounded native admission
and 08P0C2D0B2B deferred journal/leases. These providers remain accepted, not
reopened. In-progress T0A and every active08/renderer source/test/ledger hunk stay
unstaged and preserved; no camera fixture or startup semantics change belongs
to this leaf. Authorization is unchanged local generated validation only; no
new external access, package, credential or service.

## Read-only facts and finite owner inventory

FACT: `TextureDesc::mip_levels` is the exact logical count. Recording retains
that descriptor/count; SDL uses `num_levels=desc.mip_levels`. Original
`TerrainTextureClass(height)` declares A1R5G5B5/MIP_LEVELS_3. The backend ordinary
and COW `createTexture2D(..., desc.mip_levels>1, ...)` accept only a boolean and
allocate the complete native chain, not the supplied partial count.

FACT: generated-only native-camera diagnosis observes14 indexed terrain draws
and one present. All14 bind the same2048×128 BGR5A1 atlas with three declared and
uploaded mips, nearest mip selection and maximumLOD1000. Read-only native
Vulkan metadata shows12 allocated mips. Authored UV source texels and all three
uploaded levels are opaque/nonblack; readback contains black geometry writes
with zero alpha, distinct from the opaque clear background. Camera formula and
uploaded view/projection agree. INFERENCE: access to unspecified lower native
mips explains the black minified terrain; independently, exposing12 instead
of3 is a demonstrated contract defect. Acceptance must prove the correction
with controlled mip colors, not rely on that inference or change the camera.

| Owner | Required correction / preserved boundary |
|---|---|
| Public texture descriptor and source upload | Keep exact declared count, pitches, formats and authored bytes; no generated tail or CPU resampling. Validate range against dimensions and existing limits. |
| Bgfx ordinary draw | Bind the existing public subresource overload with firstMip0 and count equal to the logical descriptor. The native allocation may retain its full chain, but no undeclared level is visible through the sampled view. |
| Bgfx capture_draw | Deep-copy the exact range with the immutable texture/sampler binding and acquire the existing B1 native ownership lease. Later handle removal/reuse or COW cannot retarget an accepted command. |
| Pinned bounded manifest | Add only the bounded mip-range representation needed for replay; validate known/live texture, nonzero count and overflow-safe first/count limits before reservation or target touch. Keep explicit legacy full-range defaults source-compatible where retained. |
| Native replay/cache | Replay through the existing public subresource binding. Binding already stores/hash-compares full mip-range state; preserve exact full equality/collision behavior and zero API-thread allocation after admission. No private-header engine consumer. |
| COW/checkpoint | Descriptor and per-level known backing restore together; copy only known source mips. Failed create/upload/publication/commit preserves accepted native ownership/pixels and leaves no live candidate. Accepted monotonic tombstones/counters remain diagnostic, not reset. |
| Recreation/error/teardown | Existing source invalidation and fresh Edge generation replay the same declared range/bytes. Preserve B1 terminal device-error behavior and reverse once-only retirement; no recovery or device-loss redesign. |

Public bgfx subresource binding and Vulkan image views already support exact
first/count; ordinary four-argument binding exposes the full chain. The deferred
manifest currently lacks a range and uses that ordinary binding at replay.
Thus both routes need one coherent correction. Native binding/reservation arrays
already contain range fields: adding manifest fields must update checked slab
byte accounting/identity, not invent an unreserved replay allocation.
Vulkan image-view/cache creation is render-worker/backend work under the already
accepted boundary, never falsely claimed API-thread reversible allocation.

## Scope, state, errors and recovery

Scope: exact sampled subresource range for all existing supported2D sampled
formats/routes, ordinary binding plus journal capture/preflight/replay and its
pinned patch/bootstrap identity. Reached upload formats RGBA8/BGRA8/BGR5A1 keep
their existing conversion/pitch behavior. Compressed/depth uploads remain
fail-closed; this leaf does not admit a new format or resource kind.
Render-target/single-mip/full-chain behavior and presentation/readback stay
source-equivalent. No camera, filter, sampler-policy, source mip rounding,
generic texture loader, allocator, engine ABI/layout or Windows change.

Preserve B1 unknown-level semantics: missing/uninitialized mip backing remains
unknown, never fabricated accepted data. Validate the sampled range and its
readiness before draw/manifest mutation; no exposed unknown non-target level can
be labeled ready. Do not initialize unknown levels merely to produce a pixel.
Existing render targets obtain readiness from completed writes, not CPU backing.
Do not broaden the accepted sampler maximumLOD policy: only enforce the declared
view limit while preserving the existing supported sampler decisions.

All fallible candidate/slab/copy/native preparation completes before accepted
publication or replay. Nested/foreign/stale range or handle, count0/bound+1,
overflow and late invalid command fail before view/touch/submit/present/retire,
preserving prior pixels, owner identity, receipt and ordinary command prefix.
Abort keeps original COW descriptor/known bytes/native handle, candidate leases
retire once at the established boundary, and a clean retry emits once. Range
capture is immutable and exact for aliases as well as distinct stage bindings.
No live source registry query or fallible API-thread growth at commit.

## Implementation surfaces and generated controls

Expected owned surfaces: `src/renderer/bgfx_device.cpp`, narrowly needed captured
alias/helper contracts, `third_party/bgfx_bounded_submission.patch` and exact
bootstrap/CMake/contract identity checks; existing isolated mip, reservation,
resource and journal tests plus a CPU-only range contract witness if required.
Use the existing public shaders/fixtures; generated shader assets remain outside
shipping/install closure. Do not edit/stage `tests/renderer/test_bgfx_device.cpp`.
No pinned checkout edit without the exact corresponding durable reviewed patch.

- Partial chains with distinct base/last-declared colors: minification beyond
  the source count clamps to the last declared mip; nearest and linear mip
  filtering, square/rectangular/singleton axes, RGBA8/BGRA8/BGR5A1, single/full
  chains and render-target controls. Source bytes and source policy unchanged.
- Ordinary versus deferred identical pixels/commands/range; two stage ranges
  remain distinct under full equality, with duplicate/equal/conflicting alias
  controls and deterministic cache-collision/equality proof. No hash-only alias.
- Native manifest count0, first/count exact maxima, representable bound+1,
  overflow, stale/foreign texture and mixed early-valid/late-invalid ranges;
  unchanged receipt/pixels/ordinary prefix and zero replay allocation on rejection.
- Immutable manifest/captured range and exact B1 lease survive original input
  mutation, logical handle removal/reuse and candidate replacement. Unknown
  exposed levels reject without fabricated backing or partial accepted draw.
- Every checkpoint/create/upload/copy/publication/submission fault on partial
  chains: accepted identity/known mip bytes/live resources/pixels unchanged,
  bounded candidate tombstones separately, deterministic same-owner retry.
- Repeated abort/commit/drain, sibling isolation, fresh equivalent generation
  replay, stale handle/provider removal and total device/Edge/allocator teardown.
- Exact applied-native patch bytes equal the reviewed artifact, bx/bimg pins
  clean; Debug and Release prove zero API-thread replay allocations with range
  bindings. No normalization of upstream context or unreviewed checkout change.

## Exact commands, freeze and acceptance

Existing narrow build targets:
`cmake --build build/<preset> --target renderer_bgfx_mip_texture_tests renderer_bgfx_submission_reservation_tests renderer_bgfx_transaction_resource_tests renderer_bgfx_transaction_tests original_w3d_texture_decision_tests -j4`.
Add/register `renderer_bgfx_mip_range` CPU witness as this leaf's output before
its final focus, built by `renderer_bgfx_mip_range_tests`; no new physical fixture
owner is required. Exact8 CPU focus:

```text
^(renderer_bgfx_mip_range|renderer_bgfx_submission_reservation|renderer_bgfx_transaction_resource|renderer_bgfx_transaction|renderer_bgfx_transaction_shader_scope|original_w3d_texture_decisions|original_w3d_texture_identity|original_w3d_texture_provider_removal)$
```

Verify all8 registrations in six CTest manifests, then run GCC/Clang debug and
both sanitizers with `ASAN_OPTIONS=detect_leaks=0 ctest --test-dir build/<preset>
-R '<exact8>' --output-on-failure -j1 -V`. Strict host same8 on both sanitizer
builds uses exactly `ASAN_OPTIONS=detect_leaks=1`, no strict UBSan override.
Complete category audits; do not infer clean output from exit status alone.

Physical exact4 on GCC/Clang debug and both sanitizer configurations:
Enable the existing GPU registrations in each build cache with
`cmake -S . -B build/<preset> -DZH_ENABLE_GPU_TESTS=ON` (the ordinary preset default
is OFF); verify all four IDs before selecting them. This is registration only,
not a workload or runtime-policy change. A no-tests-found invocation is not evidence.
`ASAN_OPTIONS=detect_leaks=0 VK_INSTANCE_LAYERS=VK_LAYER_KHRONOS_validation python3 tools/run_validation_clean.py ctest --test-dir build/<preset> -R '^(renderer_bgfx_mip_texture_gpu|renderer_bgfx_submission_reservation_gpu|renderer_bgfx_transaction_resource_gpu|renderer_bgfx_transaction_gpu)$' --output-on-failure -j1 -V`.
Use existing authorized graphical host escalation; no alternate driver/gate.
Run the same native reservation/mip proof against the pinned Debug and Release
runtime builds, retaining assertions and measured zero replay allocations.
Debug native command:
`make -C build/bgfx-toolchain/source/bgfx/.build/projects/gmake-linux-gcc bgfx-shared-lib config=debug64 -j4`;
then rebuild the existing independent
`renderer_bgfx_submission_reservation_native_debug_tests` fixture from the same
source with BX_CONFIG_DEBUG=1 and the exact Debug library. Derive compiler/link
arguments from `ninja -C build/linux-gcc-debug -t commands renderer_bgfx_submission_reservation_tests`,
changing only debug definition/output paths/native library for this generated
proof; record the exact resulting commands. Run
`VK_INSTANCE_LAYERS=VK_LAYER_KHRONOS_validation python3 tools/run_validation_clean.py build/linux-gcc-debug/renderer_bgfx_submission_reservation_native_debug_tests --gpu`.
Verify `readelf -d` loaded-library identity: the normal executable has an
absolute Release DT_NEEDED, so LD_LIBRARY_PATH is not a Debug substitution.
Preserve ordinary runtime selection; do not count Release as Debug evidence.

Reviewed patch/bootstrap: `tools/renderer/bootstrap_bgfx_runtime.sh` verifies
exact pin/license/applied shaderc+submission diffs and rebuilds Release offline;
`tools/renderer/bootstrap_bgfx_shaderc.sh --offline-sources <existing-pinned-parent>`
must accept exact reviewed patches with no fetch. The existing pinned native
gmake runtime target supplies the separate Debug proof. Inspect exact source
diffs/header selection and licenses; no acquisition/license change is authorized.
Require exact applied-native/non-patch whitespace checks and exact patch-byte
identity; reviewed artifact context stays verbatim, not normalized to silence
pre-existing upstream context whitespace. All consumers/header/native binaries
must use the same amended public range representation before acceptance.

Six configurations are GCC/Clang debug/release/sanitized. Refresh overlays with
`cmake -S . -B build/<preset>`, complete `cmake --build build/<preset> -j4`,
freeze owned source/test and selected binary hashes, then six fresh serial
`ASAN_OPTIONS=detect_leaks=0 ctest --test-dir build/<preset> -LE 'gpu|lan|retail' --output-on-failure -j1 -V`.
Run accepted S0 common-contract minimal8 both native toolchains, established
validation-enabled Vulkan GCC3/Clang2, LAN4 all six and ledger checks unchanged.
Document exact frozen composition when in-progress T0A bytes remain present;
these gates do not accept T0A or absorb its hunks into R1. The unchanged generated
T0A physical command may diagnose closure of the original sampling symptom,
but camera acceptance remains T0A's independent required matrix.

## Preconditions, readiness and commit boundary

M22-T0R1-01 accepted providers are verified in history; -02 range correction is
this leaf's output; -03 existing pins/tools/test owners are entry prerequisites,
new CPU registration an output; -04 authorized host/validation facilities are
fresh acceptance gates. No private corpus or warm cache is required for R1.
Readiness: read-only ancestry/index/source contract audit, tool/preset resolution,
existing CTest selection and exact applied-native patch identity. No unresolved
architecture owner, new M0 or future provider at implementation entry.

Accept only exact declared range/pixels, all positive/negative/fault/retry/recreate
controls, clean final category/patch/ledger/privacy/staged audits and total teardown.
Stage only R1-owned production/test/patch/contract/plan/evidence/index paths or
isolated hunks. Preserve every active08/T0A/renderer hunk. Implementation commit:
`delivery: M22 08T0R1 bound declared sampled mip range`.
Then resume the unchanged T0A implementation and its own acceptance, followed
by existing evidence-only T0; do not retail-probe before both are accepted.
