# M22 plan 01 slice 07E0D: local original display frame

## Outcome and dependency

Requires accepted 07E0C. Under a temporarily published original display, original tactical W3DView and no-map W3DTerrainVisual owner graph, implement the canonical Linux `W3DDisplay::draw` transaction for one empty or one generated rigid original MeshClass scene. It performs a guarded black-clear WW3D begin, original view dispatch and WW3D end with a public bound color/depth target. The normal runtime GameClient factory remains unchanged; production publication is a later atomic child.

## Entry, state and failure

The method checks active edge, initialized original display/scene/asset owners, exact one published original tactical view and camera, original no-map terrain/effects baseline, inactive WW3D frame, matching nonzero bound target extent and absence of unsupported display modes before beginning. It calls the source no-map tactical update, then `WW3D::Begin_Render`, `Display::drawViews` and `WW3D::End_Render(false)`. Any failure leaves the source frame/public pass inactive and owner refs intact so a subsequent call can retry. Reject no edge/target, stale depth, extra/non-original view, missing terrain, active frame and unsupported global modes without a partially submitted draw.

The source and host Vulkan probe calls `W3DDisplay::draw` directly with published owners, verifies black empty output, a generated rigid object's changed pixels and detached black return, checks Recording draw injection and retry, and repeats across BGRA8/RGBA8, two extents and fresh generations. Native Windows branch and class layout remain unchanged. Terrain tiles, effects, shroud, tracks, shadows, UI overlay, movies and factory switch remain typed pending.

## Acceptance and commit

Positive/negative source and physical controls, five rebuilt full non-GPU suites including leak-capable GCC/Clang ASan+UBSan, provider/ledger identity and repeated host Khronos Vulkan validation pass. One independent plan/source/tests/evidence commit. Only then consider atomic factory publication as a separate child.
