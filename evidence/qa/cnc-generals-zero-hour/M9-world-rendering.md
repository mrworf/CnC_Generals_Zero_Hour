# M9 world-rendering command acceptance evidence

## Context

- Platform: Arch Linux x86-64.
- Compilers/configurations: GCC and Clang, Debug and Release.
- Renderer boundary: `RecordingGpuDevice`; no display, Vulkan device, or rasterized-pixel claim.
- Corpus boundary: project-owned logical M9 manifest and synthetic scene descriptions. No retail bytes, hashes, private paths, or copied assets are recorded or required.
- Explicit exclusions: effects/water/WWShade remain M10; real-GPU and pixel acceptance remain M14.

## Command and state coverage

The validated world vocabulary covers static and animation-visible meshes, terrain, roads/decals, trees, shadows, USA/China/GLA faction geometry, selection markers, cameras, dynamic vertex/index streams, and the shadow-map-to-main-pass dependency.

The fixed-function translation tests lock left-handed coordinates, 0..1 depth, clockwise front faces, top-left textures, ARGB8 colors, view-space fog, transforms, lighting, alpha test, one through four texture stages, sampler addressing, culling, depth test/write/compare/bias, blend state, and color write masks. Unknown families, factions, states, invalid numeric ranges, malformed/duplicate corpus entries, and insufficient pipeline capacity fail closed with contextual diagnostics.

## Determinism and lifecycle

Equivalent scenes supplied in opposite item orders emitted byte-identical normalized command streams. A 64-frame run retained an unchanged live set of five buffers, seven textures, four samplers, two shaders, and no more than sixteen immutable pipelines. Teardown returned every live resource count to zero. Reload reproduced the same bound, two further frames did not grow it, and the second teardown again returned all counts to zero.

## Validation

All four canonical presets configured and built successfully. Each passed all 13 `renderer-contract` tests, including the three M9 world tests:

- `linux-gcc-debug`
- `linux-clang-debug`
- `linux-gcc-release`
- `linux-clang-release`

The complete GCC Debug suite also passed all 35 tests, preserving M0-M8 behavior.

## Result

PRE-021 is satisfied for GPU-independent world command generation. Representative maps and all three factions have deterministic, validated command streams, required logical corpus state is fully mapped, resource/pipeline growth is bounded across repeated frames and reload, and teardown leaks no recorded handles. Pixel correctness is deliberately not claimed.
