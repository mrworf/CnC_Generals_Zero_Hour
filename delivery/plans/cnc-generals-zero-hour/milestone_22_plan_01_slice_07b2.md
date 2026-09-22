# M22 plan 01 slice 07B2: original 2D status-circle physical frame

## Outcome and boundary

Requires accepted 07B1, 07B2A and 07B2B. Translate the guarded `RTS2DScene::draw` body to its authored `WW3D::Render(this,m_camera)` path when a physical edge and camera exist; preserve typed pre-frame rejection otherwise. Use a distinct original-CPU-layout probe TU invoked from an initialized full GameClient scenario so original `TheGameLogic` and `TheScriptEngine` own the source state, rather than casting native GameClient material layouts or constructing proxy status geometry. The probe owns public Recording/bgfx targets and a bounded WW3D/scene frame; production factory promotion, full fade rendering, 3D scene, terrain, shroud, tracks, shadows and effects remain later 07 slices.

## Positive, negative and gate

With authored team dot enabled and a non-shell game mode, original `W3DStatusCircle::Render` must allocate and upload its original VB/IB, submit a public draw and yield non-background pixels on validation-layer Vulkan at two extents/formats and two device generations. Team-dot-disabled or shell-mode controls must omit its draw and leave the background; missing camera, stale target and injected upload/draw failure must reject and recover after reset/rebind. The source subtract-fade `REVSUBTRACT` state must reject at the CPU DX8 boundary before mutation; complete source fade support remains a later 07 slice. Preserve source owner refs and bounded teardown. Run GCC/Clang Recording and physical gates, source-only sanitizers, four full asset-free suites, identity/ABI/provider and ledger checks. One independent plan/code/test/evidence commit; do not claim full display or retail scene acceptance.
