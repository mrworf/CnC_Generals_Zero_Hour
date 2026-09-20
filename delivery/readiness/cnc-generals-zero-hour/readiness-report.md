# Milestone readiness report

## 1. Overall readiness status

**READY_WITH_EXTERNAL_DEPENDENCIES** — the packet has a complete acyclic provider graph and M0 can begin; externally owned checks are explicit at later boundaries.

This is planning readiness, not completion of any implementation milestone.

## 2. Scope and evidence basis

Product `cnc-generals-zero-hour`; packet `delivery/milestones/cnc-generals-zero-hour/status.yaml`; authoritative `docs/zero-hour-linux-port-plan.md` §7 dependency graph and §8 milestone preconditions. Source/transaction-start HEAD: `8dc0285ff6c3346273973dcd874e6983060bb3e7`. Source SHA-256: `25554c2db1273e263b574b8563615c88ed48eb4f7672e5a21e19fe119eae7b33`.

Audited packet revision: `sha256:7f486befe4e182f46fbc7f090d757ad7a19bcabddf53ce6b622b6ba90596de4d`, computed from repository root by `sha256sum delivery/milestones/cnc-generals-zero-hour/*.md | sha256sum` (status.yaml excluded). Planning transaction provenance is the canonical `workflow/delivery-planning/state.yaml#transaction.repository_commit`; the parent finalizes it before handoff. Audit date: 2026-09-20 UTC.

FACT: baseline Git status contained only parent-owned untracked `delivery/milestones/` and `workflow/`; source plan and tracked code were unchanged. No applicable AGENTS.md or CMake files/presets were found. All 19 contracts, index, traceability, status and source prerequisite/order sections were inspected. Current host tool/package/module checks are observed facts; completed future providers and clean execution below are simulations. Source-recorded host Vulkan evidence was not rerun in this restricted sandbox.

## 3. Automatically completed prerequisites

None of the game implementation prerequisites were implemented. Clarified every milestone's existing preconditions with PRE IDs, provider/observable-state contracts and entry readiness checks. Added PRE-018–027 to name already-required subsystem outputs without changing source IDs/scope or dependency order. Recorded package/readability checks and the absence of validation layers. All implementation statuses remain pending.

## 4. Confirmation-gated work

None required for compilation/readiness. No packages, persistent services, source scaffolding or implementation were installed or started. A later network permission may be required for the source-authorized initial M12 miniaudio acquisition; normal builds remain offline.

## 5. Required user actions

1. **PRE-016, first M6 interactive closure:** developer supplies accessible desktop/software display; use the M6 implementation's documented SDL window smoke to prove input, text, focus, resize and shutdown without creating a GPU device. Headless event implementation can proceed before this check.
2. **PRE-012, first M14 device validation:** developer installs Arch `vulkan-validation-layers` via normal host package management (e.g. `sudo pacman -S vulkan-validation-layers`). Outside device/display sandbox isolation, `pacman -Q vulkan-validation-layers` must succeed and `vulkaninfo --summary` must show RTX 4070 and `VK_LAYER_KHRONOS_validation`. M14 then proves actual SDL_GPU creation, scenes and validation. This is a validation prerequisite, not a new runtime package requirement.
3. **PRE-017, first M17:** maintainer/CI owner supplies disposable Ubuntu 26.04/Fedora 44 x86-64 runners/VMs/containers. Install M0-documented dependencies, record release/architecture/provisioning logs, then prove all four presets and asset-free checks offline. No second physical GPU host is required.
4. **PRE-015, optional M18 only:** user supplies readable legal Windows 1.04 save/replay fixtures with provenance/version/configuration if compatibility validation is desired. Otherwise record not validated.

PRE-008 data is already supplied; preserve its read-only symlink and local configuration. Do not reacquire or redistribute it.

## 6. Missing or unresolved prerequisites

