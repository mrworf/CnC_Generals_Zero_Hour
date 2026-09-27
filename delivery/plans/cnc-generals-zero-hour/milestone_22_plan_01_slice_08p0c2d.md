# M22 plan 01 slice 08P0C2D: source scene tree draw and retry

Status: accepted; final-source acceptance complete
after the approved display-presentation architecture checkpoint. Transaction parent is
C2C `cb8651c82d6e5b843cd3d576bb2f49c2f1c74cb6`. The existing unrelated
`tests/renderer/test_bgfx_device.cpp` diagnostic remains excluded. Persist this
plan/index checkpoint before executable edits; no additional slice is opened.

## Goal, dependency and boundary

After C2A, D0A/D0B, B0/B and C2C, consume only an admitted immutable tree phase in the native
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

## Resolved source frame, presentation and rollback composition

Native `W3DDisplay` completes its main frame with default
`WW3D::End_Render()` (presentation enabled). The CPU path's explicit `false`
and generated caller presentation were an incomplete bounded path. For an
admitted immutable tree phase, display owns one complete frame transaction:
capture the complete source checkpoint, begin the exact D0 frame journal
before `Begin_Render`, execute the source frame, end with exactly one final
present/CompleteFrame, commit, then retire the same phase outside the pass.
Do not present again in generated callers. Ordinary no-tree frames retain
their accepted behavior; unsupported SDL/other transaction backends reject.
Actual admission failure retains C2C's accepted identity without invoking C1.
Successful preparation's earlier begin/abort probe is not future availability.

Move the CPU route's later stencil/water/particle scheduling from
`Customized_Render` to its native `Flush` positions. Terrain and tracks remain
in native terrain traversal; non-stencil decals precede mesh/occluded/shader
flush; `DoTrees` follows those flushes and precedes stencil/static-water, then
translucent/particles/sorting/final pending-delete cleanup. This tree admission
is shadow-disabled: optional tree projected decals stay C3. Do not silently
admit unsupported lights, occluded/translucent geometry or sibling categories.

The device transaction restores device resources and commands, not source
ownership. Capture all reached mutable source state before frame mutation,
with checked bounded copies and explicit reference ownership:

- Edge vertex/index/texture maps and texture ownership multiplicities,
  pending stages/filter tuples, generic physical resources, source revision,
  viewport/pass state and exact target generation. Pin baseline providers so
  candidate replacement cannot destroy an identity needed by abort. Retain
  monotonic fault/token/candidate-generation counters; restore accepted state
  identities without reusing a canceled candidate handle.
- Full DX8 selected texture/material/VB/IB references and engine references,
  source shader/ShaderDirty/CurrentShader, dirty bits, physical material,
  render/stage/transform maps, fog/light state and selected buffer offsets,
  counts and draw switches. Source textures' LastAccessed and frame-expiry
  readiness metadata must restore exactly; a stale/lazy provider cannot load
  during admission or silently replace an accepted mapping.
- WW3D rendering/frame/statistics counters and reached dynamic-buffer reset
  state, source shader-manager selection/pass, mesh/static/sorting pending
  work, scene traversal/camera/visibility state and material-pass balance.
  A source-proven deterministic reconstruction may replace a transient copy
  only when it restores the exact declared baseline without simulation work.
- Terrain extra-blend visible-count/temporary work, track edge queue and
  rewritten CPU vertex bytes, particle ready/on-screen counters and smudge
  request/last-frame state. Delay destructive transient consumption until
  successful commit, or checkpoint its exact original owner; no reset-world
  shortcut or lost request on retry. Unsupported nonempty work rejects before
  commands rather than being discarded to manufacture an empty baseline.

Capture allocations and pins finish before the journal/source frame opens.
Every begin/provider/upload/draw/end/present/commit rejection aborts the device
journal and restores this complete checkpoint through no-throw operations,
including paths where existing WW3D failure cleanup already reset caches.
No partial accepted pixels, Recording commands, stale Edge mapping or source
queue escape. Baseline source resources retire only after abort restores or
commit consumes the journal. Keep the phase's exact immutable program/VB/IB/
atlas/shroud/constants independent of generic source cache mutation; draw its
resident buffers without upload/repreparation. Zero-visible phases emit no
tree triangles but follow the same successful completion/cancel ownership.

## Exact focused command contract

Add narrow generated `original_w3d_tree_draw` using the existing full probe,
rigid-tree producer and logical/visual map providers; add a physical `--gpu`
route to the same producer. Preserve every C2C preparation/lifecycle assertion
while distinguishing explicit preparation/cancellation controls from newly
completing public display draws. Do not expand the heavy 64-type/4000-instance
witness for this coverage.

