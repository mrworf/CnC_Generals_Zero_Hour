# M22 plan 01 slice 06A2: original unlit rigid/category interleaving

## Dependency and outcome

Requires accepted 06A1. Restore original `DX8RigidFVFCategoryContainer::Render` and the **same** `DX8TextureCategoryClass::Render` source body with guarded physical calls only at device boundaries. Keep source pass/category/task traversal; source texture/material/shader binding; source mesh identity/explicit world and camera-facing selection, scale/normalization and alpha/additive/material/UV override order; original polygon renderer then calls accepted A1 draw. An owned truly unlit W3D fixture goes through original insertion/category, source pass and public Recording indexed draw. Explicit lit source branch remains typed unavailable until 06A3; do not construct a second CPU category renderer, precompute pass graph or let an adapter invent a stage/material/mesh decision.

## Validation and commit

Witness original source order from mesh/category through unlit 0/1/2-stage draw, identity and nonidentity world, alpha and material/UV override positive and negatives, failure/retry at actual pass/draw, source refs and bounded reset. Preserve original lit semantics without silent unlit coercion. GCC/Clang cumulative suites, ABI/link/provider-removal and focused sanitizers/ledger pass before one commit. 06A3 completes the aggregate rigid gate; no GameClient or retail frame claim.
