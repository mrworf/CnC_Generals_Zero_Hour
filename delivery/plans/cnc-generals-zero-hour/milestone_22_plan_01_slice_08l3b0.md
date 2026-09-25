# M22 plan 01 slice 08L3B0: pathfinder wall and fresh-map owner

## Goal and observable outcome

The pathfinder admits a walk-on-wall Object with a reported capacity result and
can unwind a failed fresh `newMap` attempt without discarding bridge layers
accepted by 08L3A. A generated direct-source witness shows partial allocation
and readiness rollback, intact pre-existing layer identities and clean retry.

## Scope and ordering

Depends on accepted 08L3A and precedes 08L3B map-object composition. Own only
`Pathfinder`/`PathfindLayer` wall registration and fresh derived-map state:
zone blocks, ground cells, row pointers, allocated bridge/wall layer cells,
classification and readiness. Do not change normal map reset, terrain bridge
construction, authored MapObject traversal, radar, retail admission or pixels.

## Entry, validation and recovery

`addWallPiece` must reject null, duplicate and capacity exhaustion with an
explicit result, never silently succeed. Its rollback/removal must match the
exact Object ID and leave earlier wall registrations intact. For a fresh-map
attempt, capture the pre-attempt extent/readiness/derived owner state and
accepted bridge-layer pointer identities. If `newMap` fails at any reached
allocation or classification boundary, retire only cells and zone data
allocated by that attempt, restore the earlier readiness/extent state and
retain bridge-layer ownership and wall registrations for the caller's reverse
unwind. Never invoke the broad `Pathfinder::reset()` shortcut, which also
destroys accepted bridge layers. Reject a non-fresh or already-ready owner
instead of applying fresh-map rollback to it.

## Tests and validation

Direct generated bridge/wall owner probe: capacity boundary and duplicate
wall registration, exact removal, two real bridge-layer identities before and after
fresh-map rollback, and deterministic `newMap` retry. Inject failure after
zone, ground cells, row pointers, each active layer allocation, and before
readiness publication; check immediate residual derived owners are zero and
pre-existing bridge pointers/IDs/layer numbers are unchanged. Run two
generations, provider-removal and both strict host leak checks. Revalidate
accepted 08L3A, then all six builds/six asset-free canonical suites,
physical Vulkan 2/2, serial host LAN 4/4 per build, ledger and diff.

## Acceptance and commit boundary

One prerequisite behavior commit:
`delivery: M22 08L3B0 make fresh pathfinder map rollback atomic`.
08L3B alone closes the aggregate map attempt.
