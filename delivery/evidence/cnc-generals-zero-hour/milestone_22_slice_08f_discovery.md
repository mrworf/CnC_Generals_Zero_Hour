# M22 08F discovery: post-map prerequisite

Parent commit: `2182969a2f1758fb4cdd2f6f3cae07502f1c7638` (accepted 08F0).
This is a generated-only diagnostic and plan checkpoint, **not** 08F or 08F2
implementation acceptance. No retail provider or content was opened, and no
private names, paths, bytes, hashes, images, or raw process output were
recorded. Exploratory production and test edits were removed before commit;
accepted 08F0 behavior remains unchanged.

The native next call after `loadMapINI` is
`LinuxTerrainLogic::loadMap` → `TerrainLogic::loadMap` →
`W3DTerrainVisual::load`. The first exploratory fixture omitted the accepted
08F0 active-water settings, causing invalid teardown state; restoring those
generated settings corrected it. The pre-existing logical-only generated map
then failed the visual map parser as designed. Supplying generated flat
`BlendTileData`, a generated texture and a positive generated
`PartitionCellSize` let the original terrain owner complete. The positive
partition value is a fixture input, not a production default change.

After terrain load, a diagnostic stop initially exposed a map-loaded
`removeAllBibs` cleanup rejection. Native source clears a bib list; the
generated fixture has no bib producer and the Linux creation APIs remain
typed pending. An exact generated-selector/owner empty-list guard was tested
locally and returned zero published owners and Recording resources, then was
removed with all trial code. This remains selector-local 08F cleanup work;
it is not an active bib implementation, an arbitrary bypass, or proof that a
populated retail bib list works.

Continuing the source construction localized the next failure to
`Radar::newMap` after player and script setup. Its terrain sample calls
`TerrainLogic::isUnderwater` → `getWaterHandle` →
`W3DTerrainVisual::getWaterGridHeight`; that Linux source method currently
rejects every call. The generated map has no water-grid activation waypoint,
and the native method would return `FALSE` without querying a grid. This is
the missing source owner contract assigned to [08F2](../../plans/cnc-generals-zero-hour/milestone_22_plan_01_slice_08f2.md).
Active-grid queries remain unsupported. 08F cannot be accepted or broadly
validated until 08F2 is implemented, independently tested and committed.

Read-only checkpoint: source order and guards inspected; generated-only GCC
Debug diagnostic reached terrain load and localized radar call; exploratory
changes removed; `git diff --check` clean. The unrelated renderer test edit
remains unstaged.
