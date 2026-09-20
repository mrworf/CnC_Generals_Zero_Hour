# M7 slice 03: DX8 state cache and resize command generation

## Goal and observable outcome

Legacy renderer call sites can use a state-cached DX8-style facade to emit minimal, ordered recorder commands and explicit resize/recreation transitions without accessing SDL_GPU or Vulkan.

## Scope

Implement cached pipeline/buffer/binding/pass state, draw submission, redundant-state suppression, and resize request/begin/complete command generation. Add stable snapshot tests for draw order, state-derived pipeline reuse, resize success/failure/suspension, missing state, and resource destruction.

## Non-scope

Porting actual legacy source call sites, texture-stage/effect translation owned by M8-M10, real window/swapchain recreation, hardware presentation, and pixels.

## Dependencies and ordering

Depends on slice 01 recorder behavior and existing `ResizeState`; follows slice 02 so final validation covers the complete M7 package.

## Entry point and behavior

Tests configure `Dx8StateCache`, bind resources/state, begin a target, draw, end, and inspect the recorder snapshot. The facade creates/reuses immutable pipelines only when dirty state changes. Resize methods mirror `stable -> requested -> recreating -> stable`, emit ordered logical commands, suspend zero extents, and retain pending requests after failed recreation.

## Data/state transitions

Bindings and pipeline description remain cached until changed; dirty pipeline state clears only after successful materialization. Pass-active state mirrors the recorder. Resize transitions are committed only when the underlying `ResizeState` accepts them.

Authorization is not applicable because the facade is an in-process compatibility boundary.

## Validation and recovery

Reject draw without pass, shaders, vertex buffer, or required bindings; propagate recorder errors unchanged with facade context. Tests cover state reuse, changed-state cache growth, cache bound propagation, stale resources, nested passes, resize ordering, failed recreation recovery, and zero-size suspension.

## Expected surfaces

`include/zh/renderer/dx8_state_cache.h`, `src/renderer/dx8_state_cache.cpp`, `tests/renderer/test_dx8_state_cache.cpp`, `docs/renderer/renderer-contract.md`, and `CMakeLists.txt`.

## Validation commands

Run the focused facade test, all four preset builds and `renderer-contract` labels, the canonical full suite in one debug preset, and a source scan confirming platform GPU API names do not appear in engine-facing renderer implementation/header files.

## Acceptance and commit boundary

Facade snapshots prove deterministic ordering, bounded caching, error behavior, and resize commands. Commit as `delivery: M7 slice 03 add DX8 state cache`.
