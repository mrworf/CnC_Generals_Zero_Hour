# M22 plan 01 slice 08L3A: atomic bridge and tower owner

## Goal and observable outcome

For a generated, positioned bridge Object, the source `Bridge(Object*)` owner
creates its configured tower Objects, links their behavior interfaces and
publishes exactly one terrain Bridge plus one pathfinder bridge layer. A
missing or malformed required provider, failed tower, invalid interface or
failed layer acquisition leaves no new bridge, tower or layer owner. A clean
retry produces one coherent owner graph.

## Scope and ordering

Depends on accepted 08L2 Object/Drawable construction and precedes map-attempt
composition in 08L3B. Cover `Bridge(Object*)`, `createTower`,
`TerrainLogic::addLandmarkBridgeToLogic`, `Pathfinder::addBridge`, and a narrow
construction rollback/removal API for a bridge layer. Preserve normal bridge
deletion behavior. Do not change map traversal, radar, ordinary objects,
retail selector or physical renderer in this slice.

## Entry, state, validation and recovery

The entry is the terrain owner's landmark-bridge admission for an already
constructed bridge Object. Validate source geometry, required TerrainRoadType
and each configured tower template/interface before lasting publication. The
constructor must initialize its list link before any early exit. Tower Objects
created before a later failure are retired in reverse via 08L2 construction
cleanup, including behavior links; normal gameplay destroy hooks do not run.
Publish the Bridge to terrain only after construction is complete.
`Pathfinder::addBridge` currently returns `LAYER_GROUND` for failure; that
value must reject publication, with no false successful ground-layer claim.
If a layer was acquired then a later step fails, release that exact layer
without resetting any pre-existing layer, and remove the new terrain Bridge.
Preserve source-assigned bridge and tower Object IDs on success.

No additional authorization applies to generated test data. The retail
symlink and all content behind it are read-only and never used here.

## Investigation before production edits

- Check `Bridge(Object*)` behavior for optional tower positions and whether a
  TerrainRoadType can intentionally omit tower templates. Retain intentional
  optional behavior; fail closed only for a configured required tower.
- Check pathfinder layer allocation and zone/cache mutations to make rollback
  exact before choosing the narrow removal API.
- Select a project-owned generated bridge Object/TerrainRoadType fixture and
  direct source entry that can witness constructor, layer and retry state.

## Tests and validation

Positive: a generated modeled bridge with valid source providers publishes
one terrain Bridge, a non-ground pathfinder layer and coherent optional tower
links; two generations and a clean retry have identical counts and IDs.
Negative: missing road type, missing configured tower template, missing
bridge/tower behavior interface, injected failure after each constructed tower,
and layer exhaustion/rejection leave no new owner. Check initial/partial list
link state, the `LAYER_GROUND` failure-return case, unchanged pre-existing
bridge/layer owners, and provider removal. Exercise direct source cleanup with
zero residual Object/Drawable/module, terrain and pathfinder owners.

Run the focused generated owner and accepted 08L2 construction tests, source
identity and all three dependency-ledger checks. Before commit, run the
governing M22 complete build/suite, strict host leak, public Vulkan, serial LAN
and diff gates on the final production tree.

## Acceptance and commit boundary

One independently reviewable bridge/tower owner and rollback commit:
`delivery: M22 08L3A make bridge publication atomic`. No map-attempt
transaction claim until 08L3B.
