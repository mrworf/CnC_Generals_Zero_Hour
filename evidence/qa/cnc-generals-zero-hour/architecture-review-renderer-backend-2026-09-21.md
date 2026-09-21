# Scope

Targeted revalidation of the renderer backend decision in `docs/zero-hour-linux-port-plan.md` §9 after the M22 06C3C source-identified viewport clear blocker. This is not a general audit of the game or a claim that original retail scenes render yet.

# Executive Summary

The second §9 branch is now demonstrated: SDL_GPU 3.4.14 lacks a public viewport-scoped color/depth/stencil clear within an active render pass, while bgfx's public view API performs the required ordered, rectangular clear on this host's Vulkan RTX 4070. Select bgfx for the device/backend layer; retain SDL3 windowing and input and preserve the engine-facing `GpuDevice` abstraction. The migration and revalidation must precede M22's remaining scene acceptance and M24's dependent retail-first-tick checks.

# What Is Good

- Good: `include/zh/renderer/contract.h` exposes opaque handles and backend-neutral commands, and `RecordingGpuDevice` independently validates ordering and lifetimes. This limits the renderer swap to the device edge plus the missing clear command; completed source and CPU providers do not need wholesale reimplementation.
- Good: M22's blocker evidence names the original `CameraClass::Apply` → `DX8Wrapper::Clear` consumer and a bounded 160×120 fixture. This makes the decision testable instead of preference-driven.
- Good: the exact headless probe in `tools/renderer/bgfx_viewport_clear_probe.cpp` passed on NVIDIA RTX 4070 / driver 610.57.04 / Vulkan 1.4.341 with bgfx commit `81d81fba72c42d348c589514c774bbfe01e110fa`. Outer red/depth .25/stencil 7 survived the inset blue/depth 1/stencil 0 clear; subsequent depth-less and stencil-equal draws changed only the inset to green. All tested pixels passed, exit 0. The sandbox hid the NVIDIA ICD, so the probe ran on the host; `vulkaninfo --summary` confirmed the driver there.

# What Is Bad Or Risky

- Risky: `src/renderer/sdl_gpu_device.cpp` can express full-attachment pass-start clear and viewport for draws, but not the original camera-scoped clear after earlier frame work. Treating a full-target clear or proxy rectangle draw as equivalent would violate original rendering semantics.
- Risky: `include/zh/renderer/contract.h` currently lacks a distinct viewport-clear operation and stencil clear value. A backend swap without first correcting this contract would merely relocate the gap.
- Risky: current offline SPIR-V output and SDL_GPU resource/pipeline conventions cannot be assumed to drop into bgfx. bgfx requires its own shader packaging and public resource/state mapping. The existing renderer corpus, not just this clear probe, needs revalidation.

# What Should Change

- Change: adopt `docs/zero-hour-renderer-backend-migration.md` as the bounded §9 decision and implementation-ready migration contract. Add a recorder-visible ordered viewport clear with independent color/depth/stencil flags, exact values, clipping and negative validation, then implement it through bgfx views. Preserve SDL3 platform/input.
- Change: put a renderer migration/revalidation gate before the remaining M22 06C3C work. Recheck the M2 inventory and M7–M10 contract/shader/producer evidence, and rerun M14 hardware/visual/lifecycle acceptance on bgfx. Keep completed milestone histories intact but do not treat their SDL_GPU-specific acceptance as bgfx acceptance.
- Change: test interleaved scene clears, multiple target generations, pass/view exhaustion, resize and device failure, original W3D/WWShade draw categories, and positive/negative renderer paths. The probe establishes one decisive capability, not whole-engine compatibility.

# What I Would Not Change Yet

- Do not change the original game symlink or retail contents, rewrite gameplay/serialization/network systems, add a private Vulkan escape under SDL_GPU, or commit bgfx binaries. Those actions are unnecessary to solve this renderer boundary.
- Do not select raw Vulkan: the tested public bgfx capability satisfies the §9 fallback condition, while raw Vulkan carries a larger device-management obligation.

# Overall Opinion

The prior architecture is maintainable at the engine boundary but its provisional SDL_GPU implementation is not sufficient for source-faithful camera clears. A bounded bgfx backend migration is justified by the exact source consumer and executable GPU result. Delivery should preserve completed work, introduce a dependency-safe migration gate, and resume M22/M24 only after that gate passes.
