# Milestone readiness report

## 1. Overall readiness status

**READY_WITH_EXTERNAL_DEPENDENCIES** — revised M20 can begin after committed packet adoption; every required offline runtime provider belongs to M20, and later external validation gates have owners and completion evidence. Readiness does not assert that the original engine already builds or runs.

## 2. Scope and evidence basis

Product `cnc-generals-zero-hour`; all 26 contracts, index, traceability and `delivery/milestones/cnc-generals-zero-hour/status.yaml` reviewed. Authority is `docs/zero-hour-runtime-closure-reconciliation.md` RC-001–RC-005, source/transaction-start commit `40a1c383e04c931d938bdc5a0fe6671e0a4af249`, plus the unchanged migration supplement and base plan. Parent records the final packet fingerprint and planning payload SHA in `workflow/delivery-planning/state.yaml` (`transaction.repository_commit`).

FACT: starting worktree contained parent/compiler-owned M20–M25, index/status/traceability and planning workflow edits; no unrelated dirty paths. No applicable AGENTS.md was found. This child owns readiness additions only and makes no commits. Original GameEngine methods, factory interface, module registration, subsystem traversal, recorder and GameState were inspected alongside setup/dependency manifests, build documentation and M19 evidence. Source references below use current repository lines.

Historical M0–M14 contracts and their time-bound pending/package wording remain immutable snapshots; current index, status and readiness qualify that evidence as components. M19 remains accepted at its bounded ten-provider grade; original UnicodeString and GameEngine MemoryPool portability are M20 deliverables under RC-004.

## 3. Automatically completed prerequisites

No product implementation or new bootstrap was required. Refreshed the 34-prerequisite manifest, completed M20's component PRE references and source-semantic entry inspection, updated packet readiness, and recorded the actual runtime edges below. Tool/module availability, offline GCC Debug configuration, 3,293-path source classification and pinned miniaudio provenance passed.

The old readiness assertion that M19 was next is superseded. No historical completed contract or acceptance was rewritten. PRE-029 now means the complete required offline runtime, rather than a lifecycle shell depending on later domains.

## 4. Confirmation-gated work

None at M20 entry. No package installation, dependency acquisition, persistent service, privileged command or product code change was needed. Later validation uses the declared host/environment facilities when available.

## 5. Required user actions

1. **PRE-012/PRE-016 at M22/M23 validation:** developer/session owner supplies accessible Arch RTX 4070 Vulkan and graphical-session facilities. Verify `vulkaninfo --summary` identifies the device and `VK_LAYER_KHRONOS_validation`, then run milestone-produced original-scene and original-interaction tests with validation enabled. Record device/driver/SDL, zero Validation Error/VUID output and real-source scene/UI evidence. Installed layers and previous M14 component scenes do not discharge these later tests.
2. **PRE-017 at M17 validation:** maintainer/CI runner owner supplies clean Ubuntu 26.04 and Fedora 44 x86-64 environments, installs documented distribution dependencies, and retains OS/architecture/package and all-four-preset configure/build/test logs. These may be disposable build/headless runners; a second physical GPU is not required.
3. **PRE-015 only for optional M18:** fixture owner supplies legally readable Windows 1.04 saves/replays with version/configuration provenance. Recordings may exist in original content; their suitability has not been inspected. Without suitable fixtures, record not validated and leave Linux release acceptance unaffected.

PRE-008 retail roots are supplied and directory/ignore checks pass. M20 consumes them read-only at initialization acceptance. Local independent loopback peers at M25/M16 use the established M11 developer environment; no remote service, credential, Windows peer or second machine is required.

## 6. Missing or unresolved prerequisites

None blocking M20 start after parent finalization/adoption. Missing original implementations, ATL/PCH/compiler errors and the source-classification changes are M20 work under the accepted resolution. They are not evidence that planning is blocked. PRE-029–034 and PRE-025–027 require future implementation/acceptance; no such success is claimed here.

The source inventory still labels many deferred files by previous domain owners. RC-001 and M20 explicitly authorize updating classification as those providers enter the runtime closure; those labels cannot override actual dependencies or defer required behavior to M21–M25. Scope size and routine link failures remain engineering work within M20. Demonstrated required asset/backend limitations retain the base-plan evidence gate.

## 7. Dependency graph and source semantic audit

Each edge below names its prerequisite. Historical branches preserve component history; M19 has bounded original support assurance. Self-owned implementation is not an entry dependency or graph cycle.

