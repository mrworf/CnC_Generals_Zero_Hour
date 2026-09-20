# M2 slice 02: engine renderer contract

## Goal and observable outcome

Engine code can describe resources, passes, bindings, uploads, and draws through a backend-neutral interface whose resource limits, immutable pipeline identity, conventions, and resize lifecycle are validated without a GPU.

## Scope

- Define opaque handles and fixed-width buffer, texture, sampler, shader, pipeline, render-target, binding, upload, and draw descriptors.
- Define an internal device interface shared later by the recording and SDL_GPU implementations.
- Cap each shader stage at four uniform buffers and validate binding slots, ranges, texture dimensions/formats, primitive topology, point-size requirements, and pass lifecycle.
- Define immutable pipeline-key comparison/hash composition over every pipeline-affecting field.
- Lock handedness, 0-1 depth, winding/culling, UI half-pixel placement, color packing, texture origin, depth bias, fog distance, and premultiplied-alpha conventions.
- Model idle/requested/recreating resize transitions with zero-extent suspension and monotonic generations.

## Non-scope

No real or recording backend implementation, SDL window/device, rasterized pixels, resource allocation, or legacy call-site migration.

## Dependencies and ordering

Slice 01 and the M1 fixed-width foundation. Slice 03 consumes this contract.

## Entry point and end-to-end behavior

A contract test constructs valid descriptors/pipeline keys, validates a complete render-pass/draw description, and exercises resize request/recreation. Invalid limits, stale handles, incomplete pipelines, point-list omissions, invalid upload ranges, and illegal resize transitions return stable diagnostics before backend use.

## Data/state transitions

Renderer descriptions are immutable values. Resize transitions move `stable -> requested -> recreating -> stable` or `suspended`, incrementing the generation only after successful nonzero recreation.

## Authorization and permissions

Not applicable: pure in-process, asset-free APIs with no external access.

## Validation and error handling

- Positive: valid resources, four uniform bindings per stage, deterministic key equality/hash, conventions, and resize lifecycle.
- Negative: fifth uniform, invalid format/dimension, zero buffer, upload overflow, missing point size, bad pass descriptor, and out-of-order resize completion produce named errors.
- Public-header scan rejects D3D, SDL, and Vulkan identifiers/includes.

## Expected implementation surfaces

`include/zh/renderer/*`, `src/renderer/*`, `tests/renderer/test_renderer_contract.cpp`, and explicit CMake target source/test wiring.

## Required commands

- build `renderer_contract_tests` in GCC and Clang debug presets.
- run `ctest --preset <preset> -L renderer-contract --output-on-failure` for the affected presets.

## Acceptance criteria

- Public contract has no backend-shaped dependencies.
- Four-uniform-per-stage and all other declared limits are enforced.
- Pipeline keys include all documented immutable state and are stable hash keys.
- Convention and resize contracts have positive and negative tests.

## Commit boundary

Commit the renderer API, validation/state implementation, tests, CMake wiring, this slice plan, and governing-plan status update as `delivery: M2 slice 02 add renderer contract`.
