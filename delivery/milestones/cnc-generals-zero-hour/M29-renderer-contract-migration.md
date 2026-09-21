# Milestone M29: ordered-clear renderer contract and shader closure

## Objective

Make the existing backend-neutral renderer boundary capable of expressing the original W3D camera-scoped clear and every required shader/format family before changing device ownership.

## User/System Outcome

An original scene producer can record a full-target clear, interleaved draws, and an independently flagged color/depth/stencil viewport clear in exact order. The repository can reproducibly build the required shader families for the selected bgfx backend without a graphics device.

## Scope

- Add a recorder-visible ordered viewport-clear operation, preserving existing full-target pass clear/load and opaque resource handles.
- Recheck the M2 legacy API inventory, M7 recorder/state contract, and M8–M10 UI/world/effects command and shader families against the selected public backend. Correct affected descriptors and producers without restarting accepted CPU/source providers.
- Package repository-owned shader families in an offline, pinned bgfx-compatible build path; retain or explicitly validate equivalent uniform, texture, point-size, alpha, compressed-format, and state semantics.
- Update the source dependency ledger for the changed command and shader boundary.

## Explicit Exclusions

bgfx device ownership and GPU pixels belong to M30; complete retail scene acceptance belongs to M22. Do not rewrite gameplay, SDL3 platform/input, completed milestone histories, or user-owned retail content. No ARM64, raw Vulkan ownership, private Vulkan under SDL_GPU, or synthetic acceptance in place of original scenes.

## Source Requirements

- `REC-RM-001` — resolved renderer device-edge remediation in `workflow/reconciliation/renderer-bgfx-2026-09-21/03-remediation-backlog.md`.
- `RB-01` and `RB-03` — accepted [renderer backend migration](../../../docs/zero-hour-renderer-backend-migration.md), governed by [port plan §9](../../../docs/zero-hour-linux-port-plan.md).
- `REC-001` and `M22_VIEWPORT_CLEAR_BACKEND` are evidence provenance, not independent requirements. Exact source revisions and reconciliation IDs are in `status.yaml`.

## Preconditions

M8, M9 and M10 provide the accepted command and shader families; M7 and M2 are their completed transitive contracts. Their recorded component evidence is a preservation baseline, not acceptance of the new backend. No hardware, retail asset, or M21 simulation is required to implement this contract.

- `PRE-020`, `PRE-021`, `PRE-022` — completed M8 UI, M9 world and M10 effects command/shader families; retain their acceptance evidence and M7/M2 transitive contracts. M29 produces `PRE-038` ordered-clear contract and `PRE-039` pinned offline bgfx-compatible shader inputs/toolchain; neither is an entry prerequisite.
- `PRE-002` — the documented x86-64 compiler/CMake/Ninja and current offline shader baseline are available. bgfx/bx/bimg and its shader compiler are not currently vendored or installed as repository dependencies; revision/license review, reproducible source acquisition and offline build integration are M29 work, not assumed global packages.

## Readiness checks

- From the repository root, run `cmake --list-presets` and require all four native presets. Inspect `status.yaml` and M8/M9/M10 evidence for completed provider contracts; historical SDL_GPU results are preservation baselines only.
- Inspect `docs/zero-hour-renderer-backend-migration.md` RB-01/RB-03 and `tools/renderer/bgfx_viewport_clear_probe.cpp` without executing a GPU test; require the public-probe evidence and source-controlled shader inventory to be present. `third_party/` currently lacks bgfx; M29 must produce its pin/license/offline-toolchain record before acceptance.

## Functional Requirements

- Represent a clipped target rectangle, independent color/depth/stencil enable flags and values, and target generation in the command stream. Preserve source order relative to passes, draws and later clears; never substitute a proxy draw for a clear.
- Reject illegal timing, empty/invalid dimensions, unsupported attachment formats, stale or destroyed targets, generation mismatch, uninitialized load and view-order exhaustion before device submission. Preserve existing full-target clear/load defaults and failure behavior.
- Preserve engine-owned handle identity and backend-neutral public headers. Revised recorder snapshots remain deterministic and distinguish clear scope, selected attachment values and command order.
- Reconcile every required `docs/renderer/legacy-api-mapping.tsv` category to public bgfx support, retained CPU fallback, or explicit fail-closed rejection; unknown required source categories must fail.
- Generate all required shader binaries offline and reproducibly. Preserve the four-uniform-block contract or prove a semantically equivalent binding, point size, premultiplied alpha, BC fallbacks and M8–M10 material/effect semantics.

## Architecture / Security Constraints

The accepted §9 second branch changes only the renderer device edge; SDL3 remains platform/input. No public engine header may expose bgfx, SDL_GPU or Vulkan types. Keep retail roots read-only and do not commit retail bytes, hashes or private paths. Pin and review the bgfx and shader-tool revisions/license before use.

## Interfaces and Compatibility

Extend `GpuDevice` and recording descriptors narrowly; preserve existing handle meanings, full-target pass behavior, `Dx8StateCache` source semantics, SDL3 events/input, native CPU providers and save/network formats. M30 must be able to consume this contract without a second command vocabulary. Existing SDL_GPU-specific tests remain historical evidence, not new backend acceptance. Rollback must not alter saves, configuration or retail roots.

## Acceptance Criteria

- [ ] An original-producer-shaped command sequence records full-target and camera-subrect color/depth/stencil clears with interleaved draws in exact order; outside-rectangle and unselected-attachment state remain logically preserved.
- [ ] Positive and negative recorder tests cover timing, dimensions, flags/values, format, lifetime/generation, load initialization and bounded view order; invalid input never reaches submission.
- [ ] The complete required shader family builds offline from pinned inputs in all four GCC/Clang Debug/Release presets, with repeatable outputs and exercised state/uniform/format mappings.
- [ ] M2 and M7–M10 affected contract/producer tests pass; unaffected accepted behavior remains unchanged and no required source category is silently dropped.

## Required Validation

Run inventory and shader checks, positive/negative recorder and producer tests, all four preset builds and focused suites, and a full asset-free regression at milestone acceptance. Verify source/ledger freshness and no platform graphics type in public engine headers. Record which historical M2/M7–M10 assertions were rerun, replaced or retained, with their evidence grade. Hardware acceptance is M30-owned.

## Known Risks / Deferred Work

The public bgfx view-clear probe proves the selected abstraction on the host but not all original scene categories. M30 must prove actual backend submission, shader/pipeline operation, device failure and hardware pixels; M22 must still prove retail scenes. If another required source behavior exceeds bgfx's public API, reopen §9 with a named reproducer rather than silently narrowing coverage.
