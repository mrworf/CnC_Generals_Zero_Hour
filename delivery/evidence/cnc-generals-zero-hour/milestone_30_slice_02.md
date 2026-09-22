# M30 slice 02 — ordered target/clear evidence

Status: focused physical target/clear evidence, not full shader/draw or original retail acceptance.

- Parent slice: `64febece290c53063e0c77d02579369a85a9d617`.
- Host `vulkaninfo --summary`: NVIDIA GeForce RTX 4070, Vulkan 1.4.341, NVIDIA driver 610.57.04; `VK_LAYER_KHRONOS_validation` 1.4.357 available. No private device UUID or retail data is committed.
- Source-shaped device test: begin full red/depth clear → set camera viewport → inset blue/depth/stencil clear → end → bounded BGRA8 readback. Outer/inset boundary pixels pass on RTX. Before allocating a new view in `set_viewport`, the exact test failed because the prior full-target clear was retroactively clipped; the corrected ordered-view implementation passes.
- Device negative cases: uninitialized color/depth LOAD, nested pass, stale target generation, fractional viewport/unsupported depth range, inactive clear/end, view 257 exhaustion. A subsequent frame reclaims the 256-view budget. Separate depth-only and stencil-only clears on an initialized LOAD pass preserve the color pixels; depth and stencil initialization are tracked separately.
- GCC and Clang Debug focused builds and CPU contract tests passed. Rebuilt GCC device test passed on RTX with `VK_INSTANCE_LAYERS=VK_LAYER_KHRONOS_validation`, exit 0 and no emitted Vulkan validation errors/VUIDs.
- Exact checked standalone `tools/renderer/bgfx_viewport_clear_probe.cpp` was rebuilt against the pinned shared runtime and passed on RTX with the same explicit validation layer: outer red survives, inset blue changes, later depth-LESS and stencil-EQUAL draws show only the inset green; exit 0, no emitted validation errors/VUIDs.

The standalone probe is public-bgfx API evidence. The device itself has no shader/draw submission yet, so device-level independent depth/stencil-tested draws, M7–M10/M14 pixel acceptance, presentation, four presets and full asset-free suite remain pending. No retail content was touched.
