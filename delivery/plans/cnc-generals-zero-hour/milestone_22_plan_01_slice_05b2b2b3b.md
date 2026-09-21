# M22 plan 01 slice 05B2B2B3B: lower original pending shader state

## Outcome and dependencies

Requires accepted 05B2B2B3A. Original `ShaderClass::Apply`, mapper/material decisions, texture stage/filter, and `DX8Wrapper` ref-counted delayed state drive public Recording and SDL_GPU Vulkan shaders, pipeline and uniform data. Do not insert a dummy pass/draw, select geometry/material authority in an adapter, rearrange source commands or claim a scene: original interleaved category draw applies pending state in 06.

## Exact source-to-device translation

Build owned shader sources and public pipeline/uniform descriptors from original pending blend/depth/cull, fog, material/lighting/alpha-test, UV transform and stage/sampler decisions. Lower every B2-advertised bounded combiner operation (DISABLE, SELECTARG1/2, MODULATE, ADD) with its original source arguments and alpha/color paths; use a bounded uniform-driven interpreter if it is exact and public SDL_GPU constraints permit it. Pipeline vertex input uses B3A original FVF layout. Unsupported required retail variants or combinations must reject explicitly before partial cache mutation and be escalated, never substituted with existing heuristic `world.frag` or skipped. SDL_GPU and Recording must agree on uniform values, resource generations, pipeline descriptor and stage persistence until the actual original draw. Test authored fallback/unsupported capability decisions and shader compilation/creation failure reset/replay.

## Acceptance and negatives

Owned original shader/material/mesh fixtures drive source-selected state into Recording, with positive and negative each permitted op/argument, alpha blending/test, depth/cull, fog/material and UV semantics. Validation-layer SDL_GPU Vulkan compiles/executes matching test draw(s) with actual indexed B3A layouts, zero Validation Error/VUID, and pixel/uniform comparisons sufficient to catch wrong blend/combiner/state. Probe read-only retail required variant aggregates without committing private names/paths/bytes/hashes. Reject unsupported combinations, missing required shaders/materials, wrong stage and injected shader/pipeline/uniform failures; verify generation invalidation/recreation and bounded teardown. No fabricated pass or retail scene claim. Run provider-removal, unmixed ABI, original branch tests, GCC/Clang full suites, focused sanitizers and ledger. 06 owns original category Render, interleaved pass/draw and pass-time binding failures; 07–09 retain GameClient, retail recording, Vulkan visual/full acceptance.

## Commit boundary

One independently validated commit: `delivery: M22 slice 05B2B2B3B lower original shader state`.
