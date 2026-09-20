# Milestone readiness report

## 1. Overall readiness status

**READY_WITH_EXTERNAL_DEPENDENCIES** — replacement packet is internally ordered and M19 can begin after parent transaction/adoption finalization; later external validation environments have explicit owners and checks. This is planning readiness, not engine acceptance.

## 2. Scope and evidence basis

Product `cnc-generals-zero-hour`; 26-contract packet `delivery/milestones/cnc-generals-zero-hour/status.yaml`, index, traceability and all contracts reviewed. Authoritative remediation is `docs/zero-hour-source-engine-migration.md`, source/transaction-start commit `38b608d9109d13ad1c6e3a527fe34893512d4d3f`, plus unchanged base plan. Exact post-audit Markdown fingerprint and planning payload SHA are recorded canonically by the parent in `workflow/delivery-planning/state.yaml`, with provenance at `transaction.repository_commit`.

FACT: start status contained compiler-owned changes to M15–M18/index/status/traceability and new M19–M25; no unrelated dirty paths. No applicable AGENTS.md found. Source, CMake/presets/dependency manifests, build documentation, miniaudio provenance, M6/M14 host evidence and original-engine supplement were inspected. Observed commands below are facts; future original-source acceptance and clean-environment walking are simulations.

Historical M0–M14 contract snapshots remain unchanged, including time-bound original pending/missing-package wording. Current status/assurance and this re-audit supersede those snapshots for execution. Historical test success remains component evidence; missing original compile/link/runtime integration is now explicitly produced by M19–M25.

## 3. Automatically completed prerequisites

No feature prerequisites implemented. Added PRE-028–034 and entry checks to new contracts; restored PRE references to pending M15–M18; refreshed 34-row manifest and current status. READ-001 resolved by restoring the exact shared writable-path contract removed from index during compilation, fixing existing M1/M3/M4 anchor references without changing behavior. Existing tools/configure and pinned miniaudio provenance verified. Installed Khronos layer is now recorded correctly.

## 4. Confirmation-gated work

None at next entry. No installs, downloads, new scaffolding, persistent services, retail operations, privilege escalation or implementation. Future host/session access and runner provisioning are owned external actions below, not presumed authorization to change the machine.

## 5. Required user actions

1. **PRE-012/PRE-016 at M22/M23 validation:** developer/session owner provides access to the existing graphical/Vulkan host. Before acceptance, run `vulkaninfo --summary` in that session, confirm RTX 4070 and `VK_LAYER_KHRONOS_validation`, then run the new source-integrated tests produced by those milestones. M6/M14 already prove component host capability; original-scene/UI checks still must run. Layer package is installed, so no current install request.
2. **PRE-017 at M17 validation:** maintainer/CI owner provisions disposable Ubuntu 26.04 and Fedora 44 x86-64 runners/VMs/containers; install dependencies from `docs/building-linux.md`, record release/architecture/packages, then execute all four documented configure/build/test presets from clean source without configure downloads. Retain logs. Current Arch packages are not substitute evidence.
3. **PRE-015 only if M18 selected:** user provides legal readable Windows 1.04 save/replay fixtures and version/configuration provenance, possibly from read-only original content. No assumption that recordings are absent. Missing suitable fixtures permits “not validated” without blocking Linux release.

Read-only owned data PRE-008 is supplied. Two local independent peers need developer-permitted loopback execution as previously proved at M11; no second physical machine, credentials or online service.

## 6. Missing or unresolved prerequisites

None blocking M19; no unresolved product/architecture decision. Original ATL/ABI/factory/loader/consumer failures are deliverables of the explicitly scoped source providers, not missing external prerequisites. PRE-028–034 and PRE-025–027 remain pending implementation. Runtime host access is checked at its named boundary; clean distro runners remain unverified until M17. Unsupported required assets/backend limitations follow the existing evidence-gated source escalation, never silent substitution.

## 7. Dependency graph

Adjacency list uses output PRE IDs; self-owned creation is not a dependency cycle. Historical branches remain evidence history; new consumers require graded original-source outputs.

| Provider | Edge labels | Direct consumers |
|---|---|---|
| Existing source/developer tools | PRE-001/002 | M0 historically; M19 and all pending |
| User read-only retail | PRE-008 | M4 historically; M20–M25 and M15–M17 validation |
| External developer session/GPU | PRE-012/016 | M6/M14 historically; M22/M23 and M15–M17 validation |
| External clean runners | PRE-017 | M17 |
| Optional Windows fixture owner | PRE-015 | M18 only |
| M0 | PRE-003/004 | M1, M2, M19 |
| M1 | PRE-005 | M3, M4, M6, M19 |
| M2 | PRE-006 | M7 |
| M3 | PRE-007 | M4, M6, M7, M11 |
| M4 | PRE-009 | M5, M7, M8, M9, M10, M11, M12, M13, M20 |
| M5 | PRE-018 | M24 |
| M6 | PRE-019 | M8, M23 |
| M7 | PRE-010 | M8, M9, M10, M13, M14 |
| M8 | PRE-020 | M14, M23 |
| M9 | PRE-021 | M14 |
| M10 | PRE-022 | M14 |
| M11 | PRE-014 | M25 |
| M12 | PRE-011/023 | M13, M23 |
| M13 | PRE-024 | M14, M23 |
| M14 | PRE-013 (component only) | M22 |
| M19 | PRE-028 | M20 |
| M20 | PRE-029 | M21 |
| M21 | PRE-030 | M22, M24 |
| M22 | PRE-031 | M23 |
| M23 | PRE-032 | M15 |
| M24 | PRE-033 | M25, M15 |
| M25 | PRE-034 | M16 |
| M15 | PRE-025 | M16, M17 |
| M16 | PRE-026 | M17 |
| M17 | PRE-027 | M18 |

