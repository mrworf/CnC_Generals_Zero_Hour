# M22 08F post-08F4 generated discovery: no-model prop prerequisite

Parent commit: `9fcb7167d6a0a347053f5d912d451d34396cd2e8` (accepted 08F4).
This is a plan-only checkpoint, not 08F or 08F5 implementation acceptance.
No retail provider/content was opened; no private retail identifiers, paths,
bytes, hashes, images, or raw process output are recorded. The unrelated
renderer diagnostic remains unstaged.

A generated read-only mission fixture combined the existing authored logical
map, flat visual blend metadata and terrain texture. The diagnostic-only
separate construction selector passed accepted 08F0 parser completion, then
native-ordered terrain attach, water attach, disabled grid and accepted 08F3
map override. Fixed generated stage categories confirmed radar new-map,
accepted 08F4 partition shroud refresh, terrain logic new-map, radar terrain
refresh, pathfinder new-map and permanent-observer reveal. The map-object loop
then reached its generated no-draw prop and the CPU `W3DTerrainVisual::addProp`
typed-pending method. Source native `addProp` selects a draw model and only
calls the terrain prop buffer when the model name is nonempty. The generated
fixture's no-draw model is thus a source-equivalent no-op, but the method and
its owner checks require independent [08F5](../../plans/cnc-generals-zero-hour/milestone_22_plan_01_slice_08f5.md)
acceptance. Removing the prop from the fixture would conceal the source
object-loop boundary, so 08F remains pending.

The first generated probe briefly found a shroud/partition extent mismatch:
the authored 8x8 vertex map has a 7x7 playable boundary, and a 10-unit cell
size placed the 70-unit world extent exactly on the partition reciprocal-ceil
rounding edge. Partition counted 8 cells while shroud counted 7. A generated
12-unit cell size gave both 6 cells and passed the accepted shroud owner;
this was fixture correction only. Subsequent failure at `addProp` returned
through normal graphics rollback with zero published owners and zero
Recording resources. The existing process-pool baseline delta is not used as
an owner assertion. All trial production/test code was removed before this
checkpoint; no 08F selector or water/object transition was committed.

Checkpoint gates: generated GCC Debug probe localized the source call;
restored-source GCC Debug probe rebuilt and the accepted
`original_w3d_generated_scene_boundary` regression passed (1/1);
`git diff --check`, exact post-cleanup status and plan/index diff were
reviewed. No broad build/test matrix is claimed because this checkpoint
changes documentation only.