| Provider | Edge labels | Direct consumers |
|---|---|---|
| Existing repository/developer packages | PRE-001/002 | M0 historically; M19 completed; all remaining |
| Read-only retail owner | PRE-008 | M4 historically; M20–M25/M15–M17 validation |
| External host/session owner | PRE-012/016 | M6/M14 historically; M22/M23/M15–M17 validation |
| External clean runner owner | PRE-017 | M17 validation |
| Optional Windows fixture owner | PRE-015 | M18 only |
| M0 | PRE-003/004 | M1, M2, M19 |
| M1 | PRE-005 | M3, M4, M6, M19 |
| M2 | PRE-006 | M7 |
| M3 | PRE-007 | M4, M6, M7, M11 |
| M4 | PRE-009 | M5, M7, M8, M9, M10, M11, M12, M13, M20 |
| M5 | PRE-018 | M24 |
| M6 | PRE-019 | M8, M20, M23 |
| M7 | PRE-010 | M8, M9, M10, M13, M14, M20 |
| M8 | PRE-020 | M14, M20, M23 |
| M9 | PRE-021 | M14 |
| M10 | PRE-022 | M14 |
| M11 | PRE-014 | M25 |
| M12 | PRE-011/023 | M13, M20, M23 |
| M13 | PRE-024 | M14, M20, M23 |
| M14 | PRE-013 (component only) | M22 |
| M19 | PRE-028 (bounded support) | M20 |
| M20 | PRE-029 (complete offline runtime) | M21 |
| M21 | PRE-030 | M22, M24 |
| M22 | PRE-031 | M23 |
| M23 | PRE-032 | M15 |
| M24 | PRE-033 | M25, M15 |
| M25 | PRE-034 | M16 |
| M15 | PRE-025 | M16, M17 |
| M16 | PRE-026 | M17 |
| M17 | PRE-027 | M18 |

Semantic audit below checks dependencies hidden by class declarations, registries and lifecycle calls, beyond the milestone DAG. All required implementation maps to PRE-029/M20. This is source-backed allocation evidence; implementation still must prove complete compilation/linkage and runtime identity.

| Source evidence (FACT) | Required edge / M20 allocation | Later boundary preserved |
|---|---|---|
| `GeneralsMD/Code/GameEngine/Source/Common/GameMain.cpp:40–48`; `GameEngine/Include/Common/GameEngine.h:64–103` | Entry calls original factory/init/execute; original virtual methods and pure virtual factories require concrete providers, including source client/logic/modules and permitted removed-service adapters. Original classes and method bodies remain authoritative. | No lifecycle acceptance can wait for M21–M25 or use reduced replacement classes. |
| `GameEngine/Source/Common/GameEngine.cpp:301–354,412–482` | FS/local/archive, INI/CSF/GlobalData/name keys and data stores call real parsers, constructors and factories; M20 owns required portability and adapters. | Later scenarios consume already initialized source state. |
| `GameEngine/Source/Common/Thing/ModuleFactory.cpp:328–565,585–602,632–735`; `GameEngineDevice/Source/W3DDevice/Common/Thing/W3DModuleFactory.cpp:55–82` | Behavior and W3D registration retains construction/data callbacks; INI lookup invokes them, module instances use virtual interfaces and CRC/Xfer traverse module data. M20 owns each required registered provider and lifecycle. | M21/M22 can finish domain behavior beyond this closure, never supply missing required registry/vtable code. |
| `GameEngine/Source/Common/GameEngine.cpp:493–512` | GameClient, AI, GameLogic, teams, crates, players, recorder, radar and victory are instantiated before runtime acceptance. | M21 retains full real map/mission/skirmish scenarios. |
| `GameEngine/Source/Common/Recorder.cpp:394–433`; `GameEngine/Source/Common/System/SaveGame/GameState.cpp:309–340`; `GameEngine.cpp:539–553` | Recorder init/reset/update uses original mode/game info; GameState registers real campaign, terrain, team/player/logic/script/client/UI/particle and related snapshot instances; startup XferCRC and post-load run now. M20 owns these relationships and required implementations. | M24 retains full saves/replay/autosave/corruption/cross-compiler acceptance; its later status is no excuse to omit startup types or methods. |
| `GameEngine/Source/Common/GameEngine.cpp:755–895` | Real update calls radar/audio/client/messages/logic; execute consults tactical-view/script time and recorder cleanup. Null sinks stay at graphics/audio device boundaries and preserve source CPU state/resources. | M22 real Vulkan and M23 interactive/media acceptance remain later; M20 never needs a physical GPU/display. |
| `GameEngine/Source/Common/GameEngine.cpp:565–566,777–785` | Source explicitly sets offline TheNetwork = NULL and conditionally updates network. M20 resolves required symbols and rejects unsupported entry without fabricating a network; required non-network logic update still runs. | Full POSIX lobby/transport/lockstep is M25; no hidden network peer prerequisite for M20. |
| `GameEngine/Source/Common/GameEngine.cpp:198–239,709–740`; `GameEngine/Source/Common/System/SubsystemInterface.cpp:159–205` | Destructor, reset, post-load and reverse shutdown traverse actual providers; init registers after init/data load, so failure ownership needs explicit repair/testing. M20 owns safe partial initialization and exactly-once reverse teardown, including source UI reset calls and removed-service edges. | Teardown cannot be postponed until interactive or save/network milestones. |

