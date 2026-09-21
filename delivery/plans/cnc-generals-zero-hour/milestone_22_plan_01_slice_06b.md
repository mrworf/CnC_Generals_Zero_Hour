# M22 plan 01 slice 06B: original skin/decal/additional passes

## Dependency and behavior

Requires accepted 06A3C (the full 06A aggregate). Extend original `DX8SkinFVFCategoryContainer::Render`, `DX8MeshRendererClass::Render_Decal_Meshes`, `DX8RigidFVFCategoryContainer::Render_Delayed_Procedural_Material_Passes`, original mesh material-pass and dynamic-buffer owners. Original skin bone/vertex transform, per-pass texture/material/shader, decal depth bias and procedural pass scheduling issue their own draw commands via the same bounded public GPU edge. Preserve preexisting rigid behavior and native source device path; no adapter-owned skinning/decals/procedural scheduling.

## Acceptance

Owned W3D fixture proves skin, decal, additional and delayed procedural pass positive source call order/draws, exact buffer offsets and stage/material state. Negative invalid bones/indices, missing required resource, unsupported reached device state and injected bind/draw failure unwind/retry; no silently skipped pass or partial frame success. Original source identities, provider-removal, full GCC/Clang regression and focused sanitizers/ledger pass. Sorting/static/WWShade/full frame remain 06C. Commit one coherent tested 06B slice.
