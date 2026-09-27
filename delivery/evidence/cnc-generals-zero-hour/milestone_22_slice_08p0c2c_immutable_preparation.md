# M22 slice 08P0C2C: immutable outside-frame tree preparation

Status: accepted; exact implementation commit is identified by the governing
index's unique slice subject and Git history.
Transaction parent and approved plan-only checkpoint:
`fd87f2e45e3dca84d8242a1fa89faa3282f3350e`.
Accepted dependency C2B: `749067b`. No new prerequisite/slice.

## Exact owner and ordering

The shipping Linux full-draw display prepares after accepted shroud/camera
updates and before Begin_Render. Existing C1 builds a complete immutable
candidate before publishing any tree state, geometry, GameClient RNG or FX.
The phase shares exact geometry ownership, independently pins atlas/shroud
source refs, and owns its shaders, pipeline, samplers and both constant buffers.
It does not depend on mutable generic physical state or pending DX8 stages.
Camera constants retain native projection/view algebra, encoded sway slots,
unswayed shroud origin/scale, alpha0x60/GEQUAL, disabled blending, depth-write
LEQUAL, no cull and exact native stage combiners.

Before C1 publication, the exact backend/target opens and aborts a bounded
frame-command transaction:4096 commands/resources, maximum upload bytes and
ordered-view capacity. No live journal spans old-resource retirement.
This proves preparation-time compatibility/admission, not later availability.
Program/resource creation, upload, pinning and validation all precede C1's
no-throw phase publication; then replaced units retire before FX dispatch.
FX may remove/reset terrain. The local shared lifecycle object alone reports
cancellation after dispatch; no registry lookup/owner dereference follows FX.

Accepted retry reuses the same identity/resources/constants without another
C1/provider/RNG/FX entry. Completion/cancellation release exactly once, only
outside a source pass; stale target/edge/owner epoch cannot complete or retry.
Successful source removal, relocation, reset and disabled-tree toggles cancel
the phase. Empty/hidden states are explicit. Last-tree sink deletion performs
the same bounded probe before irreversible registry erasure and returns EMPTY,
not READY with absent phase state. Rejected deletion preserves the accepted
tree/vertex/resources/RNG for retry. No tree triangle draw, decal, factory,
retail or C2D transaction integration is opened here.

## Generated controls and correction provenance

The narrow public-display witness exercises two internal generations in each
of two outer generated runs. It covers create/upload/checkpoint/publication
failures, exact old identity and RNG preservation, same-phase retry, seven
phase-program unit retirement, foreign/double completion, stale frame generation,
pause, hidden/empty, disabled-tree cancellation, removal/reset and epoch rejection.
Pooled uniquely-owned FX nodes prove admitted collision intent survives rejected
preparation, successful commit dispatches once, the first nugget removes/resets
the terrain and the later nugget still runs. Fogged source suppression consumes
without dispatch; a real reveal admits the removal route.

The generated logical partition is larger than its8x8 visual map. A fixture-only
notification adapter records/asserts exact reveal/undo statuses, coordinates and
counts, restores the real display before execution, and never replaces the
production PartitionManager query. Each generation explicitly establishes
visited-then-fogged baseline; never-seen shroud is a distinct native branch.
The normal caller presentation boundary follows each successful display call
using an RGBA8 Recording target, avoiding cumulative ordered-view exhaustion
without widening limits or changing shipping display behavior.

The initial teardown expectation incorrectly included generic terrain programs
that remain Edge-owned. Only fixture scope changed: completion/cancellation prove
exact seven phase units, total resources zero follows Edge teardown. A subsequent
confirmed last-tree omission corrected production probe/EMPTY handling; a full
clear-source topple→DOWN→sink decrement→last-delete fault/retry witness passes.
All earlier preliminary gates are superseded by the frozen identities below.
Expanded Edge controls prove exact native descriptors/constants, pin-address
rejection before dereference, refcount balance on every candidate fault, no-touch
probe abort, and program survival after generic/stage mutation.

## Refrozen acceptance contract

No canonical suite has been reused from the prior slices. Run six complete builds,
the exact eight-control focused union on both native and both sanitizer builds,
strict host LSan eight each with `ASAN_OPTIONS=detect_leaks=1` and no UBSan
override, established physical Vulkan and serial LAN4/4 all six, then six serial
canonical nonretail suites with `-LE 'gpu|lan|retail'` and sanitizer leak detection
disabled. Audit complete logs, not exit codes alone. Final source hashes must
match before exact staging; unrelated renderer diagnostic remains unstaged.
The planned focus is persisted in the C2C plan. Ledger/diff checks pass.

## Frozen executable/test identities

Refrozen six complete builds pass. The exact eight-control focus passes all four
configurations: GCC2.07s, Clang2.20s, GCC sanitizer14.32s, Clang sanitizer9.24s.
Complete verbose output has no runtime/ASan/LSan/VUID/validation category.
Strict host LSan passes8/8 both (15.30s GCC,9.96s Clang), with exactly
ASAN_OPTIONS=detect_leaks=1 and no UBSan override. Serial host LAN passes4/4
all six; its complete logs are clean. Source hashes still match below.

