# M2 slice 03: shader registry and offline compilation

## Goal and observable outcome

Named UI, terrain, water, points, and WWShade vertex/fragment shader pairs compile offline in every canonical preset, and a checked registry completely maps the required families to pipeline features without opening a GPU device.

## Scope

- Add repository-owned GLSL for the five required renderer families.
- Compile every module to SPIR-V through explicit CMake custom commands and dependencies.
- Add a typed shader/effect registry connecting legacy family names, stages, vertex layouts, uniform counts, sampler counts, blend/depth/cull requirements, and point-size behavior.
- Test registry uniqueness/completeness, resource-limit conformance, source existence, expected SPIR-V outputs, and backend declaration.
- Document conventions, mapping-table use, provisional SDL_GPU boundary, and M14 hardware deferral.

## Non-scope

No pixel goldens, SDL_GPU calls, display, synchronization/performance claim, retail shader discovery, or Vulkan validation.

## Dependencies and ordering

Slices 01 and 02. Completes M2.

## Entry point and end-to-end behavior

Building `zh_renderer_shaders` invokes `glslc` for ten explicit GLSL files. The contract test walks the registry, validates descriptors against engine limits, and verifies every registered stage has both source and generated SPIR-V paths.

## Data/state transitions

Source GLSL deterministically yields build-tree SPIR-V. Registry data is immutable static metadata.

## Authorization and permissions

Not applicable: project-owned shader inputs and build outputs only.

## Validation and error handling

- Positive: all five families and ten stages compile and validate under GCC/Clang Debug/Release presets.
- Negative: duplicate/missing families, excessive resources, absent stage files, or an intentionally invalid registry entry are rejected with family-specific diagnostics.
- The complete M0-M2 test suite remains green in one canonical preset.

## Expected implementation surfaces

`shaders/renderer/*`, renderer registry headers/sources/tests, `docs/renderer/renderer-contract.md`, CMake shader commands/installation, and plan status/evidence notes.

## Required commands

- `cmake --build --preset <preset>` for all four presets.
- `ctest --preset <preset> -L renderer-contract --output-on-failure` for all four presets.
- full `ctest --preset linux-gcc-debug --output-on-failure`.
- `git diff --check` and tracked-content private-path scan.

## Acceptance criteria

- UI, terrain, water, points, and WWShade each have compiled vertex and fragment modules.
- Registry/table coverage and resource limits are mechanically checked.
- Documentation records SDL_GPU as provisional and assigns device/pixel/synchronization/performance acceptance to M14.
- All M2 and regression validations pass without GPU, display, retail content, or network.

## Commit boundary

Commit shaders, registry, tests, documentation, CMake integration, this slice plan, and governing-plan completion/evidence as `delivery: M2 slice 03 compile renderer shaders`.
