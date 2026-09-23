# M22 plan 01 slice 07FB: active original terrain-track route

## Outcome and dependency

Requires accepted 07FA map-frame traversal. Extend the accepted zero-module terrain-track owner to the
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

## Delivered slice

One active source module is admitted only in the published primary RTS3D scene.
It remains pending without a loaded map, acquires bounded native source buffers
after map texture publication, queues before HeightMap flushes it, and releases
its resources on reset and teardown. The Recording probe covers exact buffers
and draw range, singleton/capacity, disabled/expired, create/upload/draw
failure-retry, two generations and zero retained resources. Retail and pixels
remain out of scope.
