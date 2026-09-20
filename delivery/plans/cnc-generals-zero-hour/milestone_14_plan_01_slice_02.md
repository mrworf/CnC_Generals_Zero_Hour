# M14 slice 02: real Vulkan rendering, presentation, and lifecycle

## Goal and observable outcome

The SDL_GPU device executes pipelines, passes, bindings, draws, and presentation on a Vulkan swapchain, and recovers predictably across focus, resize, fullscreen, and explicit device recreation.

## Scope

- Map contract pipeline/raster/depth/blend/layout state to public SDL_GPU structures.
- Record uploads on copy command buffers, push uploaded uniform shadows, bind vertex/index buffers and samplers, execute offscreen render passes, and submit safely.
- Claim an SDL window, present the last completed color target through the swapchain, add debug groups/labels, and expose lifecycle operations.
- Add synthetic SPIR-V acceptance shaders and a GPU test executable covering positive and negative resource/pass/pipeline/presentation behavior.

## Non-scope

Retail scene traversal and final acceptance evidence are slice 03. Pixel-perfect legacy approval and performance tuning beyond functional sufficiency are not silently inferred from one synthetic frame.

## Dependencies and ordering

Requires slice 01 resource/device foundation.

## Entry point and end-to-end behavior

The GPU harness creates an SDL window and Vulkan SDL_GPU device, creates resources and a pipeline, uploads generated geometry, renders to a project-owned offscreen target, blits it to the swapchain, submits/presents, then exercises focus, resize, fullscreen transition, idle/wait, release, and recreation.

## Data/state transitions

Command state: idle -> upload copy -> submitted; idle -> render pass active -> completed -> submitted; window: unclaimed -> claimed -> presenting -> released. Resize/loss recreation increments a generation and invalidates no live engine handle unless recreation cannot restore it, in which case the failure names the phase and resource.

## Authorization and permissions

No application authorization applies. Test execution is explicitly opt-in because it opens a local window and uses the GPU.

## Validation and recovery

- Positive: upload, pipeline, pass, draw, present, resize, fullscreen, focus, wait-idle, destroy, and recreate succeed.
- Negative: nested/no-active passes, incompatible target/pipeline formats, missing bindings, unsupported topology/format, invalid window, and stale resources fail clearly.
- If SDL_GPU exposes a genuine public capability gap, capture the consumer and a minimal reproducer; do not work around it with Vulkan.

## Expected implementation surfaces

SDL_GPU device source/header, platform native-window access if required, acceptance shaders, GPU tests, and CMake.

## Validation commands

- `cmake --build --preset linux-gcc-debug`
- `ctest --preset linux-gcc-debug -L 'renderer-contract|ui|video' --output-on-failure`
- GPU build: `ctest --test-dir <gpu-build> -L gpu --output-on-failure`

## Acceptance criteria

- A real SDL_GPU Vulkan command stream reaches presentation through public APIs.
- Lifecycle transitions complete without leaks or stale pass state, and negative cases are tested.

## Commit boundary

One commit for executable rendering/presentation and lifecycle: `delivery: M14 slice 02 render and present with SDL GPU`.