Transitive consumption of retained components in M22–M25 does not add a backward edge. No missing provider, cycle or reverse mandatory dependency on optional M18.

## 8. Proposed milestone order

Declared and proposed order agree: retained historical M0–M14, then **M19, M20, M21, M22, M23, M24, M25, M15, M16, M17; optional M18**. Numeric sort is wrong. M24 can execute after M21 independently of M22/M23 when safely selected. No reorder required. Previous fixed-membership delivery goal remains historical; later delivery resume must explicitly adopt the replacement packet, not silently add IDs to the old goal.

## 9. Milestone 0

**Not required** as a new milestone. Existing M0 build/preset foundations are reusable. M19 owns original source closure rather than forcing it into bootstrap or making it a circular entry requirement.

## 10. Per-milestone readiness

Every contract has Preconditions and Readiness checks. Historical contracts are preserved snapshots; current interpretation is below. All pending entry checks include inspecting direct-provider acceptance in status.yaml with original-source evidence grade and `cmake --list-presets`; feature tests belong to implementation/acceptance, not entry.

| Milestone | Prerequisites/providers | Current result / check |
|---|---|---|
| M0 | PRE-001/002 | Historical component; tools/presets/configure verified now. |
| M1 | PRE-003/004, M0 | Historical component; original support reopens at M19. |
| M2 | PRE-003/004, M0 | Historical mapping/shader component; original consumer assurance at M22. |
| M3 | PRE-005, M1 | Historical lifecycle fixture; original factory lifecycle M20. |
| M4 | PRE-005/007/008, M1/M3 | Retained VFS/corpus evidence and roots; source-facing initialization M20. |
| M5 | PRE-009, M4 | Historical synthetic persistence; original traversal M24. |
| M6 | PRE-005/007/016, M1/M3 | Prior Wayland smoke; original input M23. |
| M7 | PRE-006/007/009, M2/M3/M4 | Retained recording-device component; real consumers M22. |
| M8 | PRE-009/019/010, M4/M6/M7 | Retained UI/font fixture; original UI M23. |
| M9 | PRE-009/010, M4/M7 | Retained synthetic world commands; source producers M22. |
| M10 | PRE-009/010, M4/M7 | Retained effects fixture; original WWShade M22. |
| M11 | PRE-007/009, M3/M4 | Retained transport/real loopback evidence; source lockstep M25. |
| M12 | PRE-009/011, M4 | Vendored pin tests 3/3; original manager M23. |
| M13 | PRE-009/010/011/023, M4/M7/M12 | Retained decoder/recorder evidence; original video manager M23. |
| M14 | PRE-010/020/021/022/024 and host inputs | Retained real GPU but synthetic scenes; original scenes M22. |
| M19 | PRE-001/002/004/005, M0/M1 | Entry clear; source/tools/configure pass; produces PRE-028. |
| M20 | PRE-028/009, M19/M4; PRE-008 | Wait for M19 source acceptance; roots supplied; produces PRE-029. |
| M21 | PRE-029, M20; PRE-008 | Wait for original initialization; produces PRE-030. |
| M22 | PRE-030/013, M21/M14; PRE-008/012/016 | Wait for original simulation; refresh host access at real-scene validation; produces PRE-031. |
| M23 | PRE-031/019/020/011/023/024; PRE-008/016 | Wait for M22; use native components at original seams; produces PRE-032. |
| M24 | PRE-030/018, M21/M5; PRE-008 | Wait for actual source state; no Windows fixture gate; produces PRE-033. |
| M25 | PRE-033/014, M24/M11; PRE-008 | Wait for original CRC; local peers only; produces PRE-034. |
| M15 | PRE-032/033, M23/M24; retail/session/GPU | Remains blocked on new providers; original sessions produce PRE-025. |
| M16 | PRE-034/025, M25/M15 | Wait for source lockstep and gameplay; produces PRE-026. |
| M17 | PRE-025/026, M15/M16; PRE-017 | Wait for integrated acceptance; clean runners at release validation; produces PRE-027. |
| M18 | PRE-027, M17; optional PRE-015 | Fixtures unverified; not validated is permitted. |

## 11. Clean-environment simulation

