# M9 slice 01: world scene and fixed-function translation contract

## Goal and observable outcome

A caller can describe all M9 world families and translate a complete legacy fixed-function state into a validated, immutable world material/pipeline descriptor. Invalid cameras, transforms, alpha tests, texture-stage counts, or unmapped world families fail with actionable diagnostics.

## Scope

- World camera, transform, lighting, fog, alpha-test, texture-stage/addressing, geometry-family, faction, and draw-item types.
- Validation and deterministic descriptor translation for blend, color masks, depth, culling, topology, sampler addressing, fog and alpha-test shader features.
- Coordinate-convention assertions for left-handed space, clockwise front faces, 0..1 depth, top-left textures, ARGB8 colors, and view-space fog.
- Positive, negative, and boundary contract tests.

## Non-scope

No command emission, resource ownership, retail parsing, effects, water, WWShade, GPU creation, or pixel validation.

## Dependencies and ordering

Depends on accepted M2/M7 renderer descriptors. Must precede slices 02 and 03.

## Entry point and behavior

`validate(WorldScene)` traverses the complete scene and rejects non-finite/out-of-range or unmapped state. `translate_material(...)` converts one validated effective legacy state plus shader handles into a `PipelineDesc`, sampler descriptors, and explicit shader-feature metadata.

## Data/state transitions

Pure input-to-result translation; no persistent state. A valid description yields the same descriptor and stable pipeline key on every call. A failed translation returns no partial descriptor.

## Authorization and permissions

Not applicable: this is offline in-process descriptor validation with no external I/O.

## Validation and recovery

Errors name the field or item/family. Callers may correct input and retry. Tests cover a complete scene, every family mapping, maximum texture stages, and failures for an unknown family, invalid camera/depth/fog/alpha/stage/address/transform state.

## Implementation surfaces

- `include/zh/world/scene.h`
- `src/world/scene.cpp`
- `tests/world/test_world_scene.cpp`
- `CMakeLists.txt`
- governing/slice plans

## Required commands

```sh
cmake --build --preset linux-gcc-debug --target world_scene_tests
ctest --preset linux-gcc-debug -R '^world_scene$' --output-on-failure
```

## Acceptance criteria

- Every M9 family is enumerable, named, and accepted in a complete representative scene.
- Relevant fixed-function state maps to validated renderer descriptors and explicit shader features.
- Conventions and boundary/failure behavior are locked by tests.

## Commit boundary

Commit the plan artifacts, scene contract/translation, build registration, and focused tests as `delivery: M9 slice 01 define world scene contract`.
