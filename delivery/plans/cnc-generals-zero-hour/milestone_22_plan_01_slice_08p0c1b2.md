# M22 plan 01 slice 08P0C1B2: topple, fog, bounce and sink state

## Goal and boundary

After C1B1, let eligible crusher collisions initiate the native tree-topple
state machine. Own minimum speed, normalized direction, angular acceleration,
fog freeze/reveal, angular limit and bounce/no-bounce, down state and bounded
sink/removal, in source frame order after C1A cull. Publish transformed
visible geometry and darkening with the accepted state. Stage event intent
but do not dispatch `FXList` until C1B3; this child must be testable with
effect-free generated module data. C2 draw, C3 decals and C4 factory remain
closed. Authorization is not applicable to generated local simulation.

## State, failure and surfaces

Trace `W3DTreeBuffer::applyTopplingForce/updateTopplingTree/drawTrees`,
`W3DTreeDrawModuleData`, local player and partition shroud status, and native
matrix transforms. Preflight valid crusher level, finite nonzero direction,
finite speed/acceleration/bounce/sink parameters, positive sink frames when
required, matching terrain/map/scene and available shroud/player providers.
Stage instance state, transformed visible geometry, Recording uploads and
removal candidate before publication. A failed provider, invalid parameter,
allocation/upload or injected publication leaves the accepted frame, tree
identity, partition membership, GPU resources and RNG untouched. Fogged
frames do not advance; reveal reaches the source down-state branch. Removal
and retry must not retain stale links. Expected source/test/ledger surfaces
are `BaseHeightMap` header/source and generated terrain fixture; if matrix or
effect ownership proves independently prerequisite, revise this plan first.

## Acceptance and commit

Generated positives: eligible/ineligible crusher, minimum speed, fog pause
and reveal, acceleration, bounce and no-bounce, down/sink/removal, pause,
visible/hidden trees and two generations. Negatives: missing shroud/player,
nonfinite or degenerate motion, zero sink frames, geometry and Recording
faults, owner removal/retry. Verify no FX dispatch in this child, no
GameLogic/audio RNG changes and immediate pre-teardown residuals. Run focused
GCC/Clang and sanitizer witnesses, six complete builds/canonical nonretail
suites, strict host LSan, physical Vulkan, serial LAN 4/4 all six, ledger
and diff checks. Commit one slice:
`delivery: M22 08P0C1B2 own tree topple state`.
