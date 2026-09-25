# M22 plan 01 slice 08P0C1: terrain tree frame state

## Goal and boundary

After 08P0B, implement the generated CPU terrain tree frame-state owner.
Preserve native `W3DTreeBuffer` update order: breeze/pause, camera cull and
visible list (native sort remains disabled), unit push-aside, sway, toppling,
bounce/sink and dirty geometry. Account for native frame/client RNG use without
perturbing GameLogic or audio streams. Keep atlas/shroud rendering, projected
decals and the physical factory closed for C2–C4.

Preflight exact terrain/map, source resources and frame providers. Stage
fallible per-frame mutations and side effects; publish only a complete frame
state. On rejection or provider removal, restore accepted type/instance
identities, frame state and RNG, with no queued effects or partial geometry;
retry without reset. Move/removal/reset must not leave stale visible entries.
Authorization is not applicable; this is generated, asset-free runtime work.

## Validation and commit

Generated positive/negative fixtures cover multiple tree types and positions,
camera visibility, breeze/pause, push/topple/sink, move/removal and two device
generations. Inject each owned state/failure boundary, compare clean retry
against a fresh equivalent frame, and assert immediate residuals and owner
removal. Run six complete builds and canonical nonretail suites, focused
strict host LSan, physical Vulkan controls, serial LAN 4/4 all six, ledger
and diff checks on final source. Commit one slice:
`delivery: M22 08P0C1 own tree frame state`.
