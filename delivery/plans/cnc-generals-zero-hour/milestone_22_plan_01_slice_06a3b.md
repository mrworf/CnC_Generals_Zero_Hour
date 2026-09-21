# M22 plan 01 slice 06A3B: exact original lit GPU lowering

Delivered as [06A3B1](milestone_22_plan_01_slice_06a3b1.md) → [06A3B2](milestone_22_plan_01_slice_06a3b2.md). This is an aggregate gate, not an independently committable slice; B1 retains physical lit-draw rejection and B2 completes all acceptance below. Original source owns selection, defaults and pass order; the device edge translates them, without an independent lighting policy.

## Dependency and outcome

Requires accepted 06A3A. Device edge translates only the original bounded light snapshot and already-applied source material/render state to exact normal-bearing FVF vertex shader inputs. Preserve D3D world/view normal transformation and authored normalization, ambient/diffuse/emissive/specular/material-source precedence, diffuse-vertex color, opacity and stage/fog/blend behavior. Required positive exact FVF include read-only retail 274 (`DX8_FVF_XYZNUV1`) plus owned `DX8_FVF_XYZN`/`DX8_FVF_XYZNUV2`, with others only when source reachability proves them. Unsupported FVF/lighting/state/light combinations fail explicitly before physical cache mutation. Direct original source methods may issue a lit GPU probe with test-owned geometry/material/environment; this does **not** claim source category draw.

## Validation and commit

Recording descriptor and validation-enabled SDL_GPU Vulkan pixels compare authored zero/one/four directional and point lights, source materials (including non-white and source color), normals/world transforms, opacity/alpha and supported texture stages. Negative missing environment, invalid light, unsupported FVF, buffer/upload/draw failure, owner/generation/recreation and no Validation Error/VUID. GCC/Clang cumulative suites, identity/provider-removal, focused sanitizers/ledger; one commit. C remains mandatory for category issuance.
