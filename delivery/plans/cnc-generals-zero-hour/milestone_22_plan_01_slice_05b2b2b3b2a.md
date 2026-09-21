# M22 plan 01 slice 05B2B2B3B2A: original source-issued physical binding descriptor

## Outcome and dependencies

Requires accepted B3B1. Consume its applied original shader/material/UV/FVF state and original TextureClass/TextureFilter stage owners to construct public Recording shader, pipeline, uniform and stage-binding descriptors, without an adapter-selected material/texture/draw/pass. This slice does not produce Vulkan pixels. No output from this slice may be treated as a substitute original category pass; 06 owns authored category scheduling and world/light issuance. Every resource and decision is tied to its original owner and active device generation.

## Source-to-Recording boundary

Define a stable, bounded uniform ABI for the accepted independent stage color/alpha operators and arguments, original ARGB diffuse/material/fog/alpha-test, UV source/transform/bump, and original depth/blend/cull pipeline bits. Source-issued unlit state is a positive; source-issued lit state remains semantically preserved but physical lowering rejects before GPU resource/cache mutation pending 06. Bind exactly zero, one or two source-required texture stages with original TextureClass handles and original filter sampler; no generated missing pixel/dummy binding. Preserve state across source application to the later real draw, including invalidation on TextureClass destruction/device generation change. The adapter may require original source transform issuance but may not synthesize category world/light values.

## Acceptance, negatives and commit

Owned original methods issue shader/material/texture/filter/mapper commands, then Recording assertions inspect exact pipeline/uniform/binding bytes and resource counts; check each semantic op/arg, color-vs-alpha, alpha/blend/depth/cull/fog/material/UV and source fallback. Reject lit physical requests, invalid FVF/stage, missing or stale source texture/filter, wrong device generation, unsupported required variant and injected shader/pipeline/uniform/upload errors before leaving cache resources or corrupting source replay. Confirm no dummy pass/draw or source reordering, original ABI/provider-removal and dependency ledger, GCC/Clang full suites, focused sanitizers, retail family aggregate without private names or bytes, and `git diff --check`. Commit one independently tested slice. B3B2B must execute this exact ABI with SDL_GPU Vulkan pixels; 06–09 full acceptance remains mandatory.
