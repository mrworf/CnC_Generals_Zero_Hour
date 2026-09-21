# Milestone readiness report — renderer migration

## 1. Overall readiness status

**READY_WITH_EXTERNAL_DEPENDENCIES** — M29 is executable from the completed repository contracts; M30 follows M29 and needs the declared developer RTX/Vulkan/graphical session only at its hardware acceptance boundary. This does not claim bgfx or original retail rendering is implemented. If that session is unavailable when M30 reaches acceptance, its gate becomes `BLOCKED_BY_USER_ACTION`.

## 2. Scope and evidence basis

Product `cnc-generals-zero-hour`; selected 31-contract packet `delivery/milestones/cnc-generals-zero-hour/status.yaml` and its `index.md`. Declared list order, `dependencies`, and `packet_adoption.pending_execution_order` govern, not numeric IDs. Source authority: accepted `docs/zero-hour-renderer-backend-migration.md` RB-01–RB-04 at `64c7632dcc93a5a5d9fc1519467140828a3a8813`, canonical `workflow/reconciliation/renderer-bgfx-2026-09-21/reconciliation-result.yaml` at `25ef60575bfc13aeac5a6aa7146da1af2626e59c`, and preserved earlier runtime/source authority recorded in status. Parent-owned transaction-start HEAD `c19f79fc71f72b33e37e8080a6d5f0a78a1bee1a`; final packet fingerprint and planning commit remain parent-owned/pending. At invocation the worktree contained only compiler-owned M22/M24/index/status/traceability edits and new M29/M30; no unrelated dirty paths were observed. No applicable AGENTS.md was found.

FACT: M0–M14, M19, M26–M28, M20–M21 are completed at their recorded grades. M22 and M24 retain unfinished plans and completed slices. FACT: the public bgfx probe source exists; its accepted result proves the exact camera clear abstraction on the prior host, not integrated device/retail acceptance. FACT: `third_party/` contains miniaudio but no bgfx/bx/bimg or committed bgfx shader compiler. INFERENCE: source pinning, license review and integration can be performed as M29/M30 work because RB-03/RB-02 allocate them there; no clean-checkout configure may assume a global bgfx install. This does not preapprove an unreviewed license or host package installation.

## 3. Automatically completed prerequisites

No product code, dependency installation, service or new bootstrap was changed. PRE-001–011, PRE-013/014, PRE-018–024, PRE-028–030 and PRE-035–037 retain their recorded, bounded accepted evidence. M29 enters with M8/M9/M10 command families (PRE-020–022), M2/M7 transitive contracts (PRE-006/010), four presets and baseline tools (PRE-002). This audit observed all four presets, compiler/build/shader tools, SDL3/FFmpeg/font/zlib modules, and the project-owned public probe; it did not rerun milestone acceptance. PRE-038/039/040 are newly normalized outputs, not already satisfied inputs.

## 4. Confirmation-gated work

None before M29 entry: accepted RB-02/RB-03 already authorize a bgfx dependency and its shader tooling within the device migration. M29 must document a pinned, licensed, repository-compatible toolchain and reproducible acquisition; M30 integrates the pinned runtime. A materially different vendoring footprint, incompatible license term, OS package installation, or developer-workflow change beyond that accepted scope would require a new gate with exact files/effects/reversal. No dependency was acquired or changed by readiness.

## 5. Required user actions

