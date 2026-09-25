# M22 plan 01 slice 08P0C1A: breeze, cull and visible tree geometry

## Goal and boundary

After 08P0B, own the first native `drawTrees` phase on generated CPU terrain:
breeze-version update, pause, per-type sway interpolation, camera sphere
cull/visible transition and camera-facing sort keys. Preserve source order and
disabled visible sorting. Rebuild bounded visible source geometry from accepted
models/atlas; retain no offscreen geometry in the frame candidate. Keep unit
push/topple, optional projected decals, shroud draw and factory closed.

Preflight matching map/terrain, camera, active generation, source resources
and valid breeze period/finite values. Preview only GameClient RNG when breeze
changes; stage frame state and GPU wrappers, then commit atomically. A failed
cull, geometry/upload or provider change leaves accepted tree/type identity,
prior frame and RNG untouched, with no partial Recording resources. Move,
removal and reset invalidate visible state; clean retry needs no world reset.
Authorization is not applicable; the witness uses generated assets only.

## Validation and commit

Generated positive/negative fixtures cover multiple types/positions, visible
and culled spheres, camera change, breeze version, pause, sway sequence,
move/removal and two generations. Fault each frame/geometry boundary; compare
retry to fresh equivalent state, assert immediate pre-teardown residuals and
owner removal. Run six complete builds and canonical nonretail suites,
focused strict host LSan, physical Vulkan controls, serial LAN 4/4 all six,
ledger and diff checks on final source. Commit one slice:
`delivery: M22 08P0C1A own visible tree frame`.
