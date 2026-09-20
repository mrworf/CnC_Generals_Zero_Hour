# M0: reproducible build readiness

## Outcome

An x86-64 contributor can configure, build, and test an asset-free bootstrap graph with GCC or Clang in Debug or Release mode. The checked build inventory accounts for the legacy Zero Hour manifests without claiming that the Win32-era game sources already link.

## Delivery-goal context

- Goal status record: `workflow/delivery/state.yaml`
- Product ID: `cnc-generals-zero-hour`
- Active packet and revision: `delivery/milestones/cnc-generals-zero-hour/status.yaml`, `sha256:7f486befe4e182f46fbc7f090d757ad7a19bcabddf53ce6b622b6ba90596de4d`
- Source transaction: `8dc0285ff6c3346273973dcd874e6983060bb3e7`
- Planning transaction: `2e7260543244ba11549d3728f42e51560c16ba6f` (closeout `11d39e4bc9e3ead8298746ef9ef0dd04f87f149f`)
- Fixed goal scope: M0-M18; this transaction implements M0 only
- Current milestone: M0
- Resume mode: new

## Governing contracts

- Product authority: `docs/zero-hour-linux-port-plan.md`, sections 3, 4, 7, 8/M0, and 10
- Milestone contract: `delivery/milestones/cnc-generals-zero-hour/M0-build-graph.md`
- Acceptance: PRE-003 and PRE-004, four named x86-64 presets, actionable dependency failures, labels, generated metadata, offline configure, and an asset-free smoke graph
- Repository instructions: no `AGENTS.md` is present; the milestone and product plan govern

## Current-state findings

- The repository contains the VC6 `RTS.dsw`, its main `RTS.dsp`, core engine/device/support manifests, obsolete service projects, tools, and several manifest paths whose third-party projects are absent.
- There is no native build system or test runner yet. The Arch host has CMake, Ninja, GCC, Clang, SDL3, glslc, FreeType, Fontconfig, zlib, and FFmpeg development metadata; Vulkan validation layers are deferred to M14.
- The legacy source cannot honestly be linked before later portability milestones. M0 therefore records its source closure and builds project-owned bootstrap components with the final target names.

## Decisions

- Check in explicit legacy source ownership/exclusion lists generated from all repository `.dsp` manifests, plus a deterministic audit tool that rejects unclassified, duplicate, missing, or stale records.
- Build small bootstrap implementations under the final target names. Store legacy candidates as non-compiling target metadata until their owning port milestone makes them buildable.
- Require the distribution dependency stack during normal configure and compile one project-owned bootstrap shader with `glslc`; no configure path fetches content.
- Use CTest foundation-labelled positive and negative bootstrap checks. Future labels are registered as the stable vocabulary without inventing empty passing tests.

## Scope

### Included

- PRE-003 legacy project/source inventory and audit.
- PRE-004 CMake graph, four presets, dependency/version and architecture probes, build metadata, shader build, smoke executable/tests, and build guide.

### Excluded and deferred

- Compiling or linking the legacy game implementation, runtime platform behavior, retail data, interactive devices, and release packaging.
- Installing the M14 Vulkan validation package or using the retail symlink.

## Slice index

| Slice | Plan | Outcome | Dependencies | Status | Commit | Evidence |
|---|---|---|---|---|---|---|
| 01 | [legacy manifest closure](milestone_00_plan_01_slice_01.md) | Every tracked legacy manifest source and project is explicitly classified and audit-checked | none | completed | `d28b296` | 1,889 candidates; 1,085 exclusions; 381 unavailable records; positive/negative audit tests pass |
| 02 | [offline bootstrap graph](milestone_00_plan_01_slice_02.md) | Four presets configure/build/test an asset-free dependency-backed target graph | 01 | completed | `445acb1` | Four preset builds and 24 aggregate CTest executions pass |

## Cross-slice concerns

- Compatibility and migration: checked legacy ownership is descriptive input for later ports; it does not change source behavior.
- Authorization and security: configure is local/offline and retail content is never inspected or copied.
- Invalidation and lifecycle effects: changes to a `.dsp` require updating the checked inventory before the audit passes.
- Audit and observability: configure prints architecture and dependency versions; the smoke process prints build metadata and modeled component names.
- Performance and scale: the inventory is data, not compiled source, so bootstrap builds remain small.
- Environment or external services: all dependencies are distribution-installed; no service or network is used.

## Milestone completion gate

- Run the inventory audit and its negative fixture.
- Configure, build, and run CTest for `linux-gcc-debug`, `linux-clang-debug`, `linux-gcc-release`, and `linux-clang-release`.
- Confirm the missing-program and unsupported-architecture probes fail with named, actionable diagnostics.
- Confirm `compile_commands.json`, generated build metadata, and bootstrap SPIR-V are produced without retail files.
- Review the build guide package names and tested Arch version snapshot.

## Rollback and recovery

Each slice is independently revertible. Removing slice 02 leaves the source inventory usable; reverting slice 01 also removes the inventory audit that slice 02 consumes.

## Execution notes

All slice plans were generated and inspected before production edits. The outer orchestrator owns milestone/status completion.

Slice 01 deduplicates repeated per-configuration `SOURCE=` declarations while retaining a unique classification for every manifest/source pair. Unavailable records include retail INI inputs named by `RTS.dsp` and absent third-party projects; they are documented inputs rather than bootstrap build requirements.

Slice 02 uses the repository manifests directly during its audit so exported source trees do not depend on Git metadata. The Arch dependency-negative configure was also exercised with an empty `pkg-config` directory and failed by naming `sdl3>=3.2`.

## Deferred follow-ups

- Replace each bootstrap component with its portable implementation in its owning milestone.
- Validate Ubuntu 26.04 and Fedora 44 clean environments at M17; M0 records their package contracts.
