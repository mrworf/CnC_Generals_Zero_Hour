# M22 plan 01 slice 07E0B: original W3DTerrainVisual empty ownership

## Outcome, dependency and boundary

Requires accepted 07E0A and its owner prerequisites 07E0B1–B5. The canonical original `W3DTerrainVisual` owns and publishes one empty `HeightMapRenderObjClass` through its existing `m_terrainRenderObject` and `TheTerrainRenderObject` edges during `init`, `reset`, `update`, and destruction. GameClient's original order creates Display before TerrainVisual, then destroys TerrainVisual before Display; the owned object must be gone before WW3D shutdown. This remains a no-map owner, not terrain tiles/pixels, effects rendering, map load, tactical update, factory publication, or display draw.

Native `W3DTerrainVisual::init()` unconditionally constructs track, shadow, water and smudge owners and attaches the water object to the original 3D scene even when `GlobalData` defaults to zero terrain tracks and disabled shadow volumes/decals and water plane. The Linux empty-startup dependency chain is therefore 07E0B1 no-shadow manager lifecycle, 07E0B2 zero-track owner, 07E0B3 no-water object, 07E0B4 no-smudge owner and 07E0B5 disabled-water scene membership, followed by visual composition in 07E0B. Each child must keep enabled/map-dependent draw paths typed pending and demonstrate that its default-empty state has no physical output. If an owner proves unnecessary to empty startup by source reachability, document that before dropping or reordering a child; do not silently skip native init work.

## Entry, state, failure and surfaces

In an initialized GameClient scenario, under an active original device edge and initialized W3DDisplay, instantiate and initialize W3DTerrainVisual directly. The visual owns one terrain ref, publishes the base/derived singleton identities and keeps them stable across idempotent init, empty reset and update. Reject no-edge/no-display, second visual, `load`, map-dependent or water/track/shadow operations without publishing a replacement or mutating a frame. Destruction clears the singleton before display teardown. Repeat over reset and independent device/display generations. Do not alter original class layout or native Windows branch; use typed rejection for unsupported virtual methods, not silent success.

Surfaces are canonical W3DTerrainVisual.cpp and its required original class dependencies, full-draw target, direct source/physical fixture, provider/link identity, ledger, evidence. The existing Linux GameClient factory remains unchanged.

## Acceptance and commit

Recording positive/negative owner tests cover class identity, singleton lifetime, duplicate/no-edge/stale-display rejection, idempotent init, reset/update, explicit unsupported map/load operation, and no leaked refs or scene draws. Existing empty/rigid W3DView bgfx frames remain byte/pixel-consistent across two formats, two extents, repeated Khronos generations. GCC/Clang ASan+UBSan source controls, provider-removal, dependency ledger, five fully rebuilt non-GPU suites and repeated physical validation pass. One plan/source/tests/evidence commit. 07E draw/factory remain pending.