1. `PRE-012`/`PRE-016`, first consuming hardware boundary **M30 acceptance**: developer/session owner provides an accessible RTX 4070 Vulkan graphical session and Khronos validation layer. Run `vulkaninfo --summary` in that session and inspect device/layer output; execute M30's exact public probe and bgfx GPU tests with `VK_INSTANCE_LAYERS=VK_LAYER_KHRONOS_validation`. Retain driver/device/layer and zero relevant validation-error evidence. Prior M14/probe results do not establish a fresh M30 backend.
2. `PRE-008`, M22 original retail validation: owner supplies the existing ignored Zero Hour/base Generals English roots read-only via local opt-in inputs; rerun availability/ignore checks and M22 source-scene tests. Never write, copy into tracked files, hash or expose physical retail paths. M29/M30 ordinary tests do not consume retail assets.
3. `PRE-017`, M17 release acceptance: maintainer/CI owner supplies disposable clean Ubuntu 26.04 and Fedora 44 x86-64 runners with documented dependencies and captures OS/package/all-four-preset configure/build/test and installed-prefix results. No second physical GPU is required.
4. `PRE-015`, optional M18 only: fixture owner supplies legally readable Windows 1.04 saves/replays with provenance. Existing recordings may be suitable but were not inspected; missing fixtures mean optional compatibility is not validated, never a Linux release blocker.

## 6. Missing or unresolved prerequisites

None at M29 entry. PRE-038/039 are M29 outputs, PRE-040 is M30 output, PRE-031 is M22 output, and PRE-033 is M24 output; they are not circular entry preconditions. Current lack of a committed bgfx dependency is implementation scope, not proof of an external package requirement. If M29 cannot establish a licensed/pinned offline toolchain or another required public API behavior fails, reopen the source §9 evidence gate with a named reproducer rather than silently weakening coverage. Current owner is M29/M30 as applicable.

## 7. Dependency graph

Provider-to-consumer edges, labeled by the normalized manifest IDs (transitive/historical edges remain in `status.yaml`):

| Provider | IDs | Direct consumers |
|---|---|---|
| Repository/toolchain | PRE-001/002 | M0, M19, remaining implementation |
| Retail owner | PRE-008 | M4 historically; M22–M25, M15–M17 validation |
| Host GPU/session owner | PRE-012/016 | M14 historically; M30, M22/M23, M15–M17 hardware/interaction validation |
| Clean runner owner | PRE-017 | M17 acceptance |
| Optional Windows fixture owner | PRE-015 | M18 only |
| M0–M7 | PRE-003–010, PRE-018/019 as recorded | Historical native and original providers |
| M8 | PRE-020 | M14, M28, M29, M23 |
| M9 | PRE-021 | M14, M29 |
| M10 | PRE-022 | M14, M29 |
| M11–M14 | PRE-011/013/014/023/024 | Historical consumers; PRE-013 to M30 comparison |
| M19→M26→M27→M28→M20→M21 | PRE-028/035/036/037/029/030 | Original-source chain; M21 to M22 |
| M29 | PRE-038/039 | M30; PRE-038 also M22 |
| M30 | PRE-040 | M22 |
| M22 | PRE-031 | M23 and M24 (original draw provider) |
| M23 | PRE-032 | M15 |
| M24 | PRE-033 | M25 and M15 |
| M25 | PRE-034 | M16 |
| M15→M16→M17 | PRE-025/026/027 | M16→M17→optional M18 |

There is no backward provider edge or cycle. M23 and M24 are both downstream of M22 and can execute in either dependency-safe order; the declared order retains M23 then M24. M24's retained early slices do not waive M22's draw-provider gate for its remaining slices.

## 8. Proposed milestone order

Declared and proposed order agree: completed **M0–M14, M19, M26–M28, M20–M21**; remaining **M29 → M30 → M22 → M23 → M24 → M25 → M15 → M16 → M17**, then optional **M18**. `status.yaml` and `index.md` declare this order; no renumbering. M29 can be planned before blocked M22 without discarding M22 or M24 slice history.

## 9. Milestone 0

**Not required.** Historical M0 is completed. Shader/toolchain pinning belongs to M29; bgfx device integration belongs to M30. Neither is shared pre-feature repository bootstrap or a reason to restart M0.

## 10. Per-milestone readiness

All 31 contracts now contain `Preconditions` and `Readiness checks`. Historical contract wording is retained as time-bound history and is qualified by status/index/this report. Each row's check is the contract's concrete command or inspection; `cmake --list-presets` is the common clean-entry check. Future acceptance tests are not treated as preconditions.

