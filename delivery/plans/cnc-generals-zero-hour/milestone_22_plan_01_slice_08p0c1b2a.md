# M22 plan 01 slice 08P0C1B2A: crusher topple state

## Goal and boundary

After C1B1, admit an eligible mobile crusher collision through the native
tree partition route, applying minimum speed, finite nonzero normalized
direction, angular velocity/acceleration, fog freeze/reveal, bounce or
low-velocity/no-bounce stop, and DOWN state in source post-cull frame order.
Own the initialized per-instance transform and transformed visible vertex
bytes, including source darkening, with atomic frame/Recording publication.
Do not yet sink or delete DOWN trees; that owner is B2B. Do not dispatch
topple or bounce `FXList`; that owner is B3. C2/C3/C4 remain closed.
Authorization is not applicable to generated local simulation.

## State, failure and surfaces

Trace `W3DTreeBuffer::unitMoved/applyTopplingForce/updateTopplingTree`,
`W3DTreeDrawModuleData`, `Matrix3D`, `PartitionManager` prop shroud and the
local player. `Matrix3D`'s legacy default constructor leaves elements
uninitialized; initialize the new candidate transform explicitly on tree
admission and never copy or publish undefined padding as state. Preflight
crusher level, positive finite minimum speed/velocity, finite acceleration
and bounce, nonzero finite direction and available player/partition/shroud
before mutation. Stage event intent for B3 but perform no external effects.
Stage instance and visible geometry/Recording resources, then publish only
after every fallible step. Paused or fogged frames do not advance; reveal
enters the native DOWN transform branch. Failure preserves accepted frame,
instance identity, partition, GPU resources and GameClient/GameLogic/audio
RNG. Expected surfaces are `BaseHeightMap` header/source and generated
terrain/asset fixtures; do not alter native class layout or renderer test.

## Acceptance and commit

Generated positives: eligible and ineligible crusher, public movement
callback, minimum speed, fog pause/reveal, acceleration, bounce and
low-velocity/no-bounce down, paused/visible/hidden frames and two generations.
Negatives: missing player/partition/shroud, degenerate/nonfinite motion,
geometry and Recording faults, owner removal/retry. Verify exact transformed
raised vertex bytes, no FX dispatch and immediate pre-teardown residuals.
Run focused GCC/Clang and sanitizer witnesses, six complete builds/canonical
nonretail suites, strict host LSan, physical Vulkan, serial LAN 4/4 all six,
ledger and diff checks. Commit one slice:
`delivery: M22 08P0C1B2A own tree topple state`.

## Delivered evidence

The generated logical/visual map-size mismatch required a test-only Display
notification sink for real PartitionManager reveal/undo transitions; exact
notices, real Display restoration before frame execution, and complete
shroud-cell baseline reconstruction are asserted. This prevents intentional
fog history in the first internal generation from contaminating the second.
The existing immediate pre-teardown zero-residual assertion remains intact.
The retained 64-type/4000-instance witness exceeded its prior 120-second
per-subprocess bound under isolated sanitizer load; only this script now
uses a documented 240-second bound with explicit redacted timeout failure.
No production reset, FX or sink owner was smuggled into A. Six complete
canonical suites, strict host LSan, physical Vulkan, serial host LAN and
ledger checks passed; see the linked slice evidence in the governing index.
