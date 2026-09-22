# M22 plan 01 slice 07B1: original 2D scene status-circle ownership

## Outcome and boundary

Requires accepted 07A. Translate only the original `RTS2DScene` constructor/destructor ownership path under the Linux CPU ABI: with an active physical `OriginalGpuEdge`, create the canonical `W3DStatusCircle`, attach it to the original `SimpleSceneClass`, then detach and release it on scene teardown. Without an active edge, preserve the existing typed rejection before publishing an owner. Leave `RTS2DScene::draw` guarded until 07B2 provides physical status-circle pixels and fade-state handling. This is not 07 acceptance or a production factory switch.

## Positive, negative and gate

An owned Recording device session constructs the original scene, proves the status-circle object is canonical and attached with exactly the expected refs, destroys the scene, and checks no object/device resource remains. The original no-edge test still rejects before scene/asset publication. Repeat construction/destruction and enforce source ABI, provider identity and schema/full exclusion. Run GCC/Clang Debug and sanitizer focused tests, all four full asset-free suites and ledger freshness. One isolated plan/source/test/evidence commit. No retail writes, class-layout changes or fallback renderer objects.