| Milestone(s) | Preconditions and check | Current boundary result |
|---|---|---|
| M0 | PRE-001/002; source/tool/package and preset checks | Completed historical bootstrap. |
| M1–M2 | M0 PRE-003/004; provider evidence/presets | Completed native ABI/renderer component contracts. |
| M3 | M1 PRE-005; provider evidence/presets | Completed headless component. |
| M4 | M1/M3 PRE-005/007 and retail PRE-008 for corpus; provider/retail checks | Completed; retail check must be repeated at later gates. |
| M5–M6 | M4 PRE-009; M6 also M1/M3 and display PRE-016; provider/preset/session checks | Completed component contracts. |
| M7 | M2/M3/M4 PRE-006/007/009; provider/presets | Completed historical recorder, not ordered clear. |
| M8–M10 | M4/M6/M7 or M4/M7 PRE-009/010/019; provider/presets | Completed command/shader baselines. |
| M11–M13 | Recorded M3/M4/M7/M12 edges and PRE-011/014/023/024; provider/presets | Completed component transport/audio/video. |
| M14 | M7–M10/M13 plus PRE-008/012/016; prior RTX/layer/pixel evidence | Completed historical SDL_GPU component acceptance only. |
| M19 | M0/M1 PRE-001/002/004/005; source identity check | Completed bounded original support. |
| M26→M27→M28 | PRE-028→035→036 plus existing M4/M6/M7/M8/M12/M13 components; acceptance/ledger checks | Completed scoped original providers. |
| M20→M21 | PRE-037→029, retail PRE-008; original runtime/simulation evidence | Completed at recorded source-owned grades. |
| M29 | M8/M9/M10 PRE-020/021/022 and transitive PRE-006/010, PRE-002; inspect completed evidence, run `cmake --list-presets`, inspect RB-01/03 and public probe source | **First executable milestone.** PRE-038/039 are its work/acceptance. No GPU/retail entry gate. |
| M30 | M29 PRE-038/039 and historical M14 PRE-013; inspect acceptance/pin/license, run `cmake --list-presets`; at hardware gate run `vulkaninfo --summary` and validation-enabled new tests | Waits for M29; PRE-012/016 external at hardware acceptance; produces PRE-040. |
| M22 | M21 PRE-030 and M30 PRE-040; inspect source/device acceptance/presets; retail/GPU/session PRE-008/012/016 at real-scene gate | Blocked until M30; preserves plan 01 through 06C3B3B; produces PRE-031 at scene/draw acceptance. |
| M23 | M22 PRE-031 and M6/M8/M12/M13; inspect evidence/presets/session | Waits for M22; produces PRE-032. |
| M24 | M22 PRE-031 plus M5 PRE-018 and M21 PRE-030; inspect draw evidence/presets/retail at real-map gate | Blocked until M22; preserves slices 01–05B; produces PRE-033. |
| M25 | M24 PRE-033 and M11 PRE-014; inspect original persistence/local transport evidence | Waits for M24; produces PRE-034. |
| M15 | M23 PRE-032 and M24 PRE-033, retail/session/GPU; inspect original gameplay evidence | Waits for both original providers; produces PRE-025. |
| M16 | M25 PRE-034 and M15 PRE-025; inspect local peer/gameplay evidence | Waits for both; produces PRE-026. |
| M17 | M15 PRE-025, M16 PRE-026, PRE-017 clean runners and Arch evidence; inspect all preset/installed-prefix logs | Waits for integrated release and external runner acceptance; produces PRE-027. |
| M18 | M17 PRE-027 and optional PRE-015; inspect fixture provenance | Optional, not a Linux release prerequisite. |

## 11. Clean-environment simulation

