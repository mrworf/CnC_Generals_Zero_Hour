# M22 slice 08T0R2: deferred exact terrain-logic publication

Status: accepted in this independently validated implementation commit.
Transaction parent and plan checkpoint: `075c827f4b1f81ad36430b2b2c1a84833a81557d`.
Implementation subject: `delivery: M22 08T0R2 publish deferred camera terrain identity`.

## Finite owner and ABI audit

GameLogic owns one Linux-only source-lifetime sidecar. Its exact owner/provider
and nonzero monotonic token publish only after successful TerrainLogic::init.
Source init rejects duplicate/exhausted ownership before its mutation, rejects a
null created provider before dereference, and init-unwind deletes through the
existing SubsystemInterfaceList owner. Exact withdrawal precedes provider delete;
no callback, ref, allocator, renderer lookup or diagnostic formatting is involved.
The callback-free peek compares raw source/global identities before dereference.
Reset keeps source lifetime/token, whereas retired/reused addresses require a new
token. Publication token is distinct from accepted Edge generation.

W3DView starts unbound, including native early-view initialization. Candidate
binding is admitted only from that explicit source publication; the complete
typed camera/terrain idle transaction must succeed before nonthrowing binding
publication. Failed first admission/query/upload/commit remains unbound. Every
bound entry requires the same owner/provider/token and Edge generation; removal
or replacement cannot silently rebind. T0A camera math and exclusions are unchanged.

The finite executable/test scope is nine paths below. GameLogic gains only nested
types/static methods/friendship, and W3DView only a private static generated-friend
peek. No instance fields, virtual slots, layout/serialization or Windows executable
branch changes occur. Linux class definitions do not depend on a full-client macro.
GCC/Clang object symbols show the publication registry in BSS with no static guard
symbol, proving constant initialization. Headless runtime/identity/provider-removal
controls pass, with no WW3D owner admitted to the core publication spine.
Read-only DWARF inspection compares the current GCC full-probe against the accepted
parent Clang full-probe before its relink: sizeof(GameLogic)=392 and
sizeof(W3DView)=1256 in both. This agrees with the static-only declaration audit.

## Generated witness and classified setup corrections

The unchanged registered camera control now also drives the real existing
original-factory + Recording startup, using the existing camera profile only.
Native InGameUI creates its view before GameLogic initializes logical terrain.
The generated bootstrap rejects a missing map without binding, then accepts the
late initialized publication, first-attempt injected failure/clean retry and
full ordinary startup. Real GameLogic::reset follows source-faithful idle map
cleanup; its publication/binding token must remain exact. Existing independent
view-after-logic ground/elevated/prop and physical controls remain present.

Owner-local registry controls prove fresh/missing/foreign/duplicate/stale-token,
once-only withdrawal, same-address new-token and exhaustion rejection with no
allocator delta. First-binding query/allocation/upload/late-commit faults retain
the exact typed source/device/RNG baseline and zero binding until success.
Accepted publication removal/republication rejects the old bound view; each new
fixture view admits the current source token. Two external process generations
and existing internal generations retain total initialized teardown controls.

Superseded setup: a generated cleanup call used a protected setter from outside
the existing friend. It was moved through that friend without production API
change. Initial bootstrap reached every publication/failure/retry/reset marker,
then no-map continuation rejected a still scene-linked terrain whose backing the
fixture had freed. Public source visual teardown unlinks before free. The fixture
now uses that exact public scene order; production behavior/assertions were not
weakened. Corrected GCC camera control passes1/1 in0.51s. Three headless controls
also pass. These diagnostic results precede the final real-GameLogic-reset focus
and do not substitute for final acceptance.

## Frozen source identity and final acceptance

This frozen composition includes preserved active08 bytes in the shared engine.
Only the R2 extern and conditional bootstrap call are owned there; eventual
payload staging/ledger must use the exact owned candidate blob. Twelve nonshared
preserved active08/renderer files match their prior checkpoint byte-for-byte;
the shared ledger preserves every nonowned row exactly.
No private corpus/symlink input is used by these generated controls.
The exact staged engine candidate SHA256 is
`58c5250eae292b27f6e152fad51827a90cd18b876689e61bed2623fe60681b74`;
its ledger row hashes that payload, while the frozen executable composition below
includes the unchanged preserved active08 hunks. The seven-line R2 payload is
verified against HEAD plus the reviewed owned patch; no trial hunk is staged.

| Frozen source/test | SHA256 |
|---|---|
| GeneralsMD/Code/GameEngine/Include/GameLogic/GameLogic.h | f1733e21b1a9fcab78ef08295fae1920a7a3f71c1d5aabb636dadc629bd78ccc |
| GeneralsMD/Code/GameEngine/Source/GameLogic/System/GameLogic.cpp | 707c8297339583e4c8df501b19915f100edae72b4d6c999909bd6adaeacd973e |
| GeneralsMD/Code/GameEngineDevice/Include/W3DDevice/GameClient/W3DView.h | 13bfeed1244027f8858a85476b071db4c5e4ef700d03eee8d7502c75287b9d77 |
| GeneralsMD/Code/GameEngineDevice/Source/W3DDevice/GameClient/W3DView.cpp | 1c95c42bd7a9c3c7f7632ae1836311652687bd32ce9a2ac9f7aba644f7996f04 |
| src/original_runtime/w3d_camera_startup_cpu.inc | 44d155e251298b4d74e38d1b253faffad5246643a9f652bbb4bf4a6ac0b29a4c |
| src/original_runtime/linux_game_engine.cpp | 70f090fdea2de8591be46ff1b9719bfea9d7cac14322ca381d47eecdca8856ca |
| tests/original_rendering/camera_startup_probe.cpp | 10ba51e4a22d9cbf571d247cc20b7ebc300fbdf66fbdca39af23f268d77fce6a |
| tests/original_rendering/test_w3d_camera_startup.py | 5480ad8ef0d2e9d1be03c6b23598848f6433f769cac8b9f4fa46927cd0542d68 |
| tests/original_lifecycle/test_headless_update.cpp | bf7b51e1f46e42c8eaf3f233c8afeb760a91272af9af03b7d784eb8386939f77 |

