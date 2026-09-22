# M29: ordered-clear renderer contract and shader closure

## Outcome

The backend-neutral command stream records original W3D camera-scoped color/depth/stencil clears in exact order, and every required repository shader family has a reproducible offline bgfx build route. No physical bgfx submission is claimed here.

## Delivery-goal context

- Goal status: `workflow/delivery/state.yaml`; product ID: `cnc-generals-zero-hour`.
- Active packet: `workflow/delivery-planning/handoff.yaml`, revision `sha256:0aafbac99ec645407cc89f3b80496e876c6efb5f4eae5d699ba9837dac90fd89`.
- Source reconciliation payload: `25ef605`; planning payload: `c140a0e`.
- Fixed goal from durable state: M26, M27, M28, M20, M21, M29, M30, M22, M23, M24, M25, M15, M16, M17, M18 (including completed historical members). This plan owns M29 only; new transaction at `bf6a4ebe438c83853f8b772d1d06e507bb565f53`.

## Governing contracts

- [M29 contract](../../milestones/cnc-generals-zero-hour/M29-renderer-contract-migration.md): ordered clear, inventory and shader closure, four presets, asset-free regression.
- [Renderer decision](../../../docs/zero-hour-renderer-backend-migration.md): RB-01 and RB-03, bounded by §9 of the port plan.
- [Renderer contract](../../../docs/renderer/renderer-contract.md) and [legacy inventory](../../../docs/renderer/legacy-api-mapping.tsv).
- No `AGENTS.md` exists in the repository. CMake presets and CTest are the native validation interface.

## Current-state findings

`GpuDevice` has full-target pass clear/load and ordered draw but no active-pass rectangle clear. `RecordingGpuDevice` validates opaque target generations and load initialization, while `OriginalGpuEdge` translates camera viewport but leaves source scene clear to M22. The existing glslc path emits SPIR-V for SDL_GPU. bgfx/bx/bimg are not installed as project dependencies; a public-source diagnostic checkout exists outside the repository and is not itself an accepted dependency. The worktree was clean at transaction start.

## Decisions

- A clear is a first-class ordered command scoped to the currently attached live target generation. It does not mutate the viewport or substitute a draw.
- A bounded view-order counter models bgfx's public ordered-view budget before physical submission; M30 consumes it.
- Keep historical SDL_GPU binaries/tests as preservation evidence; add bgfx packaging and semantic checks, rather than silently relabeling SPIR-V as bgfx format.
- Slice 03 pins bgfx/bx/bimg public source and license digests, stages supplied sources read-only into an ignored owned cache, and builds a narrowly patched upstream shaderc once. Configure/build/test consume that local tool fully offline; no global bgfx install or unreviewed binary is assumed.

## Scope

### Included

Ordered clear API, recorder and original-shaped producer test; affected inventory/CPU command-state reconciliation; pinned, licensed, offline bgfx shader inputs/build integration and semantic tests.

### Excluded and deferred

bgfx device implementation/pixels (M30), original retail scene completion (M22), SDL3 platform/input changes, private retail content, gameplay/save/network changes, ARM64 and raw Vulkan.

## Slice index

| Slice | Plan | Outcome | Dependencies | Status | Commit | Evidence |
|---|---|---|---|---|---|---|
| 01 | [ordered clear](milestone_29_plan_01_slice_01.md) | Original-shaped full pass, draw, camera clear, draw stream with fail-closed recorder validation | none | completed | `d79aed7` | [focused evidence](../../evidence/cnc-generals-zero-hour/milestone_29_slice_01.md) |
| 02 | [producer and inventory](milestone_29_plan_01_slice_02.md) | Affected source categories and CPU producers retain exact new command semantics | 01 | completed | `e3b4d7f` | [focused evidence](../../evidence/cnc-generals-zero-hour/milestone_29_slice_02.md) |
| 03 | [shader closure](milestone_29_plan_01_slice_03.md) | Pinned licensed offline bgfx shader route and complete family/state verification | 02 | completed | this slice commit | [focused evidence](../../evidence/cnc-generals-zero-hour/milestone_29_slice_03.md) |

## Cross-slice concerns

- Compatibility: preserve existing handles, pass defaults, snapshots and accepted CPU providers.
- Authorization/security: no retail-root mutation, no private paths, hashes or bytes in artifacts; public dependencies only.
- Lifecycle: stale/destroyed attachment and generation errors fail before submission; no implicit frame reorder.
- Observability: deterministic recorder snapshot and explicit validation errors.
- Scale: bounded view count and shader family manifest.
- Environment: GPU not required; M30 owns physical acceptance.

## Milestone completion gate

Run inventory, public-header and shader checks, relevant recorder/original/UI/world/effects tests, all four configure/build presets and focused CTest, then full asset-free CTest. Review dependency ledger, source freshness and evidence grade of historical M2/M7–M10 assertions. Do not call physical pixels accepted.

## Rollback and recovery

Each slice is separately revertible. No schema, save, retail-root or configuration migration. Preserve completed slice commits and resume at the incomplete slice if interrupted.

## Execution notes

Planning phase completed before production edits. Update this index with commit/evidence after each completed slice.

## Deferred follow-ups

M30 consumes the contract and bgfx artifacts; M22 connects full original scene and source clear call, followed by M24.
