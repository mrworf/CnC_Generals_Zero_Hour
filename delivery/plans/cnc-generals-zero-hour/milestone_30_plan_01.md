# M30: public bgfx device and renderer revalidation

This plan governs M30 only. It is a new milestone transaction; completed SDL_GPU milestone history remains intact.

## Outcome

The engine-facing `GpuDevice` submits resources, ordered camera-scoped clears, shader draws and presentation through public bgfx/Vulkan on the host RTX, with explicit failure/lifecycle behavior and renewed M7–M10/M14 evidence. M22 retains actual original retail scene acceptance.

## Delivery-goal context

- Goal status: `workflow/delivery/state.yaml`; product ID: `cnc-generals-zero-hour`.
- Packet: `workflow/delivery-planning/handoff.yaml`, revision `sha256:0aafbac99ec645407cc89f3b80496e876c6efb5f4eae5d699ba9837dac90fd89`.
- Planning payload: `c140a0e`; reconciliation payload: `25ef605`; M30 entered in progress at `4918304`.
- Fixed goal context: M29 → M30 → M22 → M24. Only M30 is in this transaction.

## Governing contracts

- `delivery/milestones/cnc-generals-zero-hour/M30-bgfx-device-validation.md` acceptance clauses 1–5 and required validation.
- `docs/zero-hour-renderer-backend-migration.md` RB-02/RB-04 and `docs/zero-hour-linux-port-plan.md` §9.
- `docs/renderer/renderer-contract.md`, `docs/renderer/bgfx-migration-ledger.md`, M29 accepted plan/evidence.
- No `AGENTS.md` was found. Build/test rules are CMake presets, CTest and milestone-specific GPU/sanitizer gates.

## Current-state findings

`GpuDevice` and M29's ordered `ViewportClearDesc` are backend-neutral. `RecordingGpuDevice` already validates ordering, generations and load. `SdlGpuDevice` still owns physical resources, shaders, draws and window presentation; its GPU tests are historical SDL_GPU evidence. M29 pins bgfx/bx/bimg and an offline patched shaderc, produces 39 bgfx Vulkan shader containers, but does not build/link bgfx runtime. The ignored cache has pinned source and shaderc; this is not a system dependency. Transaction-start HEAD is `491830444463a61ba824e7194675ba7d838a4b43`; worktree clean.

## Decisions

- Build a pinned public bgfx runtime from the existing reviewed source cache, with an explicit configure error if the cache is absent/wrong. Never silently fetch or use a system unpinned build.
- Preserve opaque engine handle generations and SDL3 window/input ownership. Shader manifests remain the build-time descriptor authority.
- Use bounded, ordered public bgfx views; reject unsupported required state before submission. No proxy clear or private Vulkan calls.
- Retain old SDL_GPU implementation/tests as historical diagnostics until equivalent bgfx acceptance is recorded; M30 cannot claim them as new-backend evidence.

## Scope

Included: bgfx runtime integration, physical resource and target lifetime, ordered clear, pipeline/draw/shader/format behavior, SDL3-window presentation, host validation and source dependency ledger.

Excluded: original WW3D retail scene/pixel acceptance (M22), gameplay/save/network, platform input rewrite, ARM64, raw/private Vulkan, retail redistribution.

## Slice index

| Slice | Plan | Outcome | Dependencies | Status | Commit | Evidence |
|---|---|---|---|---|---|---|
| 01 | [runtime/resources](milestone_30_plan_01_slice_01.md) | Pinned bgfx runtime and generation-safe resource/lifecycle path | M29 | completed | recorded in next index update | [focused evidence](../../evidence/cnc-generals-zero-hour/milestone_30_slice_01.md) |
| 02 | [ordered targets](milestone_30_plan_01_slice_02.md) | Real target/pass/viewport clears in command order | 01 | pending | | |
| 03 | [shader draws](milestone_30_plan_01_slice_03.md) | Physical shader, pipeline, bindings and indexed draw behavior | 02 | pending | | |
| 04 | [presentation and revalidation](milestone_30_plan_01_slice_04.md) | SDL3-window present, lifecycle and full host evidence | 03 | pending | | |

Four slices are necessary because each has a distinct independently observable GPU boundary and failure mode; combining all into one commit would hide the clear-vs-draw dependency and hardware acceptance results.

## Cross-slice concerns

- Compatibility: public `GpuDevice` types remain opaque/backend-neutral; prior SDL_GPU evidence is historical, not new acceptance.
- Authorization/security: no retail-root writes or private data in evidence; public pinned BSD-2-Clause dependency only.
- Lifecycle: stale handles/target generations, zero-extent and re-entry fail explicitly; no silent draw loss.
- Observability: capture public API return/error and Vulkan validation output; compare evidence grades with M14 history.
- Environment: asset-free builds are offline; RTX/Vulkan and validation layer are required only at GPU acceptance.

## Milestone completion gate

All four GCC/Clang Debug/Release builds; affected renderer, shader, UI/world/effects and full asset-free CTest; relevant ASan/UBSan; checked 160×120 public probe; physical source-shaped clear/draw trace and RTX validation-layer GPU suite; format/alpha/depth/stencil, resize/loss/re-entry; exact dependency/toolchain revision and M2/M7–M10/M14 evidence ledger. Inspect worktree/diff and report M22 handoff without claiming retail pixels.

## Rollback and recovery

Slice commits isolate physical migration. Reverting the device/build commits restores historical SDL_GPU behavior without changing user state or retail content; prior M29 contract may remain independently valid.

## Execution notes

Planning phase completed before production edits. Update this index and slice plans with findings and commits. If public bgfx lacks another required behavior, preserve a minimal repro and stop for §9 architecture decision; do not add a private escape.

Slice 01 delivers offline runtime pin/check, physically allocated RGBA8 texture uploads and two-cycle Vulkan lifecycle. Buffers/samplers remain validated descriptors until layout and binding information exists at slice 03; no renderer pixels or ordered clears are accepted yet. The physical resource test passed only with host GPU access (sandbox hid NVIDIA ICD). The exact slice 01 commit is written into this index with the next slice update, avoiding a self-referential commit hash.

## Deferred follow-ups

Actual original-source scene families and retail pixels remain M22; M24 depends on that provider.
