# M22 plan 01 slice 06A3B2: exact lit Vulkan shader execution

## Dependency and outcome

Requires accepted 06A3B1. Translate the original source-issued semantic record to bounded exact shader/uniform variants keyed to original normal-bearing FVF layouts (retail 274 XYZN+UV1, owned XYZN and XYZN+UV2), original 0/1/2 stage state and source light/material selectors. Inverse-transpose world-view normal transformation, authored normalize/local-viewer mode, directional and point attenuation/range, ambient/diffuse/specular/emissive material terms, source vertex color fallback, opacity, fog, texture combiners, blend and depth must be faithful. The GPU edge must reject unsupported required states/FVF/light variants before cache mutation; no heuristic or invented pixels. Direct original source methods may exercise owned GPU fixtures but cannot claim category-issued lit rendering before C.

## Validation and commit

Validation-enabled SDL_GPU Vulkan pixel probes compare source-authored zero/one/four directional and point lights, material/source colors, nonidentity and nonuniform normal transforms, specular/normalization selectors, alpha and zero/one/two stages. Include unsupported-state/FVF/range/slot, owner/generation/recreation, upload/draw failures, teardown and zero Validation Error/VUID; Recording/SDL parity. Cumulative GCC/Clang suites, identity/provider-removal, focused sanitizers, updated ledger/evidence, one commit. Only B2 completes aggregate B; C still must prove original category-issued order.
