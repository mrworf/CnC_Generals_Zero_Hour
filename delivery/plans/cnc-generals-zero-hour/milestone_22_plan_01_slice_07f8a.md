# M22 plan 01 slice 07F8A: terrain scene-registration prerequisite

## Outcome and dependency

Requires 07F8. Restore the smallest source-equivalent map-capable terrain
render-object attachment contract before visual loading: map-derived bounds,
the rigid original 3D-scene `Notify_Added` update registration, and symmetric
detach/re-entry cleanup. This is a reusable terrain-owner prerequisite for
07F9, rather than a `W3DTerrainVisual` special case. It remains generated
Recording-only and does not render or claim pixels.

## Source boundary

The native `BaseHeightMapRenderObjClass` derives its sphere and box from the
bound `WorldHeightMap`, publishes the base render-object scene pointer, and
registers `ON_FRAME_UPDATE` when attached. Preserve that generic `SceneClass`
contract; the later visual owner remains responsible for supplying the primary
original 3D scene. Reject absent, mismatched, duplicate, and stale-scene
attachments before state publication. Detach unregisters before clearing the
scene pointer so no stale frame callback remains. Do not implement terrain
LOD, shroud drawing, view traversal, water, tracks, shadows, or any draw.

## Acceptance

Generated-map tests prove exact map-derived bounds; attach registers the
original terrain exactly once; detach unregisters and clears ownership;
duplicate/mismatched attachment and mismatched detach reject without corrupting
the accepted attachment; and a fresh second generation leaves no scene or
Recording resources. Source identity, provider removal, ledger and canonical
sanitizer controls remain required. 07F9 is not started until this prerequisite
is accepted.
