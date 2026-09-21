# Milestone M30: public bgfx device and renderer revalidation

## Objective

Replace only the SDL_GPU device/submission implementation with a public bgfx Vulkan backend, then revalidate the affected renderer contracts and real-GPU behavior before original retail scene completion resumes.

## User/System Outcome

The source-owned camera clear occurs in its original active-frame order on the RTX Vulkan host, while already accepted UI/world/effects, resource and presentation behaviors still work through the new device edge.

## Scope

- Implement bgfx-backed `GpuDevice` resource, shader/pipeline, target, ordered view/clear/draw, resize/present, failure and cleanup behavior using M29's command contract.
- Integrate pinned/reproducible public bgfx and offline shader binaries into the four native presets and installed-resource path without retail redistribution.
- Rerun affected M7–M10 and M14 tests and real RTX/Vulkan visual, lifecycle and validation gates; retain a source dependency ledger and evidence-grade comparison to historical SDL_GPU results.
- Exercise the exact public 160×120 color/depth/stencil probe and an owned command trace matching the original W3D camera-clear/draw order, then hand the working backend to M22 for the actual original-source call path and full retail scene acceptance.

## Explicit Exclusions

Do not restart completed CPU/source providers, alter SDL3 platform/input, perform a gameplay rewrite, or claim M22 retail-scene acceptance from a synthetic probe. No ARM64, second GPU requirement, raw Vulkan ownership, private Vulkan escape under SDL_GPU, or retail content in the repository.

## Source Requirements

- `REC-RM-001` — resolved renderer migration remediation in `workflow/reconciliation/renderer-bgfx-2026-09-21/03-remediation-backlog.md`.
- `RB-02` and `RB-04` — accepted [renderer backend migration](../../../docs/zero-hour-renderer-backend-migration.md), governed by [port plan §9](../../../docs/zero-hour-linux-port-plan.md).
- `REC-001` and `M22_VIEWPORT_CLEAR_BACKEND` are evidence provenance only. Revisions and canonical handoff are recorded in `status.yaml`.

## Preconditions

M29 supplies the ordered-clear, shader and mapping contract. M14 supplies historical GPU component behavior for comparison, not acceptance of bgfx. The host Vulkan driver and validation layer are required at the named hardware acceptance gate; ordinary builds/tests remain device and retail independent. M21 is not required for backend implementation, but its source-owned scene sequence is used for M22 handoff validation.

- `PRE-038`/`PRE-039` — M29's accepted ordered-clear descriptor and pinned offline shader/toolchain path. `PRE-013` — completed M14 component hardware record for comparison, not bgfx acceptance. M30 produces `PRE-040` public-bgfx device and renewed hardware evidence.
- `PRE-012`/`PRE-016` — developer/session owner supplies the RTX Vulkan driver, Khronos validation layer and graphical session for M30's hardware acceptance. These are not required for M29 or M30's asset-free implementation checks. Runtime bgfx source pin/license and offline build integration are M30-owned, using M29's pinned toolchain record; no preinstalled bgfx package is presumed.

## Readiness checks

- From the repository root, run `cmake --list-presets` and require all four native presets. Inspect `status.yaml` and M29 acceptance for `PRE-038`/`PRE-039`, and M14 evidence for the historical comparison; do not count M14 as new-backend acceptance.
- At the hardware gate, run `vulkaninfo --summary` in the developer graphical session and inspect its device/layer output for the RTX and `VK_LAYER_KHRONOS_validation`; then run the M30-owned probe and GPU tests with that layer explicitly enabled. The probe and tests are acceptance outputs, not entry checks.

## Functional Requirements

- Map live textures, buffers, indexed/base-vertex draws, render targets, shaders/pipelines, viewport/scissor state and ordered scene/view commands through public bgfx. Preserve opaque engine handles, target generations and independent attachment load/clear semantics.
- Bound view IDs and resource/state caches. Detect exhaustion, illegal timing, stale targets, invalid formats and unsupported required states before submission; never reorder/drop draws or substitute a fake/proxy clear.
- Preserve resize, zero-extent suspension, present, loss/failure propagation, target recreation and complete shutdown/re-entry without resource leaks or writes to retail roots.
- Keep SDL3 window/event/input ownership. Preserve M8–M10 UI/world/effects and M14 visual/format/alpha/depth/stencil behavior, including required CPU format fallbacks, point size and uniform bindings.
- Ensure a hardware-exercised command trace preserves the source-established `WW3D::Begin_Render` → camera viewport/`DX8Wrapper::Clear` → scene draws → later scenes → `End_Render` order. M22 owns connection of the actual original call path and full scene-family/retail pixel acceptance.

## Architecture / Security Constraints

Use the selected §9 public-bgfx branch only; no private Vulkan calls or raw Vulkan ownership. Pin and review dependency license/version and offline shader compiler; use repository policy for generated or vendored artifacts. Public engine headers remain backend-neutral. Original retail roots are read-only; no private bytes, hashes or paths in committed evidence.

## Interfaces and Compatibility

`GpuDevice` remains the engine-facing seam; SDL3 platform/input, original W3D/WWShade/GameClient producers, engine-owned handles, save/network/data formats and XDG paths remain unchanged. Replace SDL_GPU-specific implementation and tests only where necessary, retaining historical evidence and equivalent new backend coverage. Rollback is code/build-level and must not touch user state or retail content.

## Acceptance Criteria

- [ ] The exact 160×120 ordered outer/inset probe passes on the RTX Vulkan host: outer color/depth/stencil survive, selected inset attachments change, and subsequent independent depth/stencil-tested draws expose any preservation error.
- [ ] The source-shaped camera-clear/interleaved-draw trace executes through the public bgfx edge without proxy work, silent drops or relevant Vulkan validation errors; the original-source call path remains M22 acceptance.
- [ ] Invalid state, generation, format, view budget, resize/loss and cleanup paths fail explicitly or recover cleanly; resource and worker counts return to baseline after repeat lifecycle tests.
- [ ] M7–M10 affected contracts/producers and M14 hardware/visual/validation acceptance are rerun with documented evidence grades; all four native presets and full asset-free suites pass, while SDL3 input/platform behavior remains intact.
- [ ] The device handoff is sufficient for M22 to resume its existing plan at 06C3C; no full original retail-scene acceptance is claimed here.

## Required Validation

Run all four GCC/Clang Debug/Release builds, affected recorder/shader/producer suites, full asset-free CTest, relevant ASan/UBSan, explicit Khronos validation-layer RTX/Vulkan tests, pixel/alpha/depth/stencil and resize/loss/re-entry tests. Reproduce the checked public bgfx probe and source-shaped camera-clear order; inspect validation output for errors/VUIDs. Record exact toolchain/library revisions and a historical-to-new M2/M7–M10/M14 evidence ledger. Keep private retail tests opt-in and leave M22's original-source/full scene-family acceptance pending.

## Known Risks / Deferred Work

bgfx coverage beyond the proven camera-clear case remains an implementation risk. A newly demonstrated public-API gap follows port plan §9 evidence-gated escalation; it is not permission for a private escape. M22 and then M24 remain downstream, and M23/M15–M17 retain their existing integration gates.
