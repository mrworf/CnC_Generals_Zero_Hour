# M14 delivery plan: SDL_GPU integration and hardware acceptance

## Authority and transaction

- Milestone contract: `delivery/milestones/cnc-generals-zero-hour/M14-gpu-acceptance.md`
- Product authority: `docs/zero-hour-linux-port-plan.md`, especially sections 6, 8/M14, 9, and 10.
- Transaction start: `392fb89df3ef1cbb0875171aa9dd9947454dd0a3`.
- Product id: `cnc-generals-zero-hour`.
- Retail content is read-only input. No retail byte, digest, filename inventory, or private absolute path may be committed.

## Outcome

Connect a Vulkan-only SDL_GPU implementation behind the M2/M7 device contract, run the existing command generators on that implementation, exercise presentation and window/device lifecycle behavior, and produce project-owned hardware acceptance evidence. The implementation must remain optional at configure/test time so all ordinary tests continue to run without a display, GPU, validation layers, or retail data.

M14 cannot be accepted until `VK_LAYER_KHRONOS_validation` is installed and an enabled validation run reports no errors. Missing validation tooling is an external acceptance blocker, not permission to weaken the test or make an unsupported claim.

## Dependency and design decisions

- Preserve the engine-facing `GpuDevice` boundary. Recorder-specific diagnostics used by UI/world/effects/video become backend-neutral methods on that boundary.
- Use only public SDL_GPU operations. Do not introduce raw Vulkan access or a private escape hatch.
- Keep `ZH_ENABLE_GPU_TESTS=OFF` by default. Hardware probes are registered only when explicitly enabled.
- Use checked-in synthetic shaders and generated geometry for automation. Retail inputs may select/characterize representative logical scenes, but must not be copied into the repository.
- Project-owned captures and metrics go below `evidence/qa/cnc-generals-zero-hour/` and contain only stable, redacted context.

## Slice index

1. [Slice 01 — shared device contract and SDL_GPU resources](milestone_14_plan_01_slice_01.md)
2. [Slice 02 — real Vulkan rendering, presentation, and lifecycle](milestone_14_plan_01_slice_02.md)
3. [Slice 03 — representative scene matrix and acceptance evidence](milestone_14_plan_01_slice_03.md)
4. [Slice 04 — validation-layer image-view and layout corrections](milestone_14_plan_01_slice_04.md)

## Milestone validation

- Configure and build all four canonical presets.
- Run `ctest --preset <preset> -L 'gpu|ui|video' --output-on-failure` for all four presets; GPU tests remain absent/skipped unless the preset/build explicitly enables them.
- In a separate GPU build, enable `ZH_ENABLE_GPU_TESTS=ON` and exercise Vulkan device/resource/upload/pass/pipeline/draw/presentation plus focus, resize, fullscreen, and recreation.
- In a separate retail+GPU build, enable both opt-ins and exercise the user-owned corpus without persisting private inputs.
- Record SDL, driver/device, capability, scene, lifecycle, timing, capture, and validation-layer results.

## Completion boundary

All safe implementation and non-validation-layer checks may be committed. If the Khronos layer remains unavailable, report M14 as blocked at PRE-012/PRE-013 acceptance with the exact probe result; do not label the milestone complete and do not claim zero validation errors.

## Delivery checkpoint

- Slice 01: `c17c0dc` — shared device contract and SDL_GPU resources.
- Slice 02: `89de107` — real Vulkan rendering, presentation, and lifecycle.
- Slice 03: `0440a4f` — representative scene matrix, evidence, and fail-closed validation gate.
- Slice 04: validation-layer image binding corrections and fail-closed diagnostic scanning (this slice commit).
- Acceptance evidence: PRE-012 is installed and both validation-enabled GPU suites rerun without validation diagnostics; outer milestone status remains owned by the delivery orchestrator.
