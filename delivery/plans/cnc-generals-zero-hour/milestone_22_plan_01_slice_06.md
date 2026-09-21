# M22 plan 01 slice 06: original GameClient scene integration

## Outcome and dependencies

Requires slices 04–05. Original `W3DDisplay::init` runs in authored order:
physical 2D status-circle, scene/asset creation, shroud, terrain, tracks
and shadow owners. Linux display/view traverses original
`GameClient`/`RTS3DScene` with a recording device; full-behavior original
factory replaces schema-only factory in production `zh_original_main`.

## Reached source closure

Translate original `W3DDisplay.cpp`, `W3DScene.cpp`, `W3DShroud.cpp`,
`W3DTerrainTracks.cpp`, `Shadow/W3DShadow.cpp` and reached original derived
volume/projection/decal shadow managers, plus original 2D/terrain/camera,
light/fog, water, particle and effect producers exercised by selected
scenes. For every reachable physical throw/guard, preserve original CPU
decisions and translate its physical operation; otherwise fail before a
successful frame. Inventory disabled families and negative routes explicitly.

## Test and failure contracts

Original scene traversal emits source-identified balanced recording frames
for owned scenarios including all shadow types, tracks, shroud, terrain and
effects reached. Missing required owners and unsupported state reject the
frame; reset/reentry/injected failures release all original/device resources.
Prove production factory/GameClient identity, unmixed ABI, no guarded no-op
reached, no Direct3D dependency, GCC/Clang suites and ledger freshness.
Retail acceptance remains slice 07, not an authored-fixture claim.

## Commit boundary

One independently validated commit: `delivery: M22 slice 06 integrate original GameClient scene`.
