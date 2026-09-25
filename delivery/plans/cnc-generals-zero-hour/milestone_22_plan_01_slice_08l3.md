# M22 plan 01 slice 08L3: transactional bridge and pathfinder attachment

## Goal and observable outcome

Consume 08L2 atomic construction in the native bridge phase. Generated authored
bridge-like and walk-on-wall objects are created against the neutral team,
positioned/oriented, updated from map properties and attached to terrain bridge
or pathfinder owners in source order. Radar refresh and pathfinder `newMap` see
one coherent world; failure removes only owners from the current map attempt and
retry is deterministic.

## Scope, ownership and rollback

Cover bridge registration, optional behavior/tower links, wall registration,
property update, terrain bridge IDs, radar refresh and pathfinder rebuild. Reject
absent/malformed templates or interfaces, duplicates, partial tower creation,
property/pathfinder/radar failures and provider removal before lasting mutation.
The map attempt owns each new object until all its attachments succeed. Failure
unwinds pathfinder/terrain links before retiring drawable/object owners through
08L2; pre-existing world owners remain untouched.

Do not use retail content, identify a private template, bypass bridge behavior,
skip pathfinder ownership or claim later ordinary-object/preload/camera/UI stages.

## Implementation and validation

Build a generated map-object graph with modeled bridge and wall fixtures and
fixed aggregate stages. Exercise success, each injected failure, reverse cleanup,
retry, reset/re-entry, provider removal and zero-owner teardown. Revalidate prior
generated construction, no-model prop, terrain/water and pre-map display routes,
then run complete M22 validation.

## Acceptance and commit boundary

Depends on 08L2. One commit:
`delivery: M22 08L3 attach bridges transactionally`.