**Pass with declared external dependencies.** From clean source, install documented compiler/development packages first. Committed miniaudio and shaders build offline; four CMake presets exist without private inputs. M19 starts from source/component foundations, not a prebuilt original engine or cached proprietary binary. M19 produces real support closure before M20 original factories/data initialization; supplied read-only roots enter only opt-in checks. M21 consumes initialization to run original maps/state. M22 consumes state and retained device backend and receives session/layer access at its validation boundary; M23 integrates original UI/media. M24 uses M21 source state for snapshots and M25 consumes its CRC with retained local transport. M15 joins original UI/media/persistence; M16 joins gameplay with lockstep. M17 receives clean distro runners and Arch retail/device context; optional M18 cannot block it.

No credentials, service, prior build cache, retail data for ordinary tests, or future provider assumed earlier than declared. Missing baseline packages on another clean host are an explicit provisioning stop, not a current M19 blocker. Clean distro provisioning and all new original-engine feature acceptance are simulated, not executed. Current configure verifies declared installed dependencies only.

## 12. Commands run and results

| Command / inspection | Result |
|---|---|
| `git status --short`, `git rev-parse HEAD` | Parent-owned compiler paths only; HEAD above. |
| `rg --files -g AGENTS.md` | No applicable file found (expected no-match). |
| `command -v cmake ninja gcc g++ clang clang++ pkg-config glslc`, `uname -m` | All resolve; x86_64. |
| `pkg-config --modversion sdl3 freetype2 fontconfig zlib libavformat libavcodec libavutil libswscale libswresample` | All resolve; SDL 3.4.14, FFmpeg modules 63.1.101/63.1.101/61.1.101/10.1.101/7.1.101. |
| `pacman -Q vulkan-validation-layers` | Installed 1.4.357.0-1. |
| `cmake --list-presets` | Four native presets. |
| `cmake --preset linux-gcc-debug` | Configure/generate pass; no download. |
| `python3 tests/audio/test_miniaudio_provenance.py` | 3/3 pass, pinned project dependency only. |
| Local retail directory and `git check-ignore original_game_symlink` checks | Pass; no copy/hash/write of retail. |
| M6/M14 existing host evidence inspection | Prior Wayland and validation-enabled RTX component evidence; no fresh hardware acceptance claimed. |
| Wrong initial evidence path / .github discovery | No such path; corrected to evidence/qa M14; no bootstrap change needed. |
| Initial manifest patch attempt | Rejected duplicate-target patch; no mutation, rerun as valid update. |
| Compiler structural validator | Pass: 26 milestone records/contracts; historical completed/blocked statuses explicitly allowed. |
| Packet graph/PRE/headings/local links and `git diff --check` | Pass: 26 contracts, backward-only graph, preserved historical statuses, all readiness headings, 34 PRE IDs and local link targets. Initial ID check found unpadded new manifest IDs; corrected to PRE-028–034 and rerun passed. |

No full game build/test/acceptance claimed.

## 13. Files changed

- `delivery/milestones/cnc-generals-zero-hour/M19-original-support.md`
- `delivery/milestones/cnc-generals-zero-hour/M20-original-lifecycle.md`
- `delivery/milestones/cnc-generals-zero-hour/M21-original-simulation.md`
- `delivery/milestones/cnc-generals-zero-hour/M22-original-rendering.md`
- `delivery/milestones/cnc-generals-zero-hour/M23-original-interaction.md`
- `delivery/milestones/cnc-generals-zero-hour/M24-original-persistence.md`
- `delivery/milestones/cnc-generals-zero-hour/M25-original-network.md`
- `delivery/milestones/cnc-generals-zero-hour/M15-single-player.md`
- `delivery/milestones/cnc-generals-zero-hour/M16-lan-match.md`
- `delivery/milestones/cnc-generals-zero-hour/M17-release.md`
- `delivery/milestones/cnc-generals-zero-hour/M18-windows-compatibility.md`
- `delivery/milestones/cnc-generals-zero-hour/index.md`
- `delivery/milestones/cnc-generals-zero-hour/status.yaml`
- `delivery/readiness/cnc-generals-zero-hour/prerequisite-manifest.md`
- `delivery/readiness/cnc-generals-zero-hour/readiness-report.md`

Milestone changes add prerequisite/check contracts only; index restores a referenced path contract and current readiness; status updates readiness without altering implementation history. No workflow, source or retail changes, no commits by this child. Configure touched ignored local build outputs only.

## 14. Remaining blockers

None for M19. PRE-012/016 are fresh-access gates at M22/M23 and later runtime checks; PRE-017 gates M17 validation; PRE-015 only optional M18. Parent must commit/finalize provenance before emitting a usable handoff.

## 15. Exact recommended next step

Parent Delivery Planning finalizes the combined planning transaction and handoff. On the next delivery resume in `/home/ha/projects/CnC_Generals_Zero_Hour`, explicitly adopt the readiness-approved replacement scope and create the M19 minimal-slice plan from `delivery/milestones/cnc-generals-zero-hour/M19-original-support.md`; do not resume old M15 slice execution or begin implementation during this planning request.
