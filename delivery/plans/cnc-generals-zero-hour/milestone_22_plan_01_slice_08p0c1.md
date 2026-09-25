# M22 plan 01 slice 08P0C1: terrain tree frame-state aggregate

## Goal and boundary

After 08P0B, compose C1A breeze/pause, camera cull and visible geometry with
C1B collision push-aside, toppling/fog/bounce/sink and dynamic geometry.
Native `drawTrees` runs breeze and cull before optional decal queue, then
push/topple before geometry update; C3 later inserts the independently owned
optional decal phase. Native visible sort is disabled and must remain so.
Keep shrouded draw, decals and physical factory closed for C2–C4.

Revalidate exact map/terrain and source-resource owners, frame identity,
GameClient RNG, provider removal and retry. No failed frame may leave changed
accepted tree/type identities, half-uploaded geometry, queued effects or
changed GameLogic/audio RNG. Authorization is not applicable.

## Validation and commit

Generated fixtures compose C1A/C1B across two generations and assert source
ordering, visibility, pause, collision/topple progression, geometry bytes,
failure/retry and immediate pre-teardown residuals. Reuse C1B final-source
six complete builds and canonical nonretail suites, focused strict host LSan,
physical Vulkan controls, serial LAN 4/4 all six, ledger and diff checks.
Commit the aggregate alone: `delivery: M22 08P0C1 revalidate tree frame state`.