No unspecified provider or unresolved product decision blocks M0. PRE-003–007, PRE-009–011, PRE-013–014 and PRE-018–027 are expected future milestone outputs. Miniaudio pinning is explicitly the initial M12 task. Missing validation layers (PRE-012), unverified accessible display (PRE-016) and unprovisioned clean runners (PRE-017) have owners and verifiable resolution above. Optional fixtures (PRE-015) cannot block release.

Conditional renderer fallback, unexpected formats, English shaping and loopback discovery choices remain the source's evidence-driven decisions at their named consumers. This audit does not pre-approve a backend switch or waive acceptance.

## 7. Dependency graph

Adjacency table: each provider produces the labeled prerequisites before the listed direct consumers. Self-owned production occurs within the provider's scope before acceptance; it is not a graph cycle. Consumers inherit transitive build/runtime contracts.

| Provider | Edge labels / outputs | Direct consumer milestones |
|---|---|---|
| Existing repository | PRE-001 | M0 |
| Developer baseline packages | PRE-002 | M0; inherited by all |
| User read-only data | PRE-008 | M4; later retail consumers inherit |
| External accessible display | PRE-016 | M6; later interactive consumers inherit |
| External GPU/validation host | PRE-012 | M14; M15–M17 inherit |
| External clean distribution runners | PRE-017 | M17 |
| Optional external Windows fixtures | PRE-015 | M18 only |
| M0 | PRE-003, PRE-004 | M1, M2 |
| M1 | PRE-005 | M3, M4, M6 |
| M2 | PRE-006 | M7 |
| M3 | PRE-007 | M4, M6, M7, M11 |
| M4 | PRE-009 | M5, M7, M8, M9, M10, M11, M12, M13 |
| M5 | PRE-018 | M15 |
| M6 | PRE-019 | M8 |
| M7 | PRE-010 | M8, M9, M10, M13, M14 |
| M8 | PRE-020 | M14, M15, M16 |
| M9 | PRE-021 | M14, M15 |
| M10 | PRE-022 | M14, M15 |
| M11 | PRE-014 | M16 |
| M12 | PRE-011, PRE-023 | M13, M15 |
| M13 | PRE-024 | M14, M15 |
| M14 | PRE-013 | M15 |
| M15 | PRE-025 | M16, M17 |
| M16 | PRE-026 | M17 |
| M17 | PRE-027 | M18 |

The [manifest](prerequisite-manifest.md) defines all 27 labels. No missing/backward provider, dependency cycle or reverse dependency from optional M18 was found.

## 8. Proposed milestone order

Declared and proposed order are identical: **M0, M1, M2, M3, M4, M5, M6, M7, M8, M9, M10, M11, M12, M13, M14, M15, M16, M17; optional M18**. Preserve source direct edges even when redundant transitively. M2 may execute after M0 independently of M1–M6; other parallel branches remain governed by status.yaml. No reorder is needed.

## 9. Milestone 0

**Not required as a new readiness milestone.** Retain the source-defined M0 already compiled in the packet. It owns PRE-003/004: explicit source/dependency inventory, common CMake graph/presets, dependency probes, synthetic smoke/label conventions and baseline CI/build guidance. Creating these during this planning audit would prematurely implement approved M0 work. M0 starts from existing source and installed packages without requiring its own future CMake outputs.

## 10. Per-milestone readiness

Each linked contract contains exact `Preconditions` and `Readiness checks`, including observable provider evidence. Existing `Required Validation` remains exit validation, not an obligation to have unfinished feature tests pass at entry.

