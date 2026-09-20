# M14 slice 01: shared device contract and SDL_GPU resources

## Goal and observable outcome

An SDL_GPU Vulkan device can be constructed through project code and can create, label, upload, query, and release the buffer, texture, sampler, and SPIR-V shader resources represented by the existing M2 contract. Existing UI/world/effects/video generators can target any `GpuDevice`, while recorder snapshots remain unchanged.

## Scope

- Promote recorder-only marker/error/pass-state operations needed by consumers into `GpuDevice`.
- Generalize consumer constructor/member types from `RecordingGpuDevice` to `GpuDevice` without changing generated commands.
- Add an RAII `SdlGpuDevice` with explicit Vulkan selection, debug mode, shader-root configuration, capability reporting, resource tables, labels, buffer uploads, and actionable errors.
- Add asset-free contract tests for option validation and failure behavior.

## Non-scope

Graphics pipelines, render passes, draws, swapchain presentation, lifecycle acceptance, retail-data traversal, and visual approval are deferred to later slices.

## Dependencies and ordering

Requires completed M2/M7-M10/M13 contracts. This slice must precede slices 02 and 03.

## Entry point and end-to-end behavior

Callers create `SdlGpuDevice` with a checked shader directory. With a usable Vulkan driver it reports SDL/device capabilities and services resource/upload operations; invalid options, absent shaders, stale handles, and unsupported formats fail with operation and label context. Existing recorders continue to pass their current tests through the same abstract interface.

## Data/state transitions

Device: uninitialized -> active -> drained/destroyed. Resource slots: absent -> live native object -> released, with stale handles rejected. Dynamic buffer uploads update a CPU shadow and submit through an SDL transfer buffer.

## Authorization and permissions

No authorization model applies. The opt-in device test may access the local display/GPU only. It does not write outside its build/evidence output and does not mutate retail content.

## Validation and recovery

- Positive: recorder regressions pass; a supported device can create/upload/release representative resources.
- Negative: invalid shader root/code, invalid descriptors, null uploads, stale handles, and missing Vulkan driver return actionable errors without leaks.
- Device construction/destruction and partial resource failures clean up safely.

## Expected implementation surfaces

`include/zh/renderer/contract.h`, consumer headers/sources, new SDL_GPU device header/source, renderer tests, and CMake target/test registration.

## Validation commands

- `cmake --build --preset linux-gcc-debug`
- `ctest --preset linux-gcc-debug -L 'renderer-contract|ui|video' --output-on-failure`
- Focused SDL_GPU resource test in an explicitly GPU-enabled build when available.

## Acceptance criteria

- Consumers compile against `GpuDevice`, recorder behavior remains green, public SDL_GPU creates the Vulkan device/resources, and every failure is labeled.
- No raw Vulkan call or retail data enters the implementation.

## Commit boundary

One commit containing this plan, the shared-interface seam, SDL_GPU resource/upload implementation, tests, and build wiring: `delivery: M14 slice 01 connect SDL GPU resources`.
