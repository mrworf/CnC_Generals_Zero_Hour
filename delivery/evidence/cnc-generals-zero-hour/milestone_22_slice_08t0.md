# M22 slice 08T0: bounded camera-startup aggregate

Status: accepted in this evidence-only aggregate commit.
Transaction parent: `c62160a8964291a2e218b915bbc7ccf7cb0af0e6`.
Implementation subject: `delivery: M22 08T0 close bounded camera startup`.

## Exact accepted composition

| Provider/leaf | Exact commit | Evidence |
|---|---|---|
| 08T0R1 declared sampled-mip range | cbb65e0924ba3c1b6660e6c3193717e9e9a4a123 | [R1](milestone_22_slice_08t0r1.md) |
| 08T0A bounded native camera startup | c62160a8964291a2e218b915bbc7ccf7cb0af0e6 | [A](milestone_22_slice_08t0a.md) |

Ancestry and accepted plans/evidence are verified. The source camera now uses
ordinary native angle/pitch/zoom, constrained stationary transform and ground/
elevated heightfield lookAt without fixture replacement. Typed camera/cache/
terrain idle rollback preserves exact accepted identity/resources and retry;
provider removal, excluded modes, reset and two-generation teardown remain
fail-closed. The backend binds only declared sampled mips, preserving authored
bytes and ordinary/deferred/COW/recreation ownership. No mip synthesis, filter
policy, general picking, animated camera, retail admission or later owner opens.

## Gate composition and unchanged provenance

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

All A frozen18 source/test and8 selected binaries still match after its commit.
This aggregate changes only this evidence, its plan and governing index; it
reuses unchanged accepted gates with no executable edit, relink or test rerun.
The same fourteen active08/renderer paths remain dirty, unstaged and byte-identical,
including shared engine/ledger trial hunks and the unrelated renderer diagnostic.
Exact staged/privacy/whitespace checks pass and the index is clean after commit.

## Dependency-safe continuation

Accepted S0 → R1 → A → this T0 aggregate → active08 →09. Historical accepted
owners remain closed. Resume only the existing read-only categorized scene-once
wrapper with its unchanged120-second bound and original input/symlink privacy
contract. No private names, paths, bytes, hashes, raw logs or images were read or
retained for this aggregate. Camera closure does not accept scene/Recorder/
partition/UI or a different producer; any newly reached failure is classified
read-only at its public owner boundary before an architecture decision.
