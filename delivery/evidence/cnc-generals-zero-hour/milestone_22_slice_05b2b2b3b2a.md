# M22 slice 05B2B2B3B2A: source-issued Recording physical bindings

## Producer and boundary

The original `DX8Wrapper`, `ShaderClass`, `VertexMaterialClass`, `TextureClass` and `TextureFilterClass` still issue the decisions and bytes. The CPU-only original wrapper now accepts its native `D3DTS_WORLD=256` transform alongside view/projection. `OriginalGpuEdge::prepare_applied_state` requires all three source-issued transforms, the applied B3B1 material/shader/FVF state, and exactly the selected resident TextureClass/filter owners. It emits bounded public shader/pipeline/uniform and zero-, one- or two-stage Recording bindings, with source ARGB/material, fog/alpha, independent color/alpha combiner and UV/transform/bump values encoded in a stable std140-compatible ABI. It creates **no pass or draw** and supplies no transform, texture, material or pixel of its own. The fragment shader names and uniform ABI are B3B2B GLSL obligations; this slice has no SDL/Vulkan pixel claim.

The selected original state is associated with a device generation, monotonic source-state revision and physical serial. A newly prepared state releases the prior bounded shader/pipeline/buffers; invalid original texture owner, sampler, filter, source mutation, forged uniform binding, stale serial, or another device generation fails validation before any actual draw. A source-issued lit semantic request is preserved by B3B1 but rejected by this physical preparation pending 06 category light issuance. Creation/upload errors release partial physical resources without poisoning exact source-state retry.

## Positive and negative witnesses

- Original `ShaderClass` unlit/untextured, original material and explicit original WORLD/VIEW/PROJECTION methods issue a zero-stage pipeline. Recording verifies canonical source FVF, depth/blend bits, two exact uploaded uniform byte structures, identity matrix bytes, no dummy pass/draw, four injected shader/pipeline/buffer/upload failures, clean rollback and successful replay. Missing transform, lit physical request, tampered binding, superseded state and another device generation reject.
- Owned original texture loader and `TextureClass::Apply`/filter methods supply resident one- and two-stage source textures with distinct source samplers. Recording checks exact handle/order, all five permitted color/alpha operations times three arguments at the uniform byte boundary, unsupported missing stage/UV, source changes, retry and texture owner invalidation. No fallback pixel is used as a required retail success.
- Existing read-only retail model/source-FVF aggregate and full-draw route still pass. The later retail shader-family inventory and full campaign/skirmish scenes remain 08/09. No private retail bytes, filenames, host paths or hashes were written to evidence; original symlink remains untouched.

## Validation

- GCC and Clang Debug full builds, **146/146 non-LAN and 4/4 LAN tests per toolchain**, including source ABI, original providers and ledger; one additional last-generation focused test passes on both toolchains after the full suites.
- GCC and Clang focused ASan/UBSan/LSan source shader, original texture decisions and read-only retail full-draw scenario **3/3 each**; final resource-cleanup refinement included.
- Dependency source SHA-256 updated, `git diff --check` passes. B3B2B must compile and execute these exact shader names/uniform/stage contracts under explicit Vulkan validation; 06–09 still own original authored draw, GameClient, retail recording and final visuals.
