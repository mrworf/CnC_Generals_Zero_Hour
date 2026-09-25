# M22 plan 01 slice 08L3: generated bridge and wall attachment aggregate

## Goal and observable outcome

Generated authored bridge-like and walk-on-wall objects reach the native map
bridge phase. A successful attempt publishes their Object/Drawable, terrain
bridge, pathfinder wall/layer and radar state in source order before pathfinder
`newMap`; failure removes only the current attempt's owners and retry is
deterministic. Accepted 08L2 remains the Object/Drawable rollback owner.

## Dependency-ordered implementation slices

1. [08L3A0](milestone_22_plan_01_slice_08l3a0.md) closes the borrowed
   W3DDisplay file-factory publication/unwind required for a modeled generated
   bridge. It does not admit retail scenes.
2. [08L3A](milestone_22_plan_01_slice_08l3a.md) closes `Bridge(Object*)`
   tower creation, missing-provider and partial-constructor rollback, then
   atomic terrain publication and pathfinder bridge-layer acquisition/removal.
   Its direct generated owner witness must pass before map loop changes.
3. [08L3B0](milestone_22_plan_01_slice_08l3b0.md) closes capacity-signaling
   wall admission and fresh pathfinder derived-map allocation/readiness
   rollback without clearing accepted 08L3A bridge-layer identities.
4. [08L3B](milestone_22_plan_01_slice_08l3b.md) composes the accepted owners
   with map-object traversal, property updates, radar queue restoration and
   pathfinder `newMap`. Its generated map witness and complete M22 gates close
   this aggregate.

The original single 08L3 commit boundary is superseded by four coherent
implementation commits. The governing plan records aggregate completion only
after 08L3B acceptance. Later ordinary-object, preload, camera and UI stages
remain outside this aggregate.
