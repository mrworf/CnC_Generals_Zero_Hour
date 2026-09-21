# M22 plan 01 slice 03: validation-enabled Vulkan and milestone acceptance

## Goal and observable outcome

The exact original campaign and skirmish producer path accepted on the recording device renders and presents real retail scenes on the RTX 4070 through SDL_GPU Vulkan with explicit Khronos validation, reviewed visuals, bounded resize/resource recreation, and clean teardown. The complete M22 regression matrix passes.

## Scope

- Bind the slice 02 original producer entry point to `SdlGpuDevice` and a resizable SDL window without changing the scene producer or translating through a generated scene.
- Present and visually capture every required scene family from the selected real campaign/skirmish scenes.
- Exercise resize, swapchain/window presentation transitions, device-resource recreation, a second frame/generation, wait-idle, reset, and reverse teardown.
- Run explicit validation-layer output scanning and hardware/device identity reporting without private retail details.
- Complete four-preset, full-suite, sanitizer, installed/arbitrary-CWD, identity, provider-removal, ledger, and privacy acceptance evidence.

## Explicit non-scope

- UI/media interaction, input semantics, complete gameplay sessions, performance targets, multiple GPUs, or release certification.
- Generated M14 scenes or screenshots as original-scene acceptance.

## Dependencies and ordering

- Requires slice 02 recording acceptance and external PRE-008/PRE-012/PRE-016 availability.
- Hardware tests remain separate opt-in gates and never become default asset-free CTest prerequisites.

## Entry point and end-to-end behavior

The retail gate launches the installed production rendering executable from an arbitrary CWD with isolated XDG roots, explicitly enables `VK_LAYER_KHRONOS_validation`, opens a resizable SDL window, runs each slice 02 original scene, presents it, captures permitted visual evidence, resizes/recreates resources, renders again, waits idle, resets, destroys all resources, and exits within the bounded timeout.

## Data and state transitions

- No device -> Vulkan device/window claim -> original scene resource upload -> render/pass/present -> visual capture -> resize invalidation/recreation -> second render/present -> idle -> release window/device -> zero resources.
- Validation-layer output, presentation/resource failure, incomplete scene-family capture, stale handle, timeout, or nonzero resource count fails the gate even if the process exit code would otherwise be zero.

## Authorization and permission behavior

This slice may use the verified local graphical/GPU session. Retail roots remain read-only, generated evidence is limited to permitted project-owned/derived visual output, and all writable state is temporary/XDG isolated.

## Validation and error handling

- Positive: both retail scenario families render/present required original scene families before and after resize/recreation; device report identifies Vulkan/RTX/NVIDIA/SDL without private asset data.
- Negative: validation-output scanner rejects synthetic `Validation Error` and `VUID-`; layer absence, non-Vulkan backend, missing visual frame, stale resources, and incomplete teardown fail closed.
- Visual review: confirm actual terrain/object composition, faction/world material differentiation, camera framing, lighting/fog/shroud, shadow, particles/water/effects where required, no placeholder geometry/textures, and stable resized framing/legibility.

## Expected implementation surfaces

- `CMakeLists.txt`
- original-rendering SDL/Vulkan executable and binding under `src/original_runtime/` or `tests/original_rendering/`
- validation wrapper/capture/privacy tools under `tools/`
- hardware/retail gates under `tests/original_rendering/`
- permitted evidence under `evidence/qa/cnc-generals-zero-hour/` and `evidence/platform/cnc-generals-zero-hour/`
- governing plan and all slice plan statuses

## Required validation commands

- Configure/build all four native presets and run their complete asset-free CTest suites (including established separately escalated local-UDP tests).
- Configure/build a GPU+retail acceptance tree; run with `VK_INSTANCE_LAYERS=VK_LAYER_KHRONOS_validation` through the validation-clean wrapper and scan combined output for zero `Validation Error` and zero `VUID-`.
- Run focused Clang Debug ASan+UBSan original-rendering tests with strict halt/abort settings; hardware execution need not run under ASan if the repository's SDL/Vulkan stack is incompatible, but equivalent owned producer/lifecycle coverage must.
- Run installed/arbitrary-CWD retail gates, identity/provider-removal, ledger freshness, source classification, privacy checks, and `git diff --check`.

## Acceptance criteria

- The hardware and recording paths invoke the same actual original W3D/WWShade/GameClient producer entry point and equivalent selected scenes.
- Explicit Khronos validation is active and combined output contains zero validation errors/VUIDs.
- Captured/reviewed visual evidence covers every required scene family, actual retail composition/assets, and resize/recreation without placeholders.
- Missing layer/backend/visual output, stale resources, incomplete families, or nonzero teardown ownership fails the gate.
- Four presets/full suites, focused sanitizers, installed/arbitrary-CWD, identity/provider-removal, ledger freshness, retail read-only/privacy, and bounded lifecycle checks all pass.

## Commit boundary

Commit Vulkan integration, validation/visual/lifecycle gates, cumulative evidence, and final plan status updates together as `delivery: M22 slice 03 validate original Vulkan scenes`.
