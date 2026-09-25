# M22 plan 01 slice 08P0C1B1: partition collision and push-aside

## Goal and boundary

After C1A, route a moving unit through the native `W3DGameClient` to terrain
`unitMoved` entry and the tree owner's bounded area partition. Select the
source box/cylinder radius, clamped bucket range and collision sphere; ignore
immobile units, deleted/noninteractive trees and repeat pushers within the
source frame window. Own outward/inward push state and the resulting visible
geometry position/darkening update. Do not yet initiate crusher toppling or
emit FX; those belong to C1B2/C1B3. C2 shroud draw, C3 decals and C4 factory
remain closed. Authorization is not applicable to generated local simulation.

## State, failure and surfaces

Inspect `W3DGameClient::notifyTerrainObjectMoved`, `BaseHeightMapRenderObjClass::unitMoved`,
the CPU tree registry, `W3DTreeBuffer::unitMoved/pushAsideTree/drawTrees`, and
`W3DTreeDrawModuleData`. Determine a generated test-owned `Object` fixture or
a narrow adapter that exercises the same collision decision without admitting
a physical factory. Validate finite position, direction, radius, partition
bounds and positive outward/inward frame counts before mutation. Stage
instance push state and visible geometry/Recording uploads; publish together
only after all fallible work, retaining accepted partition identity and frame
on rejection. Pause blocks advancement. No GameLogic or audio RNG changes.
Removal, relocation and reset clear or reindex partition ownership without
stale links; retry needs no world reset. Expected source/test/ledger surfaces
are `BaseHeightMap` header/source and generated terrain scene attachment
fixture; split a discovered prerequisite before production if its owner is
independent.

## Acceptance and commit

Generated positives: immobile/moving box and cylinder units, interior/edge
bucket overlap, pusher direction, repeated pusher, outward then inward
motion, pause, relocation/removal and two generations. Negatives: null or
invalid unit geometry/provider, zero/invalid frame counts, partition range,
geometry or Recording failure. Assert exact accepted state and immediate
resource residual before retry. Run focused GCC/Clang and sanitizer witnesses,
then six complete builds/canonical nonretail suites, strict host LSan,
physical Vulkan, serial LAN 4/4 all six, ledger and diff checks. Commit one
slice: `delivery: M22 08P0C1B1 own tree push collision`.
