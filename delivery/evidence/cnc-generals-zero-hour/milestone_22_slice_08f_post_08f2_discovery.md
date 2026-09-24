# M22 08F post-08F2 discovery: water override prerequisite

Parent commit: `34147fc86f220cb55543fb8aede67961f01deb24` (accepted 08F2).
This is a source-only, generated-safe plan checkpoint, not 08F or 08F3
implementation acceptance. No retail provider or content was opened; no
private identifiers, paths, bytes, hashes, images, or raw process output were
recorded. No 08F trial production/test code was introduced or retained in
this checkpoint. The unrelated renderer test edit remains unstaged.

The source call chain after accepted 08F0 `loadMapINI` is
`GameLogic::startNewGame` → `LinuxTerrainLogic::loadMap` →
`TerrainLogic::loadMap` → `W3DTerrainVisual::load`. Native terrain load adds
the terrain render object, then adds its water render object to the primary
scene, calls `WaterRenderObjClass::enableWaterGrid(FALSE)`, and calls
`WaterRenderObjClass::updateMapOverrides`. CPU terrain load currently returns
after adding terrain, before those water calls. Accepted 08D reset detaches
water from the scene, while the bounded active-water setter requires exact
scene membership; therefore 08F must implement source-order reattachment
before the disabled setter and prove rollback. Reattachment remains out of
08F3.

The native `WaterRenderObjClass::updateMapOverrides` changes an already-owned
river texture only if its name differs from the parsed standing-water setting.
The CPU branch has no definition of that method, and its bounded water owner
does not publish a river texture. This missing source-owner method is the
independent [08F3 prerequisite](../../plans/cnc-generals-zero-hour/milestone_22_plan_01_slice_08f3.md):
owner-checked no-river result; broken owner/provider, detached scene, pending
resources, and any non-null river texture fail closed. It does not add river
texture support or admit the 08F selector. After 08F3 acceptance, 08F may
resume the generated source chain and locate any subsequent prerequisite.

Read-only checks: source definitions and guards inspected; worktree had only
the unrelated renderer test edit before these plan files; no construction
selector production/test wiring exists. Full acceptance is deferred because
this checkpoint changes no production or test code.
