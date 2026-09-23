# M22 plan 01 slice 07FC: active original water-map route

## Outcome and dependency

Requires 07F9. Implement the source map-loaded `WaterRenderObjClass` route
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
