# M29 slice 01 — ordered camera-clear recording

## Goal and observable outcome

An original-shaped producer can begin a full-target clear pass, draw, set a camera viewport, clear only its clipped rectangle with independently selected color/depth/stencil values, draw again, and end the pass. Snapshot order and selected values are deterministic.

## Scope and non-scope

Extend backend-neutral descriptors and `GpuDevice`; implement fail-closed recording and tests. Do not implement bgfx submission, retail scene wiring or modify SDL_GPU to pretend it supports the operation.

## Dependencies and ordering

Uses completed M7/M22 pass/handle semantics. First M29 slice; slice 02 may connect source-facing entry points and slice 03 packages shaders.

## Entry point and behavior

`GpuDevice::clear_viewport` receives target rectangle, target generation and independently flagged attachment values during an active pass. The recorder validates the exact live attached targets, format/flags/values, clipped nonempty extent, initialized loads and bounded view order, then appends one ordered clear command. Rejection appends an error only; no clear command is submitted. Full-target `begin_pass` behavior remains unchanged. A fresh pass/full target clear establishes initialization. Target destruction or recreation changes identity/generation.

## Data/state, authorization and errors

The pass retains attachment handles and a target generation token. Clearing does not change resource identity, pass state or viewport; view order increments only on accepted commands. This is local renderer input, not user authorization. Invalid timing, dimensions, flags, target generation, missing stencil format, nonfinite/out-of-range values, uninitialized load, destroyed targets and exhausted order return actionable failures and leave command order intact.

## Surfaces

`include/zh/renderer/contract.h`, `include/zh/renderer/recording_device.h`, `src/renderer/{contract,recording_device}.cpp`, `tests/renderer/{test_renderer_contract,test_recording_device}.cpp`, renderer contract documentation.

## Tests and validation

- Positive: interleaved draw/clear order, clipped rectangle, independent flags/values, unchanged pass default and deterministic snapshot.
- Negative: outside pass, empty/outside bounds, no flags, bad/nonfinite values, stale generation/target, unsupported stencil format, uninitialized load, budget exhaustion; no clear command appears on rejection.
- Configure/build `linux-gcc-debug`; run `renderer_contract`, `renderer_recording_device`, and public-header check; inspect snapshot diff.

## Acceptance and commit

Meets M29 ordered-clear and recorder acceptance clauses with no physical submission claim. One reviewable commit contains this plan, implementation, tests and focused evidence; no other milestone paths.

## Delivered evidence

[Focused test and boundary evidence](../../evidence/cnc-generals-zero-hour/milestone_29_slice_01.md). Commit identity is the commit containing this slice file; the transaction handoff records its exact SHA without a self-referential Git field.
