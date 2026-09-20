# M9 slice 03: corpus closure and bounded lifecycle

## Goal and observable outcome

Project-owned representative map/faction cases cover every required world state, and repeated load/frame/teardown/reload cycles prove bounded pipeline/resources with zero live renderer handles after teardown. An unmapped required corpus state fails closed.

## Scope

- A logical, non-retail world corpus manifest covering representative maps, USA/China/GLA factions, and every M9 family/state.
- Manifest parser/validator that rejects unknown, duplicate, missing, or malformed required state.
- Recorder live-resource accounting and lifecycle stress assertions.
- Four-preset renderer-contract validation.

## Non-scope

No retail bytes or paths, asset decoding, effects/water/WWShade, GPU/pixel evidence, or broad performance benchmarks.

## Dependencies and ordering

Depends on slices 01 and 02. It closes M9 but does not update milestone workflow state, which remains orchestrator-owned.

## Entry point and behavior

The corpus validator reads a checked-in logical TSV, maps each row into known world families/factions/states, and reports any unmapped required value. Lifecycle tests load each representative case, run repeated frames, teardown, reload, and compare resource/pipeline high-water marks and final live counts.

## Data/state transitions

Manifest text becomes a validated coverage set or an error without partial acceptance. Device resource counts increase within declared bounds during load and return to zero after teardown; reload repeats the same bounds.

## Authorization and permissions

Read-only project-owned test data only. No private retail media, network, display, or GPU access.

## Validation and recovery

Positive tests require complete coverage. Negative/boundary tests inject an unknown state, duplicate entry, missing faction/family, and exceed a bounded resource/pipeline capacity. Corrected input or a fresh recorder may retry.

## Implementation surfaces

- `data/corpus/world-rendering-manifest.tsv`
- `include/zh/world/corpus.h`
- `src/world/corpus.cpp`
- `include/zh/renderer/recording_device.h`
- `src/renderer/recording_device.cpp`
- `tests/world/test_world_corpus.cpp`
- `CMakeLists.txt`
- governing plan completion record

## Required commands

```sh
cmake --build --preset linux-gcc-debug
ctest --preset linux-gcc-debug -L renderer-contract --output-on-failure
cmake --build --preset linux-clang-debug
ctest --preset linux-clang-debug -L renderer-contract --output-on-failure
cmake --build --preset linux-gcc-release
ctest --preset linux-gcc-release -L renderer-contract --output-on-failure
cmake --build --preset linux-clang-release
ctest --preset linux-clang-release -L renderer-contract --output-on-failure
```

## Acceptance criteria

- Required logical corpus coverage is complete and unmapped state is rejected.
- Repeated frames/load/teardown/reload stay within fixed high-water limits.
- All live buffer, texture, sampler, shader, and pipeline counts are zero after teardown.
- All four supported presets pass the renderer-contract suite.

## Commit boundary

Commit corpus closure, accounting, lifecycle tests, and final plan record as `delivery: M9 slice 03 prove bounded world lifecycle`.
