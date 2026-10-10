# Milestone prerequisite manifest

Product: `cnc-generals-zero-hour`. Packet:
`delivery/milestones/cnc-generals-zero-hour/linux-upstream-v2/status.yaml`.
Audited packet revision: recorded in `workflow/delivery-planning/handoff.yaml`.
Source transaction: `e0a32f1c92b113733bbe588cfc00aeb7b019e4d7`.
Planning transaction: finalized in `workflow/delivery-planning/handoff.yaml::planning.repository_commit`.
Audit: 2026-10-10. Intended order: N0→N1→N2→N3→N4→N5→N6→N7,
from `docs/zero-hour-linux-port-plan.md::Delivery order`. N0/N1 completed; next N2.

| ID | Description | Consumers | Classification | Current status | Proposed provider | Class | Verification | Commands executed | Files changed | Blocking | Evidence |
| --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- |
| PRE-01 | Approved baseline and build tools | N0,N1,N2 | Already satisfied | N0 preserved; current tools verified | Repository/host | AUTO | Status evidence; compiler/CMake/Ninja/Python version queries | Version queries passed; no archive access | None | Before build; satisfied | FACT: AGENTS.md; status.yaml; environment checks below |
| PRE-02 | Pristine upstream sources/acquisition | N1,N3,N4,N7 | Owned by the consuming milestone | N1 historical pass; future acquisition/verification pending | N1 already delivered; N3/N4 verify; N7 upgrade | AUTO | Locked sources match third_party/bgfx.lock.json; documented verify/acquire/build | Not run (no renderer change) | None | Before renderer/upgrade validation | FACT: tools/upstream_bgfx.py and docs/build.md; no current network claim |
| PRE-03 | Physical Vulkan/session/validation facility | N1,N3,N4 | Requires user action | Historical host success; current GPU/session unverified | Local user/host | USER_ACTION | Normal-host vulkaninfo --summary; live supported session; public capability result | CLI presence only; device test not run | N3/N4 readiness sections | At N3/N4 physical validation; not N2 | FACT: N1 evidence is historical; INFERENCE: facility can be reprovided; verify at boundary |
| PRE-04 | Locally supplied original data and representative scenarios | N2,N3,N4,N5,N6,N7 | Requires user action | Unverified for current native startup/scenarios; earlier index/integrity only | Asset owner supplies read-only roots; consumer validates | USER_ACTION | Actual native post-init/teardown and redacted integrity for N2; scenario/install checks at later consumers | Not run; no private inputs inspected | N2/N6/N7 readiness sections | N2 retail probe and acceptance, later retail validation; not generated development | FACT: HQ-06; evidence/qa/N2-original-data.md is not native startup |
| PRE-05 | Declared development headers/libraries | N1,N2,N3,N4 | Already satisfied | Current discovery/configure passed; later integration may require own declared additions | Host; docs/build.md | AUTO | pkg-config plus fresh renderer-disabled CMake configuration in three variants | All queries and three configures passed | docs/build.md | Before relevant configure; satisfied for N2 | FACT: cmake/OriginalRuntime.cmake requires zlib/FreeType/Fontconfig/OpenSSL3/SDL3 |
| PRE-06 | Accepted renderer and original process | N3,N4 | Provided by an earlier milestone | N1 completed; N2 in progress | N1,N2 | AUTO | Inspect status plus exact accepted source/configuration evidence | Manifest inspection; acceptance not rerun | N3/N4 boundary and readiness sections | Before N3/N4 implementation consumes providers | FACT: approved graph and status.yaml |
| PRE-07 | Integrated original world, UI/media | N5 | Provided by an earlier milestone | Pending | N3,N4 | AUTO | Source-runtime scene/UI/media acceptance | Not run | N5 readiness section | Before N5 consumes providers | FACT: approved graph; HQ does not replace later acceptance |
| PRE-08 | Playable deterministic original process | N6 | Provided by an earlier milestone | Pending | N5 | AUTO | Full-match and persistence/determinism acceptance | Not run | N6 readiness section | Before N6 implementation consumes provider | FACT: N5 contract and graph; local UDP split into PRE-13 |
| PRE-09 | Clean Arch/Ubuntu/Fedora environments | N7 | Requires user action | Docker CLI present; daemon/images/clean hosts unverified | User/maintainer environment owner | USER_ACTION | Clean environment with declared tools/development packages; successful clean configure/build/install matrix | CLI presence only; no daemon/service/image operation | N7 readiness section | At N7 portability validation | FACT: CLI exists; INFERENCE: environments obtainable; verify before validation |
| PRE-10 | Current bounded definition and packet | N2,N3,N4,N5,N6,N7 | Already satisfied | Exact source hashes, recorded passes and producer ancestry verified | Committed definition package; this planning transaction | AUTO | Hash/ancestry/graph/coverage/contract validation | Passed; provenance finalized before handoff | Packet/state/handoff | Before execution; no unresolved authority | FACT: definition package; compiler-result.yaml |
| PRE-11 | Remaining N2 plan, native root/H1/evidence harness | N2 | Owned by the consuming milestone | Pending implementation; old slice03 cannot govern new work unreviewed | N2 delivery planner and implementation | AUTO | Fresh governing plan; accepted-work inventory; HQ-01–07 mapped finite outcomes; actual native traces | Contract/source-target inspection only; no acceptance | N2 contract and packet index | Plan before code; runtime/harness before acceptance | FACT: EXCLUDE_FROM_ALL link probe is not runtime; HQ-01–07 |
| PRE-12 | Original interaction/audio/media validation facility | N4 | Requires user action | Package discovery verified; actual session/devices unverified | Local user/host | USER_ACTION | Supported display/input and audible output for playback evidence; exercise no-device separately | Not run | N4 readiness section | At N4 physical interaction/media validation | FACT: N4 functional/acceptance contract; INFERENCE: host facility available, verify then |
| PRE-13 | Nonprivileged local UDP harness | N6 | Owned by the consuming milestone | Pending probe/harness | N6 | AUTO | Two local processes bind/exchange and complete required deterministic session/failure checks | Not run | N6 readiness section | Before LAN validation; host restrictions cause explicit blocker | FACT: N6 owns local transport adaptation; no external service required |
| PRE-14 | Synchronized match outcome | N7 | Provided by an earlier milestone | Pending | N6 | AUTO | N6 accepted two-process match evidence | Not run | N7 readiness section | Before N7 consumes provider | FACT: approved N6→N7 edge |

