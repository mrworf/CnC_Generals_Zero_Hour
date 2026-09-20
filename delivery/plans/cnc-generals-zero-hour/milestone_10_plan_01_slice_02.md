# M10 slice 02: deterministic effects command recorder

## Goal and observable outcome

Record valid, deterministic device command sequences for every checked effects mapping, including explicit offscreen dependencies, point size, depth bias, premultiplied blending, and ordered WWShade multipasses, with bounded resources and complete teardown.

## Scope

- Add effect request/session validation and an effects recorder behind `RecordingGpuDevice`.
- Record source, main-effects, and post-effect passes in dependency order.
- Record every manifest effect, including multipass draws and exact state markers.
- Prove deterministic ordering independent of caller order, lifecycle recovery, render-target dependencies, and pipeline-capacity failure cleanup.
- Exercise the complete effects manifest through repository-owned synthetic requests.

## Non-scope

No SDL_GPU implementation, window, real GPU, screenshots, or pixel comparison. No retail asset bytes are required.

## Dependencies and ordering

Depends on slice 01's canonical effect mapping and M7's normalized recording device.

## Entry point and end-to-end behavior

`EffectsRecorder::load` validates and sorts requests, resolves each effect, and creates bounded resources/pipelines. `record_frame` uploads deterministic state, records offscreen inputs before consumers, records main effects and ordered multipasses, then records post processing after its input. `teardown` destroys every owned resource and enables a clean reload.

## Data and state transitions

`empty -> loaded -> frame-recorded -> loaded -> empty`. Double load, frame-before-load, regressed ticks, and double teardown fail without corrupting the valid state. Failed load cleans all partial resources.

## Authorization and permissions

Not applicable: all requests and fixtures are synthetic and repository-owned; the recorder performs no filesystem, device, network, or privileged operation.

## Validation, errors, and recovery

- Empty labels/materials, unknown effects, zero/excessive geometry, and duplicate logical requests fail with request context.
- Unknown required effects name the logical asset and material.
- Pipeline capacity is bounded and failed construction releases partial resources.
- A valid load/frame/teardown cycle can follow rejected calls.

## Expected implementation surfaces

`include/zh/effects/recorder.h`, `src/effects/recorder.cpp`, `tests/effects/test_effect_recorder.cpp`, and explicit CMake source/test linkage.

## Positive tests

- Every canonical effect emits a marker and draw command.
- Offscreen source precedes water/stealth; main effects precede post processing.
- WWShade pass 1 precedes pass 2.
- Point size, depth bias, premultiplied alpha, and render-target dependencies appear in normalized evidence.
- Caller ordering does not alter the normalized stream; teardown leaves zero resources.

## Negative tests

- Unknown effect includes logical asset/material.
- Frame/lifecycle misuse and tick regression fail, then recover where applicable.
- Insufficient pipeline capacity fails cleanly with no leaked resources.

## Required commands

```sh
cmake --preset linux-gcc-debug
cmake --build --preset linux-gcc-debug
ctest --preset linux-gcc-debug -R effects_recorder --output-on-failure
ctest --preset linux-gcc-debug -L renderer-contract --output-on-failure
```

Then run the governing plan's four-preset milestone validation.

## Acceptance criteria

All scoped effects record valid, ordered commands; diagnostics and lifecycle bounds pass; four-preset renderer-contract validation passes.

## Commit boundary

Commit recorder API/implementation, focused tests, CMake wiring, and final plan evidence together as `delivery: M10 slice 02 record effects`.
