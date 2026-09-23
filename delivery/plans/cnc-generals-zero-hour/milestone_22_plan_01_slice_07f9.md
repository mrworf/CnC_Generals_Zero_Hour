# M22 plan 01 slice 07F9: transactional map-loaded terrain visual

## Outcome and dependency

Requires 07F8A and the accepted empty `W3DTerrainVisual` owner chain. Implement
the smallest source-equivalent `W3DTerrainVisual::load` transaction using a
generated, read-only map: construct the source `WorldHeightMap`, initialise the
existing `HeightMapRenderObjClass`, attach it to the original 3D scene, and
reverse every publication on a malformed map, missing source tile, failed
buffer operation, detach, reset, and second generation. Texture-atlas creation
is lazy at the existing 07F8 rendering boundary, so its injected failure stays
there rather than claiming map-load rollback. This is the
first map-loaded owner route; it does not accept retail content or physical
pixels.

## Source boundary

Native `W3DTerrainVisual::load` performs the map stream transaction before
`initHeightData` and scene attachment. Preserve that owner order; map-object
and light iteration are deliberately deferred with the inactive scene-content
branches. Begin with a no-map-object, no-water, no-track, no-shadow generated
profile. Do not bypass `TerrainVisual`, synthesize terrain geometry, or make
the visual's map state successful after a failed child.

## Acceptance

Recording tests cover one loaded flat map, exact terrain attachment and the
07F8 two-pass submission, malformed/missing tile and injected buffer
failures, reset/detach/reload, and zero owners/resources. Identity,
provider-removal, ledger and normal/sanitized suites remain required. Active
shroud, tracks, water, shadows, particles/effects, tactical update and retail
inputs remain later children.
