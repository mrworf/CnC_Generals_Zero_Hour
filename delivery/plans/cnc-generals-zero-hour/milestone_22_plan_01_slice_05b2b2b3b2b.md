# M22 plan 01 slice 05B2B2B3B2B: original applied shader GPU execution

## Outcome and dependencies

Requires accepted B3B2A. Compile bounded owned GLSL/SPIR-V to execute the same original source-issued shader/material/texture/filter/FVF uniform ABI, with exact zero-, one- and two-required-stage variants (no artificial sampler or generated texture). Carry independent color/alpha operations and DIFFUSE/CURRENT/TEXTURE args, authored ARGB material/fog/alpha-test, original UV indices/transforms and pipeline blend/depth/cull. Lighting remains physically unavailable until original category issuance in 06, though semantic mapping preserves original lit state. Do not reorder source calls or create an adapter/category draw.

## Acceptance, negatives and commit

Use source-issued unlit original fixtures for public indexed SDL_GPU Vulkan lowering probes; Recording and SDL observe the same descriptors/uniform bytes and pixels for every permitted combiner op/argument and the reached material/blend/depth/cull/fog/alpha/UV families. Test zero-, one-, two-stage binding, wrong layout/stage/owner/generation, unsupported variants, shader/pipeline/uniform/upload errors and replay, source teardown, two generations/resize and bounded resource release. Explicit `VK_LAYER_KHRONOS_validation` must report zero Validation Error/VUID. A device-lowering probe is **not** an original category frame: 06 must prove source-issued world/light state and actual authored category draw/interleaving, 07–09 own GameClient, retail recording and scene Vulkan/visual acceptance. Verify retail shader family aggregate read-only, provider-removal, ABI/link identity, ledger, GCC/Clang full suites, focused sanitizers and diff check; commit one independently tested slice.