`cmake --build build/<preset> --target zh_original_w3d_full_probe original_w3d_cpu_graph_tests original_w3d_tree_program_tests original_w3d_source_reference_tests original_w3d_stage_transaction_tests renderer_recording_transaction_tests renderer_bgfx_transaction_resource_tests renderer_bgfx_transaction_tests -j4`

`ctest --test-dir build/<preset> -R '^(original_w3d_tree_draw|original_w3d_tree_preparation|original_w3d_tree_program|original_w3d_terrain_shroud_projection|original_w3d_terrain_map_frame|original_w3d_source_reference|original_w3d_stage_transaction|renderer_recording_transaction|renderer_bgfx_transaction_resource|renderer_bgfx_transaction)$' --output-on-failure`

Run this exact ten-control union on GCC/Clang native and both sanitizers,
`ASAN_OPTIONS=detect_leaks=0` for sanitizer focus. Strict host LSan selects
the same ten with exactly `ASAN_OPTIONS=detect_leaks=1`, no UBSan override.
Physical generated tree draw:
`VK_INSTANCE_LAYERS=VK_LAYER_KHRONOS_validation ASAN_OPTIONS=detect_leaks=0 python3 tests/original_rendering/test_w3d_tree_draw.py --source-root . --executable build/<preset>/zh_original_w3d_full_probe --asset-producer build/<preset>/original_w3d_cpu_graph_tests --gpu`.
Run all four focused configurations with host graphical environment/escalation;
retain exact generated program/shroud and established Vulkan controls. Audit
complete logs, not exit codes. Six serial canonical suites, strict LSan,
serial LAN and ledger/diff remain mandatory below.

Witnesses include a revealed nonuniform-shroud/source-atlas tree with nonzero
sway slot and authored diffuse lighting/darkening, exact index ranges and
adjacent-family markers; faults before/between/after terrain and tree draws,
frame end/present and native commit rejection; identical same-phase retry
after each fault; hidden/pause/empty, wrong target/provider and removed/reset
owners; two generations with exact phase-only and final Edge residual counts.
Assert complete source checkpoint restoration and unchanged prior accepted
pixels/Recording result, not merely absence of a tree marker. Successful frame
consumes the phase once; retry never advances C1/RNG/FX.

## Surfaces, tests and acceptance

Expected surfaces: CPU BaseHeightMap source draw, source HeightMap terrain-pass
mark, RTS3DScene ordering/Flush and source display completion/retry; accepted
program/shroud/prepared owners, generated integrated Recording witness and
ledger. Audit each changed owner; the governing scope freeze requires an
explicit architecture checkpoint before any new prerequisite/split.
Preserve Windows/source ABI and unrelated renderer diagnostic.

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

## Resolved implementation and final workload facts

The bounded checkpoint also captures the exact asset-hash texture identities
visited by TextureLoader expiry, including initialized nonresident sources.
Only mutable access/inactivation/Initialized metadata is copied; neither lazy
loading nor initialization is admitted. Both hash entry count and distinct
combined metadata identity count are4096. Exact capacity and capacity+1,
resident/nonresident expiry rollback and clean retry are generated controls.
Dynamic DX8 VB/IB overwrite and growth restore exact old identity, capacity,
offset and full bytes; sorting and unsupported populated work remain closed.

Display owns the real final present/CompleteFrame for admitted tree frames;
generated callers never present twice. Independent owner round trips and
integrated populated particle/smudge late-fault controls preserve exact order,
identity, counters and bytes, then drain once. Per-operation faults use fresh
equivalent device generations while retaining failure/same-phase retry within
each instance; a separate control retains exact tombstone-capacity rejection.
The minimal sweep packet extracts exact generated mesh bytes from the unchanged
full preparation packet, not a regenerated source record.

The inherited30s per-process wrapper limit was shorter than the isolated clean
sanitizer workload (GCC41.87/42.35s, Clang40.93/41.13s). Only the new tree-draw
wrapper now has a bounded60s deadline; all assertions/workload and explicit
TimeoutExpired failure remain. No global or existing heavy-witness timeout
changed. Final gates use this corrected frozen test identity.

[Accepted evidence](../../evidence/cnc-generals-zero-hour/milestone_22_slice_08p0c2d_source_tree_draw.md)
records all final gates and superseded exploratory corrections. Six complete
builds and canonical278 suites, exact focus10 all4, strict10 both, physical
all4 and serial LAN4 all6 pass with clean complete logs and exact frozen hashes.
The implementation is identified by the unique commit subject above and Git
history; unrelated renderer diagnostics remain excluded. C2 aggregate may reuse
these unchanged executable gates without rerunning or changing source.
