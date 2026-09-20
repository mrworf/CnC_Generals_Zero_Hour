# M9 delivery plan: world rendering command generation

## Authority and outcome

This plan delivers [M9](../../milestones/cnc-generals-zero-hour/M9-world-rendering.md) from `docs/zero-hour-linux-port-plan.md` at transaction parent `9d76630c324abd2e8ef5f845a385a0af0c878464`. It produces deterministic, recorder-backed world command streams for the full M9 scene vocabulary without requiring retail media, a display, or a GPU.

M4's logical English corpus and M7's normalized renderer contract are accepted inputs. M10 effects/water/WWShade and M14 rasterized-pixel acceptance remain out of scope.

## Design decisions

- Add a backend-neutral `zh::world` scene contract and recorder that owns map-scoped GPU resources and emits through `RecordingGpuDevice` only.
- Represent every M9 world family explicitly: camera, mesh, streamed/animated geometry, terrain, road/decal, tree, shadow, faction geometry, and selection marker. Unknown or incomplete scene state fails closed with a scene/item diagnostic.
- Normalize the effective fixed-function tuple into immutable descriptors. Transform/light/fog/alpha-test/texture-stage decisions are recorded as named markers and the sampler, cull, depth, blend, and color-mask decisions are present in the validated pipeline key.
- Use M7's left-handed, clockwise-front-face, top-left-origin, 0..1 depth conventions, with focused contract assertions.
- Create map resources once, stream dynamic vertices/indices/uniforms by bounded reuse each frame, and destroy every owned handle on teardown. Reloading the same scene must not grow the live resource or pipeline set.
- Keep representative corpus input project-owned and logical. No retail bytes, hashes, or absolute host paths are committed.

## Slices

1. [Slice 01 — world scene and fixed-function translation contract](milestone_09_plan_01_slice_01.md)
2. [Slice 02 — deterministic recorder-backed world commands](milestone_09_plan_01_slice_02.md)
3. [Slice 03 — corpus closure and bounded lifecycle](milestone_09_plan_01_slice_03.md)

Slices are dependency ordered and must be committed separately. The milestone-level validation after slice 03 is:

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

## Completion record

Slice commits are recorded here after each validated transaction.

- Slice 01: `e478c10` (`delivery: M9 slice 01 define world scene contract`)
- Slice 02: pending
- Slice 03: pending
