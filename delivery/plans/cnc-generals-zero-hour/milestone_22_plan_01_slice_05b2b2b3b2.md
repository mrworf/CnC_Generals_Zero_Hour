# M22 plan 01 slice 05B2B2B3B2: public GPU shader lowering

## Outcome and dependencies

Requires accepted B3B1. Translate only original applied shader/material/texture/filter state into the public `GpuDevice` shader, pipeline, stage/sampler and uniform resources for Recording and SDL_GPU Vulkan. Use B3A canonical FVF layout without repacking. No adapter-authored scene, synthetic required pixels, dummy category pass or source call reordering. B3B1 source semantics and original producer identity are unchanged; 06 owns the actual original category pass/draw and category-issued transform/light state.

## Physical and shader contract

Compile owned GLSL/SPIR-V variants or a bounded uniform-driven interpreter for each B3B1-accepted original operator and argument, independent color/alpha stages, original ARGB material/diffuse, authored fog, alpha test, blend/depth/cull and UV transforms/coordinates. Public sampler/stage binding must match original `TextureClass` lifetimes/generation and remain pending until the original 06 draw. Reject unsupported required retail variants or lighting routes clearly rather than replacing them with a generic shader or skipped frame; category-owned light/world emission is a 06 prerequisite before a lit retail draw. Resource creation/upload failures unwind and allow exact source-state replay, no stale cache after generation change/recreation.

## Acceptance and negatives

Owned source-issued fixtures produce Recording pipeline/uniform/stage descriptors and public indexed validation-layer SDL_GPU Vulkan device-lowering probes; compare pixel and uniform outcomes for each permitted combiner op/argument and required material/blend/depth/cull/fog/alpha/UV family. Test wrong layout/stage/texture, unsupported variant, injected shader/pipeline/uniform/upload errors and source teardown/retry. Explicit `VK_LAYER_KHRONOS_validation` reports zero Validation Error/VUID. These probes prove physical state lowering only, never original category scheduling or retail frames. Include read-only retail shader family inventory, provider-removal, ABI/link identity, ledger, GCC/Clang full suites, focused sanitizers and bounded recreation/teardown. Slice 06 then proves original category-issued light/world and interleaved draw, 07–09 full GameClient, retail recording and final Vulkan/visual acceptance unchanged.

## Commit boundary

One independently validated commit: `delivery: M22 slice 05B2B2B3B2 execute original shader state`.
