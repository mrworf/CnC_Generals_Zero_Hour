# M22 plan 01 slice 08P0C1B: tree interaction aggregate

## Dependency-ordered children

Native source separates the `unitMoved` partition collision and push-aside
route, post-cull toppling/fog/bounce/sink state, and immediate `FXList`
dispatch. The last route can create audio, drawable, light or other external
effects and is not a reversible GPU/state mutation. Deliver these owners as
[C1B1](milestone_22_plan_01_slice_08p0c1b1.md),
[C1B2](milestone_22_plan_01_slice_08p0c1b2.md) with ordered state/sink
children, then
[C1B3](milestone_22_plan_01_slice_08p0c1b3.md). This file is their
validation aggregate; do not merge the children into one production commit.

## Goal and boundary

After C1A, own the native `unitMoved` area-partition collision route and the
post-cull `drawTrees` interaction phase: push-aside outward/inward, crusher
topple force, fogged pause, angular acceleration/bounce/down state, optional
FX and sink/removal. Update the accepted visible geometry's positions and
darkening from staged state. Preserve exact source partition scope and frame
ordering; optional projected decals still belong to C3, shrouded draw C2,
and full-instance factory C4.

Preflight unit/partition, map, shroud/player, source geometry and effect
providers before mutation. Stage candidate interaction state, geometry and
effects; commit only after all fallible tree/resource work, or preserve accepted
instance, frame and GPU without gameplay destroy hooks or external dispatch.
External effects begin only after that commit; a partially throwing dispatch
is accepted/consumed at most once, not reversible. Missing
providers, invalid motion/sink parameters, upload fault, owner removal and
retry must not leave stale partition links, FX or buffers. Do not consume or
change GameLogic/audio RNG. Authorization is not applicable.

## Aggregate validation and commit

Generated fixtures cover immobile/moving units, box/cylinder collision,
partition boundaries, repeated pusher, outward/inward motion, eligible and
ineligible crusher, fog pause/reveal, bounce/no-bounce, sink/removal, pause,
move/removal, two generations and owner removal. Inject each interaction,
effect and geometry boundary; assert immediate residuals and clean retry.
After the three children, revalidate their composed event ordering, failed
state/geometry retry without duplicate effects and owner removal on generated
inputs. Run six complete builds and canonical nonretail suites, focused strict
host LSan, physical Vulkan controls, serial LAN 4/4 all six, ledger and diff
checks on final source. Commit only aggregate evidence:
`delivery: M22 08P0C1B revalidate tree interactions`.

## Delivered

Accepted C1B1, C1B2 and C1B3 compose the public collision, transformed frame,
sink/deletion and admitted/consumed FX transaction. The wording above now
states the children's explicit irreversible dispatch contract, not an
external-effect rollback claim. The unchanged final-source matrix and
generated owner/fault/retry composition are recorded in
[aggregate evidence](../../evidence/cnc-generals-zero-hour/milestone_22_slice_08p0c1b.md).
No draw/decal/factory/retail admission or production merge is included.
