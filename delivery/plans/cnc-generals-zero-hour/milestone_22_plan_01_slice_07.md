# M22 plan 01 slice 07: original GameClient scene integration

## Outcome and dependencies

Requires slice 06. Original `W3DDisplay::init` runs in authored order:
physical 2D status-circle, scene/asset creation, shroud, terrain, tracks
and shadow owners. Linux display/view traverses original
`GameClient`/`RTS3DScene` with recording `GpuDevice`; full-behavior original
factory replaces schema-only factory in production `zh_original_main`.
Retail acceptance remains slice 08.

## Reached source closure

Translate reached original `W3DDisplay.cpp`, `W3DScene.cpp`,
`W3DShroud.cpp`, `W3DTerrainTracks.cpp`, `Shadow/W3DShadow.cpp` and
derived volume/projection/decal shadow managers, plus original 2D/terrain,
camera, light/fog, water, particles and effect producers exercised by
selected scenes. Every reached physical throw/guard either translates the
original operation or fails before successful frame. Preserve source-owned
selection and state; inventory disabled families and negative routes.

## Test and failure contracts

Original GameClient scene traversal emits source-identified balanced
recording frames for owned scenarios including all selected shadow types,
tracks, shroud, terrain and effects. Missing required owners and unsupported
state reject the frame; reset/reentry/injected failures release all original
and device resources. Prove production factory/GameClient identity, unmixed
ABI, no reachable guarded no-op, no Direct3D dependency, GCC/Clang suites,
focused sanitizers and ledger freshness. Do not substitute owned fixtures
for retail acceptance.

## Commit boundary

One independently validated commit: `delivery: M22 slice 07 integrate original GameClient scene`.

## Reconciled closure

The accepted no-map and 07F8 terrain children do not satisfy this slice. See
the [07 reconciliation](../../evidence/cnc-generals-zero-hour/milestone_22_slice_07_reconciliation.md): source map loading reaches terrain visual/map
publication, shroud-before-view update, track flush, water, shadow and active
particle/smudge routes, while the current Linux view/display branches still
reject loaded maps. The required pre-retail closure is
`07F8A -> 07F9 -> {07FA, 07FB, 07FC, 07FD, 07FE} -> 07FF -> 07FG`; only after 07FG may
slice 08 read private retail data. Slice 07 remains pending.