| Milestone | Entry providers / PRE IDs | Current result / boundary |
|---|---|---|
| [M0](../../milestones/cnc-generals-zero-hour/M0-build-graph.md) | PRE-001/002 | Source manifests/tools/modules verified; begin bootstrap. |
| [M1](../../milestones/cnc-generals-zero-hour/M1-portable-foundations.md) | PRE-003, PRE-004 | Pending M0 source/build contracts. |
| [M2](../../milestones/cnc-generals-zero-hour/M2-renderer-closure.md) | PRE-003, PRE-004 | Pending M0; offline source/API/shader analysis, no GPU gate. |
| [M3](../../milestones/cnc-generals-zero-hour/M3-headless-startup.md) | PRE-005 | Pending M1 portable foundations. |
| [M4](../../milestones/cnc-generals-zero-hour/M4-retail-vfs.md) | PRE-005, PRE-007; PRE-008 | Pending M1/M3; retail roots supplied; full traversal is M4 work. |
| [M5](../../milestones/cnc-generals-zero-hour/M5-persistence.md) | PRE-009 | Pending M4; Linux fixtures generated here, no Windows dependency. |
| [M6](../../milestones/cnc-generals-zero-hour/M6-sdl-platform.md) | PRE-005, PRE-007; PRE-016 | Pending M1/M3; PRE-016 gates interactive closure only. |
| [M7](../../milestones/cnc-generals-zero-hour/M7-renderer-core.md) | PRE-006, PRE-007, PRE-009 | Pending M2/M3/M4; recorder is produced here. |
| [M8](../../milestones/cnc-generals-zero-hour/M8-ui-fonts.md) | PRE-009, PRE-019, PRE-010 | Pending M4/M6/M7; locale/font decision owned here. |
| [M9](../../milestones/cnc-generals-zero-hour/M9-world-rendering.md) | PRE-009, PRE-010 | Pending M4/M7; validate world commands without GPU. |
| [M10](../../milestones/cnc-generals-zero-hour/M10-effects.md) | PRE-009, PRE-010 | Pending M4/M7; validate effects commands without GPU. |
| [M11](../../milestones/cnc-generals-zero-hour/M11-lan-transport.md) | PRE-007, PRE-009 | Pending M3/M4; actual loopback peers plus virtual failure injection. |
| [M12](../../milestones/cnc-generals-zero-hour/M12-audio.md) | PRE-009 | Pending M4; pin/review miniaudio before integration. |
| [M13](../../milestones/cnc-generals-zero-hour/M13-video.md) | PRE-009, PRE-010, PRE-011, PRE-023 | Pending M4/M7/M12; headless media commands then M14 pixels. |
| [M14](../../milestones/cnc-generals-zero-hour/M14-gpu-acceptance.md) | PRE-010, PRE-020, PRE-021, PRE-022, PRE-024; PRE-008/012/016 | Pending M7/M8/M9/M10/M13 and PRE-012 host/layers; implement device before opening it. |
| [M15](../../milestones/cnc-generals-zero-hour/M15-single-player.md) | PRE-018, PRE-020, PRE-021, PRE-022, PRE-011, PRE-023, PRE-024, PRE-013 | Pending M5/M8/M9/M10/M12/M13/M14 acceptance; hardware cannot be bypassed. |
| [M16](../../milestones/cnc-generals-zero-hour/M16-lan-match.md) | PRE-020, PRE-014, PRE-025 | Pending M8/M11/M15; complete two local peers, no physical second host. |
| [M17](../../milestones/cnc-generals-zero-hour/M17-release.md) | PRE-025, PRE-026; PRE-017 | Pending M15/M16 and PRE-017 clean environments; replay Arch hardware checks. |
| [M18](../../milestones/cnc-generals-zero-hour/M18-windows-compatibility.md) | PRE-027; PRE-015 | Pending M17; absent optional PRE-015 means not validated. |

Common later entry check: inspect completed direct-provider acceptance evidence in status.yaml and run `cmake --list-presets` after M0 supplies the presets. This is a future executable check; no nonexistent configure/build command was presented as already successful.

## 11. Clean-environment simulation

**Pass with declared external dependencies.** Starting from a clean checkout with no generated outputs, the developer installs declared baseline distribution packages before M0. M0 supplies shared build/test instructions and the four presets; M1/M2 can then produce portable foundations and source/API/shader mapping. M3's null executable enables M4 with explicitly supplied read-only data. M4 produces its logical corpus for M5 and the renderer/media/network consumers.

