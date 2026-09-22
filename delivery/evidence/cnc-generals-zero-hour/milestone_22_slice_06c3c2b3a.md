# M22 06C3C2B3A original source/static bgfx evidence

## Outcome

The original `WW3D::Begin_Render` → `Render(SimpleSceneClass, CameraClass)` → `End_Render` path submits a source asset-manager rigid mesh and two `MeshClass` static-sort levels through the public bgfx Vulkan device. The fixture loads three generated W3D mesh families through `WW3DAssetManager`, never adapter-authored geometry. Original static-list callbacks drain level 2 then level 1. The pixel oracle distinguishes full-frame outer clear, camera viewport clear, rigid coverage, both static levels, and removal of level 1 at 160×120 and 200×150 in BGRA8 and RGBA8. Each format/extent gets a fresh device generation. Public caller-owned targets retire after original edge/WW3D shutdown, and live handles reach zero after target destruction.

## Boundary corrections and controls

- Original `DX8MeshRendererClass::Flush` now unbinds selected VB/IB in the Linux CPU path on normal and exceptional exits, preserving authored source cleanup. Null buffer setters release engine refs even without a live edge; non-null binds still require one. The sorting CPU fixture now binds separate bounded negative-control sorting buffers after Flush, while its source-generated sorted triangles, upload/pipeline/draw failure injections, invalid ranges/index/depth and nonfinite-triangle rejection remain exercised.
- A failed Recording draw kept a `PolyRenderTaskClass` mesh reference linked during category invalidation. CPU category destruction now retires the pending task before unregistering its model. The Recording source scene injects that failure, retries a fresh frame, and asserts all three fixture mesh refcounts return to one before release. GCC and Clang ASan+UBSan source-only runs with leak detection enabled pass.
- Original applied pipeline color/depth formats now come from the live bound public targets; stale/unsupported targets fail closed. BGRA8 and RGBA8 physical pixels pass. The public bgfx sampler accepts source maximum LOD 0 only for a one-mip sampled texture; unsupported LOD 1 and stale/uninitialized resources remain rejected. Renderer Vulkan contract positive/negative and original source draw tests pass.

## Validation

- Final diagnostics-clean Clang ASan+UBSan source-scene run with `VK_INSTANCE_LAYERS=VK_LAYER_KHRONOS_validation` passed 30/30 fresh processes under a 120-second bound per process. Every process exercised all four format/extent cases, original rigid/static pixels, source teardown and two or more fresh device generations. The `run_validation_clean.py` CTest wrapper passed with no `Validation Error` or `VUID-` output. Prior 30/30 pre-repair and 30/30 post-repair repeats were diagnostic context, not substitutes for the final gate.
- GCC and Clang sanitized focused original CPU graph, static-sort failure, sorting, GPU-edge failure, and Recording source-scene tests passed. GCC and Clang leak-capable Recording source-scene controls passed outside the sandbox's ptrace restriction. Public bgfx device-contract and Vulkan resource tests passed, including one-mip LOD 0 pixels and unsupported-LOD rejection.
- All four GCC/Clang Debug/Release preset builds passed. In each preset, the final canonical asset-free CTest suite passed 179/179 with host loopback permission; the intermediate 174/174 non-ledger/non-LAN suites, four LAN pairs and three ledger checks also passed separately. Dependency-ledger drift from this slice and prior A0 allocation work was corrected. `git diff --check` passes.

The earlier unresolved blocker record is historical diagnostic evidence. It is superseded by the allocator prerequisites and this source-scene acceptance. No retail symlink or corpus content was read for this slice or modified.
