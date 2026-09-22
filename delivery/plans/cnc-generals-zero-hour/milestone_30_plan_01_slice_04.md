# M30 slice 04: SDL3 presentation and renewed milestone acceptance

## Goal and outcome

The bgfx device presents to an SDL3-owned window on RTX/Vulkan, handles resize/zero extent/loss/shutdown/re-entry cleanly, and passes the complete M30 acceptance matrix with a durable evidence ledger for M22.

## Scope / non-scope

Own SDL3 native-window handoff to public bgfx, presentation/readback, lifecycle recovery, affected integration-test migration and evidence ledger. Do not rewrite SDL3 input/event ownership or claim original retail scene pixels.

## Dependencies / order

Slices 01–03 physical resources, ordered targets and shaders/draws; M14 historical acceptance for comparison. Finish M30 only after this slice and cumulative gate pass.

## End-to-end behavior and state

SDL3 creates/owns window → bgfx receives public native-window platform data → renders ordered pass/draw → presents once per frame → resize reset/suspend/recreate honors target generations → loss reports explicit failure → shutdown/re-entry returns resource/worker counts to baseline. UI/world/effects continue through unchanged `GpuDevice` seam.

## Authorization / validation

No permission surface. Reject null/double window claim, invalid/zero-source present and stale target; suspend zero extent without corrupting state. Inspect validation-layer errors/VUIDs; never write retail roots or evidence with private bytes/paths.

## Implementation surfaces

Expected: bgfx device and SDL3 integration, CMake/test target changes, `tests/renderer/gpu_acceptance.cpp` equivalent backend coverage, `docs/renderer/bgfx-migration-ledger.md`, `delivery/evidence/cnc-generals-zero-hour/` result.

## Tests and commands

- Positive: visible/offscreen present, resize/re-entry, four preset builds, affected M7–M10/M14 tests, full asset-free CTest, ASan/UBSan, exact probe, source-shaped clear/draw trace with `VK_LAYER_KHRONOS_validation` on RTX.
- Negative: null/double window, zero extent, loss/failure and cleanup; inspect validation output for relevant errors.
- Record exact source, compiler, Vulkan/NVIDIA/bgfx/toolchain revisions and historical-to-new evidence grades. Private retail tests remain opt-in.

## Acceptance / commit boundary

One commit includes integration behavior, tests, evidence and this plan/index update after full required validation. If an external GPU gate is unavailable, leave this slice incomplete and record exact resume point; do not mark M30 accepted.