M6 closes only after the external interactive smoke. M7 consumes completed source/API, headless and VFS providers; M8–M10 consume recorder/platform/corpus contracts. M11 provides loopback/virtual transport without an external peer; M12 acquires and pins miniaudio before compiling it, then supplies audio to M13. Headless renderer/media suites do not require GPU hardware. M14 receives the declared host/layers/display and implements the real device before its device-open test. M15/M16 consume completed single-player/transport and rendering evidence; M17 provisions clean distribution environments, retains separate Arch hardware acceptance and excludes optional compatibility fixtures from mandatory suites. M18 cannot influence preceding release acceptance.

Fresh distribution provisioning, future compile/test/interactive/GPU acceptance and miniaudio resolution were simulated, not executed. No undeclared service, credential, cache, private build path, retail input for ordinary tests, or future provider was assumed available at an earlier boundary. A clean host missing baseline packages would stop before M0 until provisioned; this current Arch host has the installed portion verified.

## 12. Commands run and results

| Command/check | Result |
|---|---|
| `git status --short`, `git rev-parse HEAD` | Parent-owned untracked packet/workflow; HEAD as above |
| `rg --files` for AGENTS/CMake/legacy manifests | Legacy .dsw/.dsp inputs found; no AGENTS.md or CMake project/presets (no-match exit 1 expected) |
| `test -r GeneralsMD/Code/RTS.dsw && test -r GeneralsMD/Code/RTS.dsp` | Pass |
| `command -v cmake ninja gcc g++ clang clang++ pkg-config glslc` | All resolve under /usr/bin |
| `uname -m` | x86_64 |
| `pkg-config --modversion sdl3 freetype2 fontconfig zlib libavformat libavcodec libavutil libswscale libswresample` | All resolve; SDL3 3.4.14; FFmpeg modules 63.1.101/63.1.101/61.1.101/10.1.101/7.1.101 |
| `pacman -Q cmake ninja gcc clang pkgconf shaderc sdl3 freetype2 fontconfig zlib ffmpeg` | Installed versions recorded in prerequisite manifest |
| `pacman -Q vulkan-validation-layers` | Expected failure: package not installed |
| `test -d original_game_symlink`, base-root directory and INIZH.big readability, `git check-ignore original_game_symlink` | Pass; ignored local corpus remains untouched |
| `git diff --check` | Pass for tracked diff; parent validates all new artifacts before commit |
| Packet hash command in §2 | Hash recorded above |
| Read-only Python/YAML packet validation | Pass: all 19 contracts re-read; pending statuses, source dependency order, optional M18 isolation, unique PRE-001–027 rows, readiness headings, local Markdown links and whitespace verified |

The initial combined inspection ended with exit 1 because no CMake/AGENTS files matched; earlier tool/readability checks passed. No build or milestone acceptance has been claimed.

## 13. Files changed

- Every `delivery/milestones/cnc-generals-zero-hour/M0–M18-*.md` contract: explicit PRE provider and readiness checks, production-versus-entry boundary.
- `delivery/milestones/cnc-generals-zero-hour/index.md`: readiness outcome and artifact links.
- `delivery/milestones/cnc-generals-zero-hour/status.yaml`: readiness status/artifact references; implementation statuses unchanged.
- `delivery/readiness/cnc-generals-zero-hour/prerequisite-manifest.md`: normalized inventory, provider/owner/classification and acquisition evidence.
- `delivery/readiness/cnc-generals-zero-hour/readiness-report.md`: this audit.

No source, retail, implementation, workflow-state or Git commit changes were made by the child auditor. The parent owns the coherent final planning transaction and exact path staging.

## 14. Remaining blockers

**None for next milestone M0.** Future conditional boundary gates: PRE-016 at M6 closure; PRE-012 at M14 validation; PRE-017 at M17 validation. PRE-015 affects optional M18 only. Planning handoff remains contingent on the parent successfully committing/finalizing provenance.

## 15. Exact recommended next step

After the parent finalizes the planning transaction, begin M0 slice planning from `delivery/milestones/cnc-generals-zero-hour/M0-build-graph.md` in repository `/home/ha/projects/CnC_Generals_Zero_Hour`, using the readiness-approved packet status.yaml. No game implementation is started by this handoff.