Exact12 selection and all commands are in the [plan](../../plans/cnc-generals-zero-hour/milestone_22_plan_01_slice_08t0r2.md).
All required generated/build/common/canonical gates below are complete. Final
source/category/hash/ledger/privacy/index and exact staged audits pass. No full
gate is inferred from prior A/R1 acceptance or process exit alone. The unchanged
bounded redacted continuation is classified separately below; it is guard-closure
discovery, not retail scene acceptance.

Frozen GCC native exact12 completed12/12 in316.68s; Clang completed12/12 in320.60s.
Both complete logs have no sanitizer or validation category. Native focus is
retained only if exact full-probe/producer
binary hashes survive those builds; otherwise the affected native focus reruns.
Reconstruction reversing only the six owned ledger rows matches the entire prior
ledger snapshot hash; nonowned active08 rows are unchanged.
All six complete builds pass. Both native exact full-probe/producer binary hash
pairs remain identical to the native exact12 runs, preserving that final-source
focus provenance without duplicate execution. Fresh minimal8 passes8/8 on each
native toolchain. GCC sanitizer exact12 passes12/12 in1359.97s with no sanitizer
or validation category; its long prop-frame control passes in1117.66s. Clang
sanitizer exact12 passes12/12 in1182.03s without sanitizer/validation categories;
its prop-frame control passes in970.95s. All nine frozen source/test identities
and24 selected binary identities still match.
Strict GCC passes12/12 in1362.56s under exactly `ASAN_OPTIONS=detect_leaks=1`
with no strict UBSan override and no sanitizer/leak/validation category. Strict
Clang passes12/12 in1183.28s under the same strict contract and with no
sanitizer/leak/validation category. Generated validation-enabled camera physical
controls pass all four configurations. Established Vulkan passes GCC3/3 in5.13s
and Clang2/2 in4.17s. Serial LAN passes4/4 on all six configurations
(1.71/1.71/1.71/1.71/1.86/1.78s in prescribed configuration order). Complete
strict/physical/Vulkan/LAN log category audits are clean. Six fresh297-control
canonical suites pass serially with complete clean category audits:

| Configuration | Canonical result | Seconds |
|---|---|---|
| GCC Debug | 297/297 | 653.25 |
| Clang Debug | 297/297 | 642.82 |
| GCC Release | 297/297 | 291.23 |
| Clang Release | 297/297 | 238.77 |
| GCC sanitizer | 297/297 | 2437.89 |
| Clang sanitizer | 297/297 | 2036.60 |

The final Clang sanitizer prop control completes in968.59s; its preceding GCC
counterpart completes in1114.14s. All nine source/test and24 selected binary
hashes match after the complete queue; preserved active08/renderer files and
nonowned ledger rows remain exact. No completed suite was restarted.
The temporary runner's tail exited127 after all six successful build returns/hash
checks because an in-progress count-helper edit shifted its later Bash read offset.
The current runner is syntax-valid and frozen; no child build failed and none was
restarted. This orchestration-only tail result is not runtime acceptance evidence.
Concise AGENTS.md lessons require freezing shell runners during execution and
filtering private-input wrapper observation to public command/guard summaries.
These documentation-only additions change no frozen source/test or binary bytes.

## Bounded redacted continuation and public classification

The existing unchanged120-second scene-once wrapper completes in62.970s. Fixed
categories: status3, reach1, setup1, scene0, init3, logic5, mapINI4, requested
stage1, Recording0, teardown1; sanitizer/origin/frame categories are0. These
counts alone do not identify the failed contract. No private selector/root,
input-derived bytes/hash, raw log or image is retained or committed.

A read-only debugger routed only to that existing continuation subprocess emits
public owner frames and fixed identity booleans; its complete run takes63.692s.
The sole throw is `cameraRequire` from
`W3DView::CameraStartupAttempt::CameraStartupAttempt` at the existing64MiB typed
terrain-tile checkpoint predicate, called by `applyCameraStartup` and
`setAngleAndPitchToDefault` in `GameLogic::startNewGame`. `admit(source)` has
already returned and the preceding structural check passed. Thus R2's explicit
initialized logical-publication guard is crossed. The first view binding remains
empty because preparation failed, exactly as R2 requires. Current logical owner
and provider are present. No signal/debugger/sanitizer category occurs.

The approved classification is a separate active08 camera-capacity boundary, not
an R2 logical-provider defect. R2 closes without changing capacity, allocation,
timeout, camera math or source behavior. After the separate evidence-only T0
refresh, that capacity/ownership question requires read-only audit and explicit
architecture disposition before an edit. This discovery is not retail acceptance.
