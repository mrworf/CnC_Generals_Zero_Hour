# M22 plan 01 slice 08P0C2D: source scene tree draw and retry

## Goal, dependency and boundary

After C2A/B, D0A/D0B and C2C, consume only an admitted immutable tree phase in the native
terrain scene draw. Preserve terrain/tracks, non-stencil object decals,
mesh/occluded/shader flush, DoTrees, stencil shadows, static/water and final
translucent/particle order. The CPU scene currently performs some later shadow/
water operations inside Customized_Render; audit and restore exact affected
ordering without admitting unimplemented sibling modes. Optional projected
tree decals remain C3, physical W3DTreeDraw factory C4 and retail admission 08.
Generated local runtime work needs no authorization change.

## Draw, failure and source ownership

Native BaseHeightMap/renderTrees and tree pass are the producer. Bind exact
prepared VB/IB/atlas, world/material/program/shroud state and source ranges;
preserve alpha-test/depth-write/LEQUAL, diffuse lighting/darkening, disabled
fog, texture/filter and cull semantics. No adapter geometry, generic program,
unshrouded proxy or per-Drawable draw can stand in for the native terrain pass.
The in-frame consumer never calls updateTreeVisibleFrame or consumes RNG/FX.
Hidden/zero visible geometry emits no triangles while preserving admitted
phase semantics. Preflight all resources, source counts and capacities before
commands; stale/removed owners fail closed.

On failed draw/provider/submission/end, cancel owned frame state/commands,
preserve the prior accepted render result and retain C2C's exact accepted
simulation phase for clean retry. Retry uses identical state/buffers/constants
and cannot advance C1 or replay effects. Only successful source completion
retires the prepared phase. Owner removal/reset cancels safely without a
world-reset shortcut; resource retirement respects the active-frame boundary.

The completed rollback audit found no checkpoint: OriginalGpuEdge abort calls
only end_pass, Recording appends commands, bgfx touches/submits immediately
and SDL end submits its command buffer. Required prerequisite D0A owns bounded
public capability/Recording rollback; D0B owns public bgfx deferred commands.
Use only those admitted real transactions, never fixture mutation of private
containers or unsupported-backend success. Physical submissions retain their
explicit irreversible commit boundary. Never claim reversible external C1
effects or an accepted partial scene frame.

## Surfaces, tests and acceptance

Expected surfaces: CPU BaseHeightMap source draw, source HeightMap terrain-pass
mark, RTS3DScene ordering/Flush and source display completion/retry; accepted
program/shroud/prepared owners, generated integrated Recording witness and
ledger. Audit each changed owner and split further only if independently
reviewable. Preserve Windows/source ABI and unrelated renderer diagnostic.

Tests assert actual source-issued triangles/indices/atlas/program/shroud and
exact adjacent-family ordering, nontrivial sway/darkening/lighting, hidden/
visible/pause, two generations and owner removal. Inject provider/stage/draw/
frame faults; assert no successful partial frame, prior accepted render result,
same immutable phase and no C1/RNG/FX advance on retry. Exact resource counts
return to immediate baseline; no placeholder or reset shortcut. Persist exact
focused commands before edits; run both toolchains/focused sanitizers, six
complete builds/canonical nonretail suites (`-LE 'gpu|lan|retail'`, sanitizer
`ASAN_OPTIONS=detect_leaks=0`), exact serial host LSan
(`ASAN_OPTIONS=detect_leaks=1`, no UBSan override), physical Vulkan, serial
LAN 4/4 all six, ledger/diff on final source. Commit one slice:
`delivery: M22 08P0C2D integrate source tree terrain draw`.