## Verification boundaries and owner instructions

PRE-04: generated H1 work is available without retail inputs. Before the N2 supplied-data
probe, the asset owner makes existing lawful Generals/Zero Hour inputs readable using local
explicit roots and separate user storage. No upload, copy into Git or retail modification.
The N2 implementer supplies the actual native probe and bounded audit wrapper; exact CLI is
that deliverable, not an invented command here. Completion is redacted actual post-init,
clean teardown and unchanged integrity. Missing data then blocks acceptance. Existing
stage4/mask63 evidence cannot clear this gate. Later consumers validate their own scenes.

PRE-03/PRE-12: before physical validation, use the normal host execution context and
supported display/input/GPU/audio facilities. If checks fail, the user/host owner restores
the required facility; do not install OS packages, change drivers or privileges here.
Record capability and bounded diagnostic results without private device/host identifiers.
N2 requires logical/runtime libraries, but no physical graphics/audio session.

PRE-09: before N7, the maintainer provides clean Arch/Ubuntu/Fedora hosts or containers
with declared build dependencies and lawful local data for installed smoke tests. Verify
usable environments and the clean build/install matrix, not only a container CLI. Privileged
OS setup or unavailable facilities become user-action blockers at that boundary.

PRE-11: milestone-owned plan and product code are deliverables, not missing external
prerequisites. A fresh remaining-work plan must adopt HQ-01–07, preserve accepted code/tests
and dirty work, replace stale delivery pointers coherently and choose finite behavior exits.
No feature implementation or slice creation is performed by readiness.

## Current observed environment

- `gcc`: g++ (GCC) 16.2.1 20260810 (exit 0).
- `clang`: clang version 23.1.1 (exit 0).
- `cmake`: cmake version 4.4.4 (exit 0).
- `ninja`: 1.13.2 (exit 0).
- `python`: Python 3.14.7 (exit 0).
- `pkg:sdl3`: 3.4.18 (exit 0).
- `pkg:freetype2`: 26.6.20 (exit 0).
- `pkg:fontconfig`: 2.18.3 (exit 0).
- `pkg:zlib`: 1.3.2 (exit 0).
- `pkg:libavcodec`: 63.1.102 (exit 0).
- `pkg:libavformat`: 63.1.102 (exit 0).
- `pkg:libavutil`: 61.1.102 (exit 0).
- `pkg:libswresample`: 7.1.102 (exit 0).
- `pkg:libswscale`: 10.1.102 (exit 0).
- `pkg:vulkan`: 1.4.363 (exit 0).
- `pkg:x11`: 1.8.13 (exit 0).
- `pkg:gl`: 1.2 (exit 0).
- `pkg:wayland-client`: 1.26.0 (exit 0).
- `pkg:libcrypto`: 3.6.5 (exit 0).

Three fresh temporary CMake configurations passed: normal GCC, GCC with ZH_SANITIZE,
Clang with ZH_SANITIZE; all Debug/Ninja, ZH_BUILD_RENDERER=OFF. No compile/runtime/sanitizer
or H1 acceptance executed. Installed versions differ from historical evidence; historical
results are not transferred to these configurations. Configure observed current worktree,
including preserved unaccepted edits; prerequisite discovery was also inspected in committed
CMake sources. No dependence on ignored build outputs was needed for these configurations.
