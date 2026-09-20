# M9 slice 02: deterministic recorder-backed world commands

## Goal and observable outcome

A validated representative world scene loads and records render-target setup, uploads, pipeline/resource creation, and ordered draws for meshes, animation-visible geometry, terrain, roads/decals, trees, shadows, all factions, and selection markers. Re-recording an equivalent scene produces the same normalized command stream.

## Scope

- `WorldRecorder` map-load, frame-record, and teardown entry points.
- Bounded vertex/index/uniform streaming and map-owned textures, samplers, shaders, pipelines, and render targets.
- Camera, render-target dependency, material-state, family/faction, animation, and selection markers in the normalized stream.
- Deterministic ordering independent of caller item order.
- Positive representative-map and negative resource/state tests.

## Non-scope

No effects, water, WWShade, real GPU, rasterized pixels, or retail asset decoding.

## Dependencies and ordering

Depends on slice 01's validated scene/translation contract and M7's recorder. Must precede lifecycle/corpus closure in slice 03.

## Entry point and behavior

`WorldRecorder::load(scene)` validates and allocates one bounded map resource set. `record_frame(tick)` uploads deterministic streams/uniforms, begins dependency passes in fixed order, emits family-sorted draws, and ends each pass. Invalid lifecycle calls and device failures return contextual diagnostics. `teardown()` destroys all owned resources in dependency-safe order.

## Data/state transitions

`empty -> loaded -> frame(s) -> empty`. Loading while loaded, drawing while empty, a tick regression, or teardown during a device pass fails without silently mutating lifecycle state.

## Authorization and permissions

Not applicable: synthetic descriptions and the in-memory recorder require no files, devices, or privileges.

## Validation and recovery

Tests compare two normalized snapshots, verify fixed pass/draw ordering and all family markers, exercise multiple frames and animations, reject invalid lifecycle order, and confirm teardown permits a clean reload.

## Implementation surfaces

- `include/zh/world/recorder.h`
- `src/world/recorder.cpp`
- `tests/world/test_world_recorder.cpp`
- `CMakeLists.txt`
- governing plan completion record

## Required commands

```sh
cmake --build --preset linux-gcc-debug --target world_recorder_tests
ctest --preset linux-gcc-debug -R '^world_recorder$' --output-on-failure
```

## Acceptance criteria

- Every M9 family and render-target dependency emits recorder commands.
- Equivalent scenes yield byte-identical snapshots with stable ordering.
- Stream uploads reuse bounded buffers and lifecycle errors are actionable.

## Commit boundary

Commit the recorder implementation, registration, tests, and plan record as `delivery: M9 slice 02 record deterministic world commands`.
