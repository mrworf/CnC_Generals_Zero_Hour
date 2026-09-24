# M22 plan 01 slice 08F2: disabled water-grid query prerequisite

## Goal and source boundary

Close the first missing source owner reached after the generated 08F0 parser
boundary and the original terrain load. `GameLogic::startNewGame` calls
`Radar::newMap`, whose terrain sampling calls `TerrainLogic::isUnderwater`,
`getWaterHandle`, then `W3DTerrainVisual::getWaterGridHeight`. The Linux method
is typed pending. In the native source, a disabled grid returns `FALSE`; an
enabled grid queries the water owner. This slice implements only the disabled
result and leaves active-grid sampling fail-closed.

## Scope

Implement the source `W3DTerrainVisual` disabled-grid state/query on the
existing original water owner with explicit published-owner checks. Preserve
native `enableWaterGrid(FALSE)` state only after its underlying call succeeds.
Null, foreign, unpublished, or inconsistent terrain/water owners and enabled
grid requests reject without alias or resource changes. Do not synthesize a
height, grid, polygon water area, radar result, map frame, or retail scene.
The default factory and accepted 08D/08F0 stops remain unchanged.

## Enabled and edge decision

The native getter returns `FALSE` immediately when its grid-enabled flag is
clear, without reading coordinates or writing the caller's height. The Linux
branch will require the live original terrain/display/water aliases and active
original GPU edge (Recording in the generated probe) even for this disabled
result, then leave the output untouched.
Both a scene-attached water owner and the accepted 08D reset-detached owner
are valid for the query; scene attachment is not a query precondition. The
disabled setter still delegates to the water owner, which rejects a detached
owner; 08F must resolve any later map-load reattachment handoff in its own
source transition. The accepted bounded water owner rejects
`enableWaterGrid(TRUE)`, so this slice must preserve the
flag on that failure and reject any inconsistent enabled flag at query time.
There is no active-grid sampling implementation. Missing edge, foreign or
removed provider, and pre-init/retired owner reject before state access.

Use generated read-only inputs. A focused source probe should verify disabled
query returns `FALSE` before and after a native disable call, active-grid
selection rejects, missing owner rejects, and reset/retry across two fresh
generations retires all owners and Recording resources. Source/provider-removal
controls and the canonical full acceptance matrix apply after production
changes. Retail providers and private identifiers, paths, bytes, hashes,
images, or raw process output are excluded.

## Dependency and commit

Requires accepted 08F0. 08F post-map construction resumes only after this
slice is independently accepted. One production/test/evidence commit for
08F2; this discovery plan alone is not implementation acceptance.

## Result

Complete. The source visual now answers a disabled-grid height query with
`FALSE` and leaves the caller's height untouched for an exact published
original owner, including the accepted reset-detached water state. The
enabled request, inconsistent enabled flag, missing/foreign/pending provider,
and pre-init call reject. The generated terrain-water source probe checks
recovery after a failed provider acquire and zero-resource teardown. No 08F
selector or post-map scenario construction was admitted. Full acceptance is
recorded in [08F2 evidence](../../evidence/cnc-generals-zero-hour/milestone_22_slice_08f2.md).
