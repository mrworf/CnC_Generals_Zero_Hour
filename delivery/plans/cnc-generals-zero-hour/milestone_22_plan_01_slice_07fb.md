# M22 plan 01 slice 07FB: active original terrain-track route

## Outcome and dependency

Requires 07F9. Extend the accepted zero-module terrain-track owner to the
smallest source-created active track module, including update/fade, bind,
`HeightMapRenderObjClass::Render` flush ordering, failure rollback and
teardown. This is required before a real scene family can exercise a track;
the zero-module bootstrap does not close that reached render route.

## Boundaries and acceptance

Use an owned generated map and source track definition. Recording must show
the source terrain pass followed by the source track flush, with no draw on
disabled/expired tracks. Missing texture/resource, capacity overflow,
injected upload/draw failure and reload must leave no pending module or
sampler. Do not substitute decals or generated adapter geometry, and do not
claim water, shadows, effects, retail data or physical pixels.
