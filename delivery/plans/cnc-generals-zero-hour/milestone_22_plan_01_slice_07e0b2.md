# M22 plan 01 slice 07E0B2: original zero-track owner lifecycle

## Outcome and dependency

Requires accepted 07E0B1. The canonical `TerrainTracksRenderObjClassSystem` supports W3DTerrainVisual's default empty-startup owner with `GlobalData::m_maxTerrainTracks == 0`: one initialized scene-bound source system, no track modules, no GPU buffer pending, safe reset/update/flush/resource release/reacquire and teardown before W3DDisplay. Native W3DTerrainVisual::init constructs the system unconditionally even when zero modules are requested. This does not claim active track pixels; the existing nonzero CPU pool/bind route and typed physical flush must remain intact.

## Entry, state and failure

Under an active original edge and initialized W3DDisplay scene, initialize one zero-module system and publish the existing global pointer. Idempotent empty init/reset and two edge/display generations retain no scene ref or GPU allocation. Reject no edge, absent/stale/wrong scene and duplicate publication without displacing the owner. `bindTrack` with zero modules must return no track; `flush` emits no draw. After destruction, both the global pointer and pending-resource state are clear. An enabled nonzero track pool follows the previously accepted source CPU route and still rejects active GPU flush. Preserve native Windows semantics and the original class layout.

Surfaces: original W3DTerrainTracks.cpp, source/physical owner scenario, provider/ledger identity and evidence. Do not alter factory, terrain visual, map data, or retail symlink content.

## Acceptance and commit

Recording positive/negative ownership controls, nonzero-track regression, source view/scene physical pixel consistency under Khronos Vulkan in both formats/extents and repeated generations, GCC/Clang sanitized source, provider-removal/ledger and five rebuilt full non-GPU suites pass. One independent plan/source/tests/evidence commit. 07E0B3/B4, W3DTerrainVisual and 07E remain pending.
