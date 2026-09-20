# M14 slice 04: validation-layer image-view and layout corrections

## Goal and observable outcome

The validation-enabled RTX/Vulkan acceptance run completes the full generated and retail-backed M14 suites without `Validation Error` or VUID output. Projected textures bind an image view compatible with the shader's 2D sampler, and a render target that is subsequently sampled reaches the shader-readable layout through public SDL_GPU operations.

## Scope

- Diagnose and correct the projected-texture cube-versus-2D binding that triggers `VUID-vkCmdDraw-viewType-07752`.
- Diagnose and correct the shadow render-target write-to-sample transition that triggers `VUID-vkCmdDrawIndexed-imageLayout-00344`.
- Preserve the public `GpuDevice` abstraction and use public SDL_GPU APIs only.
- Add focused regression detection that makes validation diagnostics fail the GPU test when SDL exposes a usable logging path.
- Update M14 evidence with the validation-enabled results and final disposition, without private paths or retail content.

## Non-scope

Raw Vulkan access, backend replacement, renderer redesign, unrelated effect changes, performance tuning, and changes to retail inputs are excluded.

## Dependencies and ordering

Requires slices 01-03, the installed `vulkan-validation-layers` package, a discoverable `VK_LAYER_KHRONOS_validation`, the RTX/Vulkan display session, and the existing opt-in GPU and retail+GPU builds. This is the corrective successor to validation evidence recorded at commit `2e65665`.

## Entry point and end-to-end behavior

The existing `renderer_gpu_acceptance` entry point drives effects and world rendering through `EffectsRecorder`, `WorldRecorder`, and `SdlGpuDevice`. Investigation showed that SDL_GPU preserved the declared texture dimensions correctly: the effects consumer incorrectly reused its cube texture in WWShade's 2D slot. The shadow finding was same-pass attachment feedback: a shadow caster sampled the active shadow-map color target, rather than a missing transition between passes. Each consumer now binds a dimension-compatible, non-attached texture through the unchanged public device abstraction. The test captures combined validation output, counts validation errors/VUIDs, and returns failure if any are observed.

## Data and state transitions

- Ordinary effect slots remain 2D; the dedicated environment texture is bound only to the bump shader's cube sampler.
- A shadow caster uses a neutral 2D fallback while the shadow map is its active attachment; main-world draws bind the completed shadow map after the dependency pass ends.
- Validation diagnostics move the acceptance result from running to failed; only a complete zero-error run may be recorded as accepted.

## Authorization and permissions

No application authorization applies. Hardware/display execution may run outside the restricted sandbox. Retail roots remain explicit, caller-owned, and read-only; no retail bytes, filenames, hashes, or private absolute paths may enter source or evidence.

## Validation and error handling

- Positive: 2D projected textures and rendered shadow textures are sampled successfully across both device generations; validation-enabled GPU and retail+GPU suites exit successfully with zero `Validation Error` and zero VUID output.
- Negative: an injected/captured validation error causes the diagnostic monitor or test helper to fail; unsupported/stale texture state continues to return actionable device errors.
- Recovery: on any validation finding, retain the diagnostic, identify the exact scene/resource, and keep M14 unaccepted until the same complete suites rerun cleanly.

## Expected implementation surfaces

Effects/world recorder sources and headers, focused recorder tests, GPU acceptance registration and validation-output helper coverage, the governing plan, build documentation, and `evidence/qa/cnc-generals-zero-hour/M14-gpu-acceptance.md`. No SDL_GPU backend change is required because the defect is in consumer bindings.

## Required validation commands

- Focused build and renderer/effects tests for texture dimension and render-to-sample behavior.
- `VK_INSTANCE_LAYERS=VK_LAYER_KHRONOS_validation ctest --test-dir build/m14-gpu -L gpu --output-on-failure` with captured combined output checked for `Validation Error` and `VUID-`.
- The same validation-enabled `gpu|ui|video` suite in the retail+GPU build, with combined output checked identically.
- Canonical full regression suite and affected preset build when implementation stabilizes.

## Acceptance criteria

- Neither reported VUID reproduces in either validation-enabled suite.
- Automated regression detection fails on validation errors when supported by SDL's public logging interface.
- Both complete outputs contain zero `Validation Error` and zero `VUID-`; all required tests pass.
- Evidence records the installed validation-layer version, device/runtime context, clean result, and scene/lifecycle coverage without private data.

## Commit boundary

One exact-path commit containing this plan/index, both public-SDL corrections, targeted regression coverage, and updated evidence: `delivery: M14 slice 04 clear Vulkan validation errors`.
