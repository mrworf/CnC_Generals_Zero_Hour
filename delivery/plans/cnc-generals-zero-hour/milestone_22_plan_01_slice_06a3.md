# M22 plan 01 slice 06A3: original category light environment and exact lit FVF

## Dependency and outcome

Requires accepted 06A2. Original `DX8TextureCategoryClass::Render` calls original `DX8Wrapper::Set_Light_Environment` at each reached mesh; preserve authored ambient, up to four directional/point lights, world/view normal transform, material-source fields, specular and opacity in source order. The CPU configuration exposes the source light decision and a bounded physical source-light record to `OriginalGpuEdge`, which executes exact lit shader variants keyed to original `FVFInfoClass` layouts. Current owned category closure includes XYZN and XYZNDUV2 beyond B3B2B's normal and diffuse variants; inspect read-only retail aggregate for additional required families and reject unsupported ones explicitly. No constant white as surrogate for lit source material, no adapter-owned light setup and no synthesized normal/vertex input.

## Validation and commit

Original owned lit W3D fixtures prove category-issued light/world/material precedence and source normal/color, identity/nonidentity, alpha override and exact source FVF indexed pixels/Recording descriptors. Negative absent light environment when required, invalid light/range/state, unsupported FVF/lighting combination, injected bind/draw failure and device-generation retry fail without publication. Validation-enabled Vulkan exact variants report zero Validation Error/VUID; original provider identity, ABI/link, GCC/Clang full tests, focused sanitizers and ledger pass. Commit only after the full 06A aggregate acceptance is satisfied; 06B then begins skin/decal/additional passes.