Paths abbreviated with `GameEngine/` or `GameEngineDevice/` are under `GeneralsMD/Code/`. INFERENCE: the integrated ownership eliminates the recorded backward implementation dependency because every reachable provider, including newly discovered transitive providers, has one authorized owner. Validation needed before M20 completion remains actual compile/link/runtime witnesses, missing-provider negative control, real state transitions, all-stage failures and fixture/read-only-retail tests. The audit does not claim a static listing exhausts every translation unit.

## 8. Proposed milestone order

Declared and proposed order agree: preserved M0–M14 and completed M19; remaining **M20, M21, M22, M23, M24, M25, M15, M16, M17; optional M18**. M24 may follow M21 before the rendering branch when safe. No new IDs or reorder is required. M20's historical blocked state is preserved until a later delivery resume explicitly adopts this committed revision and replaces plan 01 with plan 02.

## 9. Milestone 0

**Not required** as a new milestone. Existing M0 toolchain/build foundations are available. Original runtime porting belongs to M20 and is not bootstrap work.

## 10. Per-milestone readiness

All 26 contracts contain Preconditions and Readiness checks; no historical completion claim is broadened. Pending entry checks use recorded provider acceptance and `cmake --list-presets`, plus the declared environment inspection. Product acceptance tests remain implementation deliverables.

| Milestone | Preconditions/providers | Current readiness result |
|---|---|---|
| M0 | PRE-001/002 | Historical bootstrap; sources/tools/configuration verified. |
| M1 | PRE-003/004, M0 | Historical components; remaining original consumers owned M20. |
| M2 | PRE-003/004, M0 | Historical renderer descriptors/shaders; original consumers M20/M22. |
| M3 | PRE-005, M1 | Historical fixture lifecycle; original runtime M20. |
| M4 | PRE-005/007/008, M1/M3 | Historical VFS/corpus; source-facing binding M20. |
| M5 | PRE-009, M4 | Historical codec/toy persistence; original full scenarios M24. |
| M6 | PRE-005/007/016, M1/M3 | Historical SDL/Wayland component; binding M20/interaction M23. |
| M7 | PRE-006/007/009, M2/M3/M4 | Available recording device; source producers bind in M20. |
| M8 | PRE-009/019/010, M4/M6/M7 | Available font/UI components; source startup M20, flows M23. |
| M9 | PRE-009/010, M4/M7 | Historical synthetic commands; real scenes M22. |
| M10 | PRE-009/010, M4/M7 | Historical effects; real producers M20/M22. |
| M11 | PRE-007/009, M3/M4 | Available local transport; original lockstep M25. |
| M12 | PRE-009/011, M4 | Available pinned audio component; original manager binding M20. |
| M13 | PRE-009/010/011/023, M4/M7/M12 | Available decoder/recording component; original required providers M20. |
| M14 | PRE-010/020/021/022/024, host inputs | Historical RTX validation on component scenes; real-source M22 remains mandatory. |
| M19 | PRE-001/002/004/005, M0/M1 | Completed bounded ten-provider acceptance; PRE-028 available. |
| M20 | PRE-028/009/019/010/020/011/023/024, M19/M4/M6/M7/M8/M12/M13; PRE-008 | Entry clear after committed adoption. Produces complete PRE-029 without M21–M25 or GPU prerequisites. |
| M21 | PRE-029, M20; PRE-008 | Waits for accepted original runtime, produces PRE-030. |
| M22 | PRE-030/013, M21/M14; PRE-008/012/016 | Waits for original simulation; host access gates hardware acceptance; produces PRE-031. |
| M23 | PRE-031/019/020/011/023/024, M22/M6/M8/M12/M13; PRE-008/016 | Waits for original scenes; session gates interaction; produces PRE-032. |
| M24 | PRE-030/018, M21/M5; PRE-008 | Full source persistence scenarios; produces PRE-033; no Windows input needed. |
| M25 | PRE-033/014, M24/M11; PRE-008 | Original local networking; produces PRE-034. |
| M15 | PRE-032/033, M23/M24; retail/session/GPU | Blocked on original providers, then integrated gameplay produces PRE-025. |
| M16 | PRE-034/025, M25/M15 | Real two-process complete match produces PRE-026. |
| M17 | PRE-025/026, M15/M16; PRE-017 plus Arch inputs | Clean distro provisioning gates release validation, produces PRE-027. |
| M18 | PRE-027, M17; optional PRE-015 | Suitable fixtures unverified; missing means not validated, release unaffected. |

