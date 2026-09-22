# M22 plan 01 slice 07E0A: original empty terrain render owner

## Outcome and dependency

Requires accepted 07D. The canonical original `BaseHeightMapRenderObjClass`/`HeightMapRenderObjClass` source owns an empty, no-map terrain render object and publishes the existing `TheTerrainRenderObject` pointer only while that object is alive. This is the smallest terrain prerequisite for native `W3DView::update`: the reached terrain calls use the base pointer's `doesNeedFullUpdate()` and `updateCenter()`, not W3DTerrainVisual-specific behavior. The object remains unpopulated and makes no terrain pixels. It must not claim map loading, texture/tile rendering, shroud, water, tracks, W3DTerrainVisual factory, tactical update, or production display draw.

## Entry, state, errors and surfaces

Under an active original GPU edge and initialized W3DDisplay owners, create one original HeightMapRenderObjClass, establish the singleton with explicit ownership, verify `getMap()==NULL` and no full update, then release it before WW3D shutdown. Repeat across reset and a second edge/display generation. Reject duplicate publication, no edge, stale owner and attempts to render, load a map, or perform unsupported terrain operations. The failed operation must leave the singleton and source scene recoverable; destruction clears only its own publication and must not clear a successor's pointer. This needs a real source-derived class, not a test surrogate or new interface layout.

The dependency surface is `BaseHeightMap.cpp`, `HeightMap.cpp` and existing headers, full-draw target linkage, source/physical probes as appropriate, and ledger identity. Preserve native Windows branches. Do not modify the retail symlink or unrelated renderer diagnostics. The original abstract base and concrete derived classes need their virtual tables for a real owner; map-dependent virtuals not reached by this empty baseline must reject with typed errors, while empty resource/update operations may be explicit no-ops. No silent success stub or unrelated terrain behavior counts as acceptance.

## Acceptance and commit

Positive/negative tests prove original class identity, null-map/empty update, publication/duplicate/no-edge rejection, reset/re-entry and teardown/reference lifetime. Provider-removal, GCC/Clang sanitizer and complete mandated asset-free suites pass; any physical bgfx test verifies unchanged clear-only pixels and explicit Vulkan diagnostic rejection. Evidence records the exact native source call sites and the no-map boundary. One plan/source/tests/evidence commit. 07E remains pending.