One exploratory established Vulkan bootstrap run during concurrent compilation
failed its ready-to-engine paired count, while display-owner and map controls
passed. Preserve that failed log as superseded, not accepted. A separately built
exact accepted-parent source archive and frozen C2C each pass four isolated
complete bootstrap sequences: original ready6181→engine6189 (+8), residual26
and baseline22; both device-only deltas are +8 with absolute checkpoints varying
between6205→6213 and6208→6216. The original failing device-only sample was
6213→6216 (+3), not a C2C source/residual increase. All three resolved C2C
preparation entry breakpoints have zero hits in the generated no-map route;
map=0, phase/program/probe do not execute. Bootstrap main and process services
have exact accepted-parent byte identity; no class data/vtable change occurs.
Concurrent worker sampling timing is an inference, not a claimed stack-level
root cause. The approved classification requires complete isolated physical
rerun, with no source change, count waiver or weakened assertion, before
canonicals. The isolated complete established rerun passes3/3 GCC and2/2 Clang;
exact tree-program and nonuniform shroud physical proof also pass all four
native/sanitizer configurations. Complete output has no runtime/sanitizer/
VUID/validation/failure category. All six serial277-test canonical suites now
pass; complete per-test output is archived and audited, not inferred from exit0.

## Final accepted matrix

| Configuration | Canonical result | Seconds |
| --- | --- | --- |
| GCC Debug | 277/277 | 283.67 |
| Clang Debug | 277/277 | 270.17 |
| GCC Release | 277/277 | 152.46 |
| Clang Release | 277/277 | 103.89 |
| GCC ASan+UBSan | 277/277 | 897.00 |
| Clang ASan+UBSan | 277/277 | 711.91 |

Six complete builds, exact8-control focus all four, strict host LSan8 each,
serial LAN4 each all six, established physical Vulkan3/3 GCC and2/2 Clang,
and exact program/shroud physical all four pass on these same frozen bytes.
Sanitizer canonical/focus/physical/LAN use ASAN_OPTIONS=detect_leaks=0;
strict LSan uses exactly ASAN_OPTIONS=detect_leaks=1, no UBSan override.
Physical/LAN/strict commands use host escalation; physical uses the existing
graphical environment and Khronos validation, with no suppression.

Complete canonical, focused, strict, LAN and physical logs have no runtime-error,
ASan, LSan, VUID or validation category. The unchanged heavy witness passes
268.91s GCC sanitizer and225.51s Clang sanitizer total; its already-committed
240s per-generated-process contract is unchanged, with no workload/assertion
reduction. Final source hashes, ledger, header/constructor/value-member audit
and diff checks pass. No temporary markers remain. Exact14 owned paths are
the implementation/fixture, ledger and plan/index/evidence payload; the unrelated
two-line renderer diagnostic remains unstaged. No executable rerun is required
for final evidence/status edits. C2D remains the next gated draw owner.

The following ten executable/test hashes are the final accepted identities.

```text
57e435ee96ad654f3b8b6516462b1c3b8f91c408d86b731195d8e0389a7d1b2e  CMakeLists.txt
9dfa0b437b27c480401ab367452bb701183f8521d4d35606e147ff369ad65a8b  GeneralsMD/Code/GameEngineDevice/Include/W3DDevice/GameClient/BaseHeightMap.h
e2d59f5859de98a41a412646bec0f9b807fa077a3443915f3ca19f779016a43d  GeneralsMD/Code/GameEngineDevice/Source/W3DDevice/GameClient/BaseHeightMap.cpp
afa8dc4b22d5f2ba4159f90a7714ce7b99570783bac00fda7e6da63fc9c7a6ef  GeneralsMD/Code/GameEngineDevice/Source/W3DDevice/GameClient/W3DDisplay.cpp
556557dae4e091caa40e38e6e33066bbc4ef47aea1bd849deb00956a5e9d8582  src/original_runtime/linux_game_engine.cpp
ba06b22141ec65cad128398722ea35b2f0423156671f095ba42e0e17c05e8cb1  src/original_runtime/original_gpu_edge.h
99d10e0405eac465947b93e355e610d757aa401394c9d2f39ea7b681768afc21  src/original_runtime/original_gpu_edge.cpp
c920a4eade1370a35206cae2f4a9e64b9f7c0ec46588d477cc94b1f35b8e4e9b  tests/original_rendering/test_tree_program.cpp
afc565440bd30ef4320f2d19af590600d1d654d301e2aa20f2c3cb7779ca1a80  tests/original_rendering/terrain_tree_preparation_probe.cpp
36c8233ed9b85c5e79934ebf69866cbf34a99694f168eb799a5f28d46d64ef3e  tests/original_rendering/test_w3d_tree_preparation.py
```