**Pass with declared later external dependencies.** Conceptual x86-64 checkout starts with only documented `docs/building-linux.md` distribution packages, tracked source, four presets, no cache/global bgfx install, no retail files or active GPU/session. Historical completed providers are committed. M29 can inspect source, implement PRE-038 and establish PRE-039 through pinned/reviewed source and offline shader generation; its ordinary acceptance needs no physical GPU or retail assets. M30 consumes these committed outputs, implements PRE-040 with no configure-time download, runs asset-free builds first, then requires the declared PRE-012/016 hardware session to close its GPU acceptance. M22 consumes accepted M21 and M30 and adds read-only retail scenes, producing the original draw path required by M24. M23 and M24 then reach independent original interaction/persistence gates, followed by M25/M15/M16. M17 receives declared clean runners; optional M18 never feeds Linux acceptance. No milestone depends on its own unfinished output at entry, and no future milestone provides an earlier provider. Host-specific and future test outcomes were simulated, not run.

## 12. Commands run and results

| Command/inspection | Observed result |
|---|---|
| `git rev-parse HEAD`, `git status --short --untracked-files=all` | HEAD above; parent/compiler dirty paths only at invocation; no child commit. |
| `rg --files -g AGENTS.md` | No applicable AGENTS.md found. |
| `command -v cmake ninja gcc g++ clang clang++ pkg-config glslc vulkaninfo` | All listed host executables resolve; no GPU access inferred. |
| `pkg-config --modversion sdl3 freetype2 fontconfig zlib libavformat libavcodec libavutil libswscale libswresample` | All modules resolve; SDL3 3.4.14 observed. |
| `cmake --list-presets` | Four native GCC/Clang Debug/Release presets listed. |
| `rg --files tools/renderer third_party` and contract/authority inspection | Public probe source and pinned miniaudio present; no repository bgfx/bx/bimg/toolchain integration yet. |
| `ruby -ryaml` structural check | Not run: `ruby` is not installed; no dependency was added for this audit. |
| `python3 -c` YAML graph check | 31 unique milestones; all declared dependencies precede consumers; M29 is first remaining. |
| `python3 -c` manifest row check and shell heading loop | 40 sequential unique PRE rows; all 31 contracts have both readiness headings. |
| Packet graph/readiness/manifest structural inspection | 31 stable contracts, backward-only dependency edges, PRE-001–040, both readiness headings in every contract. |
| `git diff --check` | Passed after readiness edits. |

No full build, retail traversal, GPU invocation, package installation, network fetch or milestone acceptance ran in this documentation audit. Historical evidence remains historical.

## 13. Files changed

- `delivery/milestones/cnc-generals-zero-hour/M29-renderer-contract-migration.md` — PRE-038/039 and concrete first-gate readiness checks.
- `delivery/milestones/cnc-generals-zero-hour/M30-bgfx-device-validation.md` — PRE-040 and explicit M30 hardware gate/checks.
- `delivery/milestones/cnc-generals-zero-hour/M22-original-rendering.md` — explicit PRE-040 consumption.
- `delivery/milestones/cnc-generals-zero-hour/M24-original-persistence.md` — explicit PRE-031 draw-provider consumption.
- `delivery/readiness/cnc-generals-zero-hour/prerequisite-manifest.md` — current provenance, classifications, consumers and PRE-038–040.
- `delivery/readiness/cnc-generals-zero-hour/readiness-report.md` — complete re-audit and clean-checkout simulation.

Compiler's other packet changes and parent workflow state/handoff remain parent-owned. This child made no implementation edits or commit.

## 14. Remaining blockers

None at M29 entry. PRE-012/016 are declared later M30 hardware-acceptance user-action gates, PRE-008 is a later M22 retail-validation gate, PRE-017 gates M17 and PRE-015 only optional M18. M22 and M24 remain blocked in delivery until their provider acceptances. Parent planning commit/provenance is required before this packet is consumable.

## 15. Exact recommended next step

Parent Delivery Planning validates and commits the combined packet/readiness transaction, finalizes provenance/handoff, and permits Delivery to create M29's governing implementation plan in `/home/ha/projects/CnC_Generals_Zero_Hour`; implementation does not start during this planning transaction.
