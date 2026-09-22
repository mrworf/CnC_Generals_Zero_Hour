# M22 plan 01 slice 07F6C: flat terrain base material selection

## Goal and observable outcome

Requires accepted 07F6B. The canonical flat, unblended terrain base atlas can
be selected through original `DX8Wrapper` delayed state and its virtual
`TerrainTextureClass::Apply` method. Recording witnesses the exact published
handle and sampler state, with no draw submission or physical pixel claim.

## Dependency finding and scope

In the native source, `TerrainTextureClass::Apply` calls
`TextureClass::Apply(stage)` and all remaining custom fixed-function code is
inside the source's obsolete `#if 0` block. Therefore the exact smallest
source-equivalent Linux branch is the base call already accepted by the
texture-decision/edge slices. Do not invent terrain-specific shader state.

The flat-map submission path still cannot render after this change:
`HeightMapRenderObjClass::Render` depends on the unported
`W3DShaderManager::ST_TERRAIN_BASE` pass selection. The active blend alias has
a separate, live `AlphaTerrainTextureClass::Apply` state machine; active edge,
cloud/light maps, shroud projection, effects and their material behavior also
remain later children. This slice does not compile or emulate that behavior.

## Entry, state, errors and surfaces

Reuse the generated authored flat map and F6B atlas owner. Install a
production-compatible Recording edge, queue its base terrain texture through
`DX8Wrapper::Set_Texture`, and execute
`DX8Wrapper::Apply_Render_State_Changes`. Verify source order, the exact atlas
handle, bounded default filter/address sampler state, idempotent re-apply and
texturing-disabled null selection.

Missing device ownership, invalid stage, stale/prior-generation texture,
apply after atlas teardown and injected sampler failure must remain rejected
without a draw or stale selected resource. Releasing DX8 delayed state precedes
map teardown. A second device/map generation repeats cleanly and all device
resources return to zero. Retail inputs and retail symlink content remain
untouched.

Expected edits are the Linux `TerrainTextureClass::Apply` body, the focused
generated-map probe (plus a narrow Recording sampler fault control only if
needed), ledger hash, plan/index and evidence.

## Tests and acceptance

Positive tests cover canonical virtual apply, exact handle/sampler descriptor,
texturing enabled/disabled, idempotence, delayed-state release, teardown and
two-generation re-entry. Negative tests cover missing edge, invalid stage,
stale generation and sampler/application rollback. Recording draw count stays
zero throughout.

Run GCC and Clang focused source/Recording tests with leak detection, identity,
provider-removal and ledger controls, then the exact five non-LAN suites and
serial LAN split. This CPU/Recording material-selection slice makes no terrain
submission or physical pixel claim.

## Commit boundary

One commit: `delivery: M22 slice 07F6C apply flat terrain base material`.

## Result

Complete. The canonical virtual base-atlas Apply now delegates exactly to
`TextureClass::Apply`, preserving source handle/filter order in delayed
public-GPU state. Enabled/disabled selection, injected sampler failure and
retry, idempotence, state retirement and two-generation atlas ownership pass
with zero draws. Alpha blending, terrain shader/pass selection, submission and
pixels remain pending.
