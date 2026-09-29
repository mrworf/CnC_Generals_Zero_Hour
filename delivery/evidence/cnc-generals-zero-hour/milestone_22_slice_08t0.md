# M22 slice 08T0: bounded camera-startup aggregate

Status: accepted in this separate evidence-only aggregate refresh.
Transaction parent: `24d61a509977f36342317e646abe425d30d875d9`.
Implementation subject: `delivery: M22 08T0 close bounded camera startup`.
Historical aggregate: `8e13acdc410ef0cf6b4bd5ab3c0fb9758739c698`.

## Exact accepted composition

| Provider/leaf | Exact commit | Evidence |
|---|---|---|
| 08T0R1 declared sampled-mip range | cbb65e0924ba3c1b6660e6c3193717e9e9a4a123 | [R1](milestone_22_slice_08t0r1.md) |
| 08T0A bounded native camera startup | c62160a8964291a2e218b915bbc7ccf7cb0af0e6 | [A](milestone_22_slice_08t0a.md) |
| 08T0R2 deferred exact logical-terrain publication | 24d61a509977f36342317e646abe425d30d875d9 | [R2](milestone_22_slice_08t0r2.md) |

Ancestry and accepted plans/evidence are verified. The source camera now uses
ordinary native angle/pitch/zoom, constrained stationary transform and ground/
elevated heightfield lookAt without fixture replacement. Typed camera/cache/
terrain idle rollback preserves exact accepted identity/resources and retry;
provider removal, excluded modes, reset and two-generation teardown remain
fail-closed. The backend binds only declared sampled mips, preserving authored
bytes and ordinary/deferred/COW/recreation ownership. No mip synthesis, filter
policy, general picking, animated camera, retail admission or later owner opens.

GameLogic now explicitly publishes only its initialized TerrainLogic owner/
pointer/token; W3DView admits that publication and binds only after successful
camera transaction commit. Genuine native early-view/later-logic startup,
failed-first binding, exact retry, reset token, removal and address reuse are
accepted. There is no arbitrary-global late binding or core WW3D dependency.

## Historical A/R1 gate provenance

A fresh exact12 passes GCC/Clang debug and both sanitizers, category-clean:
368.81/371.68/1535.60/1269.44 seconds. Its fresh strict12 passes both under host
escalation with exactly `ASAN_OPTIONS=detect_leaks=1`, no strict UBSan override:
1387.21/1205.14 seconds. Generated source-camera physical proof passes all four
focused configurations under `VK_LAYER_KHRONOS_validation`, with exact pixels/
rollback/retry and total teardown assertions. R1's own exact8/strict8/physical4
and independent native Debug/Release proofs are accepted separately.

The final A audit changed no executable/test bytes. Approved proportional common
reuse is exact: six complete builds, minimal8 both, established Vulkan GCC3/Clang2,
LAN4 all six and six canonical297/297 with clean category audits. Five preceding
clean canonicals survive only the verified181-archive/36-R1-binary/eight-R1-source
identity proof plus all-six corrected camera controls; Clang sanitizer is the
fresh corrected complete canonical, not the superseded296/297 diagnostic. A
evidence retains this qualification and source/selected-executable fingerprints.
No gate is inferred from an exit code, earlier failure, or aggregate status.

Those are historical A/R1 acceptance records, not substitutes for R2's fresh
final-source gates. The legitimate R2 view/core correction supersedes the changed
source fingerprints while preserving accepted success semantics.

## Current R2 composition and unchanged aggregate reuse

All six complete builds pass. Exact12 passes GCC/Clang native and both sanitizers
in316.68/320.60/1359.97/1182.03s with complete clean category audits. Strict12
passes both sanitizers in1362.56/1183.28s under host escalation and exactly
`ASAN_OPTIONS=detect_leaks=1`, with no strict UBSan override. Fresh minimal8
passes both native toolchains. Generated validation-enabled source-camera
physical proof passes all four focused configurations; established Vulkan passes
GCC3/Clang2 and serial LAN passes4/4 on all six configurations. Six fresh
canonical suites each pass297/297, with clean complete-log category audits:
653.25/642.82/291.23/238.77/2437.89/2036.60s in GCC Debug, Clang Debug,
GCC Release, Clang Release, GCC sanitizer, Clang sanitizer order.

R2 evidence records nine frozen source/test and24 selected-binary hashes,
constant-initialization/minimal linkage/ABI proof, exact six-row ledger ownership,
shared-engine staged-vs-frozen composition and runner-tail classification. All
identities still match after its exact14-path commit. This aggregate changes only
this evidence, its plan and governing index; it reuses those unchanged accepted
gates without executable edit, relink or rerun. The same fourteen active08/renderer
paths remain dirty and unstaged; their current working bytes and shared trial
ledger/engine hunks are untouched. Exact staged/privacy/whitespace checks pass
and the index is clean after commit.

## Dependency-safe continuation

Accepted S0 → R1 → A → R2 → this refreshed T0 aggregate → active08 →09.
Historical accepted owners remain closed. The unchanged bounded redacted
continuation crosses R2 admission and stops at the existing camera typed terrain
checkpoint64MiB capacity predicate before Recording. Read-only public stack
classification, clean teardown and fixed categories are in R2 evidence. No
private names, paths, bytes, hashes, raw logs or images are retained here.

Audit that capacity/ownership boundary read-only before proposing a bound or
allocation change. No further behavior, producer or timeout is admitted by this
aggregate. Any later unchanged scene-once probe retains the120-second bound and
original input/symlink privacy contract. Camera closure does not accept retail
scene/Recorder/partition/UI or a different producer; architecture authority is
required before a behavioral correction.
