# M7 delivery plan: renderer core contracts

## Governing contract

Deliver [M7 renderer core](../../milestones/cnc-generals-zero-hour/M7-renderer-core.md) from the authoritative Linux port plan. The transaction starts at `52128c7447f40d6d963b89cff9e2b08d9d685ef0`. It remains asset-free, headless, and GPU-independent; M14 owns the SDL_GPU hardware implementation and pixels.

## Scope and invariants

The implementation supplies a recording `GpuDevice`, opaque generation-checked resources, bounded immutable pipeline reuse, deterministic normalized command snapshots, DDS/TGA loading with BC fallback, resize command sequencing, and the legacy-facing state cache. Engine-facing renderer code continues to depend only on `include/zh/renderer`; SDL_GPU/Vulkan types and calls are forbidden outside the future backend.

Authorization is not applicable: these are in-process renderer contracts. Inputs are validated before state mutation. Errors retain operation and resource-label provenance. Retail files, display servers, GPU devices, and network access are neither read nor required.

## Dependency-ordered slices

1. [Slice 01: recording device and resource contracts](milestone_07_plan_01_slice_01.md)
2. [Slice 02: DDS/TGA parsing and BC fallback](milestone_07_plan_01_slice_02.md)
3. [Slice 03: DX8 state cache and resize command generation](milestone_07_plan_01_slice_03.md)

Each slice is separately reviewable and revertible. The final milestone validation builds all four presets and runs `ctest --preset <preset> -L renderer-contract --output-on-failure`, followed by a source-boundary scan proving no engine-side SDL_GPU/Vulkan escape.

## Acceptance

All M7 behaviors and named negative cases have deterministic automated evidence. Snapshots normalize unstable numeric handles while preserving event order, descriptor state, labels, and error provenance. Earlier milestone labels remain passing in the canonical full suite.

## Delivery record

Commits are recorded here after each completed slice.

- Slice 01: this slice's commit (`delivery: M7 slice 01 add recording GPU device`)
- Slice 02: this slice's commit (`delivery: M7 slice 02 add texture parsing fallback`)
- Slice 03: pending