## 11. Clean-environment simulation

**Pass with declared later external dependencies.** Conceptual clean checkout starts with documented x86-64 packages, committed source/dependencies/shaders and the four presets; no build cache, downloaded binary or private media is required by ordinary tests. M19's committed support providers and retained native components are available. M20 owns all missing original code required by fixture startup/update/reset/teardown and brings supplied read-only roots into its separate retail acceptance. Required real registries, recorder/game-state relationships and offline network symbols cannot be deferred. Null/recording sinks remove physical-device prerequisites without replacing source subsystems.

After accepted M20, M21 validates complete source scenarios. M22 receives host/GPU/layers for scenes and M23 receives session access for interactions; M24 independently consumes M21 state for full saves/replays. M25 consumes its CRC plus existing transport, then M15/M16 join complete gameplay/network behavior. M17 receives clean distro runners and Arch runtime evidence. Optional M18 never feeds the Linux release chain.

The future source-port results, new hardware scenarios and clean distro runners were simulated, not executed. Observed local configuration only verifies currently installed dependencies. Missing packages on a fresh machine require the documented host provisioning before entry; no undeclared package or secret is assumed.

## 12. Commands run and results

| Command / inspection | Observed result |
|---|---|
| `git status --short`, `git rev-parse HEAD` | Parent/compiler-owned paths only; source HEAD above. |
| `rg --files -g AGENTS.md -g '!original_game_symlink/**'` | No applicable file found (expected no-match). |
| `command -v cmake ninja gcc g++ clang clang++ pkg-config glslc` | All eight tools resolve. |
| `pkg-config --modversion sdl3 freetype2 fontconfig zlib libavformat libavcodec libavutil libswscale libswresample` | All resolve; SDL 3.4.14 and required distribution modules present. |
| `pacman -Q vulkan-validation-layers` | Installed 1.4.357.0-1; GPU not opened. |
| Retail directory checks and `git check-ignore original_game_symlink` | Supplied roots exist and link is ignored; no copying/hashing/writing. |
| `cmake --list-presets`, `cmake --preset linux-gcc-debug` | All four presets listed; offline configure/generate passed. |
| `python3 tools/original_source_classification.py --check` | 3,293 paths covered exactly once. Classification is inventory evidence, not runtime closure acceptance. |
| `python3 tests/audio/test_miniaudio_provenance.py` | 3/3 pass for project dependency pin/license/provenance. |
| Original factory/registry/subsystem/recorder/GameState inspection | Source-backed RC-002 ownership audit above; no runtime success inferred. |
| Initial optional `scripts` directory search and multi-file patch | No scripts directory; initial patch context mismatch caused no change and corrected patch applied. |
| Compiler structural validator with evidenced completed/blocked states allowed | Passed all 26 milestone contracts. |
| Final contract/graph/PRE/link inspection and `git diff --check` | Passed: 26 contracts, backward-only graph, 34 PRE IDs, all readiness sections/local links and preserved M19/M20 history; no whitespace errors. Final packet fingerprint returned to parent. |

No full build, milestone acceptance, retail traversal or physical device test was needed for this documentation audit.

## 13. Files changed

- `delivery/milestones/cnc-generals-zero-hour/M20-original-lifecycle.md` — precise component PREs, semantic audit and read-only-root entry checks.
- `delivery/milestones/cnc-generals-zero-hour/index.md` — current readiness result and next boundary.
- `delivery/milestones/cnc-generals-zero-hour/status.yaml` — readiness result only; historical statuses preserved.
- `delivery/readiness/cnc-generals-zero-hour/prerequisite-manifest.md` — current source/provenance, bounded M19, integrated M20 and consumers.
- `delivery/readiness/cnc-generals-zero-hour/readiness-report.md` — complete current audit and semantic dependency proof.

Compiler changes to M20–M25/index/status/traceability and parent workflow edits remain parent-owned. This child made no workflow, product-source, retail or implementation-plan changes and no commits. Configure refreshed ignored build outputs only.

## 14. Remaining blockers

None blocking M20 planning/implementation after committed adoption. PRE-012/016 gate later original hardware/interactive validation, PRE-017 gates M17 validation, PRE-015 applies only to optional M18. Parent commit/provenance finalization is required before this becomes a consumable handoff.

## 15. Exact recommended next step

Parent Delivery Planning finalizes the combined transaction and handoff. On the next delivery resume in `/home/ha/projects/CnC_Generals_Zero_Hour`, explicitly adopt the committed readiness-approved revision and create `delivery/plans/cnc-generals-zero-hour/milestone_20_plan_02.md` from revised M20 while preserving blocked plan 01. No implementation starts during this planning request.
