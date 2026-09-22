# M22 plan 01 slice 07B2A: original preset material CPU lifecycle

## Outcome and boundary

Requires accepted 07B1. Native `DX8Wrapper::Init` initializes `VertexMaterialClass` presets before original status-circle rendering, and native `DX8Wrapper::Shutdown` releases them. The Linux CPU-only WW3D path omits both wrapper calls; first `W3DStatusCircle::Render` dereferences an absent `PRELIT_DIFFUSE` preset. Restore only this original preset lifecycle at the enclosing CPU `WW3D::Init`/`Shutdown` boundary, preserving class layout and native branches. This prerequisite does not accept physical status pixels or full 07B2.

## Positive, negative and gate

With an active Recording edge, obtain/release the authored `PRELIT_DIFFUSE` preset after WW3D init, shut down, and repeat in a second generation without leaked refs or buffers. A no-edge full init must reject before source initialization. The status-scene probe's first-frame null-preset crash is the before-fix negative. Run GCC/Clang focused source and sanitizer tests, four full asset-free suites, source identity/ABI/provider and ledger freshness. One independent plan/source/test/evidence commit. Keep pending 07B2 probe edits separate.
