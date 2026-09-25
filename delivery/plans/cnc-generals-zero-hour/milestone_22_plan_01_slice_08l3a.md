# M22 plan 01 slice 08L3A: atomic bridge and tower owner

## Goal and observable outcome

For a generated, positioned bridge Object, the source `Bridge(Object*)` owner
creates its configured tower Objects, links their behavior interfaces and
publishes exactly one terrain Bridge plus one pathfinder bridge layer. A
missing or malformed required provider, failed tower, invalid interface or
failed layer acquisition leaves no new bridge, tower or layer owner. A clean
retry produces one coherent owner graph.

## Scope and ordering

Depends on accepted 08L2 Object/Drawable construction and
[08L3A0](milestone_22_plan_01_slice_08l3a0.md) borrowed W3D file-factory
closure; precedes map-attempt
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

Admission failure leaves the already positioned input Object with its caller:
`addLandmarkBridgeToLogic` returns `FALSE` after retiring only its terrain
Bridge, constructed towers and acquired layer. The direct generated witness
uses the accepted 08L2 rollback on that caller-owned Object; 08L3B must do
the same in the map attempt before using the failed `obj` again. This avoids
a dangling map-loop pointer while preserving a one-owner-at-a-time contract.

No additional authorization applies to generated test data. The retail
symlink and all content behind it are read-only and never used here.

## Investigation before production edits

- A road type's empty tower name is intentionally optional; the current
  constructor tries all four positions and skips a missing tower after
  `createTower` returns null. Preserve empty names as absent towers, while a
  nonempty unresolved tower name is a required-provider failure.
- `PathfindLayer::reset` releases one layer's bridge pointer and allocated
  cells. Add a checked, pointer-matched pathfinder rollback entry so the
  current Bridge alone can release its layer. The existing normal
  `TerrainLogic::deleteBridge` calls gameplay destruction and is unsuitable.
- `Pathfinder::addBridge` iterates through numeric `LAYER_WALL` (15) as its
  final bridge slot before returning `LAYER_GROUND`; pointer identity, not a
  blanket wall-layer exclusion, protects actual wall state during rollback.
- `Bridge(Object*)` currently returns early when TerrainRoadType is missing,
  leaving `m_next` uninitialized. It can also create a partial tower graph
  before failure. `addLandmarkBridgeToLogic` publishes the Bridge before
  `Pathfinder::addBridge`, whose `LAYER_GROUND` value means failure.
- Use project-owned `Bridge` INI and modeled Object definitions in a generated
  fixture. The direct terrain admission entry is the focused 08L3A witness;
  authored MapObject traversal remains 08L3B.
- The first modeled generated bridge probe reached `W3DModelDraw` but
  `WW3DAssetManager::Load_3D_Assets` dereferenced a null `_TheFileFactory`.
  `OriginalDrawOwners` borrows W3DDisplay's assets/scene while omitting the
  `W3DFileSystem` owner. This independently testable lifecycle gap is assigned
  to 08L3A0. The uncommitted 08L3A source/test probe was removed before that
  prerequisite's plan checkpoint; no bridge acceptance is claimed.

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
