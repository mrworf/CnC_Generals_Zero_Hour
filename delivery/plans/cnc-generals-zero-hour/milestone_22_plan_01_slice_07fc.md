# M22 plan 01 slice 07FC: active original water-map route

## Outcome and dependency

Requires 07FA. Implement the source map-loaded `WaterRenderObjClass` route
needed when the original visual's map configuration enables water: load/map
override, scene membership, grid state, source render ordering and
release/reacquire. The accepted no-water owner is only a bootstrap
prerequisite, not evidence for a map with water.

## Boundaries and acceptance

Drive one owned generated water-enabled map through the original terrain
visual and water owner. Record its source resources/state/draws and prove
disabled-water absence, malformed/unsupported water configuration, injected
resource/draw failure, reset/reload and zero teardown. Preserve original
water type/grid decisions; sky, cloud/reflection variants and Vulkan pixels
remain separately guarded until they are reached and planned.

## Implemented slice decision

The CPU route admits exactly one `WATER_TYPE_0_TRANSLUCENT` plane owned by the
published terrain visual in the primary RTS3D scene. It uses a four-vertex,
six-index source quad after the terrain and optional accepted terrain-track
flush. Grid mutation, cloud/reflection variants, shadows and particles remain
outside this slice. Recording evidence covers malformed active configuration,
resource/draw failure rollback, retry, two generations, and zero resources.
