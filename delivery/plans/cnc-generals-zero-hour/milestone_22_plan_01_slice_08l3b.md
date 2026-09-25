# M22 plan 01 slice 08L3B: atomic map bridge and wall attempt

## Goal and observable outcome

The native bridge phase consumes generated authored bridge-like and
walk-on-wall MapObjects. It constructs and positions neutral-team objects,
attaches the accepted 08L3A terrain Bridge/pathfinder layer or wall piece,
applies map properties, refreshes radar and calls pathfinder `newMap` in source
order. Success exposes one coherent world; failure retires only the current
map attempt and a retry reproduces it.

## Scope and ordering

Depends on accepted 08L3A and [08L3B0](milestone_22_plan_01_slice_08l3b0.md)
pathfinder owner prerequisite, and closes the 08L3 aggregate. Cover the
`GameLogic::startNewGame` bridge loop through pathfinder `newMap`, including
bridge/wall selection, object position/orientation, property update, optional
bridge tower links, radar refresh and map-attempt rollback. Keep road/bridge
flagged terrain-side MapObjects in their source branch. Do not claim later
ordinary-object traversal, preload, camera/UI, retail admission or pixels.

## Entry, state, validation and recovery

Enter through generated-only source map load with an authored bridge and
walk-on-wall object graph. Validate template/interface and property providers
before lasting mutation where possible; reject duplicate map owner or
unsupported attachment rather than silently continuing. Track every new
Object, Drawable, bridge/layer/tower, wall registration and affected derived
owner. On failure, use 08L3B0's fresh derived-map and exact wall rollback,
then unwind terrain attachments before retiring
objects through 08L2 construction cleanup; preserve pre-existing world owners.
`Radar::refreshTerrain` clears its queued refresh frame; capture and restore
that prior queue state on failed attempt. Do not claim atomicity from resetting
the whole world.

No additional authorization applies to generated data. The retail symlink
and its content remain read-only; tests report only fixed aggregate categories.

## Investigation before production edits

- Identify whether `Object::updateObjValuesFromMapProperties` can fail or
  partially mutate, and record exact recovery for every reached failure edge.
- Verify the radar queue-frame restore contract, and compose 08L3B0's
  pathfinder rollback before publishing a successful map attempt.
- Build a project-owned map-object graph with modeled bridge and wall source
  templates, neutral team, valid terrain and bounded property dictionary.
- Identify any bridge-tower behavior that reaches normal destroy callbacks
  during unwind; route it through construction cleanup without changing
  accepted gameplay deletion.

Investigation resolved: the property updater can publish ambient audio and
apply upgrades outside the new Object, so the generated route admits only
bounded `originalOwner`, `objectName`, `objectMaxHPs` and
`objectInitialHealth` values before any construction. A property-stage fault
occurs after the bounded Object-local mutation and reverse cleanup retires
that Object without gameplay destroy hooks. Radar exposes its prior queued
frame for exact failure restoration. 08L3A tower rollback and 08L3B0 fresh
pathfinder rollback compose in reverse order; neither resets an accepted
bridge layer. The ordinary map loop remains unchanged outside the conjunctive
generated route.

## Tests and validation

Positive: source-ordered bridge and wall construction, terrain and pathfinder
attachments, property effects, radar refresh and pathfinder `newMap`; mission
and skirmish, two generations, deterministic retry, zero-owner teardown.
Negative: absent/malformed template or interface, duplicate attachment,
partial tower, property failure, radar/pathfinder failure and provider removal
leave no attempt owner; pre-existing world counts and IDs stay intact. Inject
each publication boundary and assert immediate pre-teardown residual zero.
Revalidate accepted generated construction, no-model prop, terrain/water and
pre-map display routes. Run the complete M22 six-build/six-suite, focused
strict host leak, public Vulkan, serial LAN, ledger and diff gates on the
final production tree.

## Acceptance and commit boundary

One map-attempt behavior commit:
`delivery: M22 08L3B attach bridge and wall map objects transactionally`.
This commit closes aggregate 08L3 when every acceptance check passes.
