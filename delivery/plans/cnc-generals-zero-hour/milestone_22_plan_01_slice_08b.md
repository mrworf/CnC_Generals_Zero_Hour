# M22 plan 01 slice 08B: source retail terrain-configuration closure

## Outcome and dependency

Requires accepted 08B3.  Its private-safe, read-only campaign and skirmish
audit returned the permitted aggregate mask `30`: unsupported terrain-track
count (bit 1), shadow volumes (bit 2), source shadow decals (bit 3), and cloud
plane (bit 4).  No root, resource name, byte, hash, or raw Recording command
is recorded.  08B aggregates the independently accepted source-owned route
for each reached bit; it may
not clear the mask, change retail settings, or make `W3DTerrainVisual::init`
permissive merely to advance startup.

## Scope

Use the now available test-selected Recording factory and the actual original
campaign/skirmish consumers to establish source owner/update/draw/reset order
for the reported terrain tracks, volume/decal shadow mode, and cloud plane.
Retain all existing source ownership, profile absence and unsupported variants
as fail-closed.  Emit only aggregate command/family/count evidence and verify
input metadata remains unchanged.  Support for other observed mask bits,
retail model families beyond the reached terrain boundary, physical pixels,
resize, UI/gameplay changes, and raw private material remain out of scope.

## Dependency split

The source review found that only track cardinality has an already linked,
general CPU owner/flush implementation. Cloud-plane ownership is separately
guarded by water, scene, and display code; the original volumetric manager is
not linked in the CPU full-draw target. The safe order is therefore 08B1
(multi-track pool), 08B2 (cloud plane), then 08B3 (volume shadows). Existing
bounded decal support remains part of the aggregate only; no profile selector
or configuration flag is used to pretend an unimplemented family exists.

## Required proof

Positive: both selected consumers pass the original terrain guard only when
every mask-30 family is source-created, updated and drawn through Recording in
source order, with no duplicate producer.  Negative: each missing family,
unsupported track cardinality/shadow/cloud variant, create/upload/draw
failure, producer removal, reset/retry, and two-generation re-entry fails or
recovers without aliases/resources.  Validate factory identity and zero
ownership, source/provider/ledger controls and the separate read-only retail
metadata invariant.  Record only sanitized aggregate family/command counts.
