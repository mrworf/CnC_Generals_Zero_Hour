# M22 plan 01 slice 07F8: bounded original base-terrain submission

## Goal and observable outcome

Requires accepted 07F7. The canonical CPU-only `HeightMapRenderObjClass`
performs the smallest original `Render` branch that is now source-ready: an
initialized generated flat map owns its existing source vertex/index buffers,
selects the F7 base/alpha atlas passes, and emits two indexed terrain draws
through an active Recording edge.

## Scope and dependency findings

The existing Linux owner already creates the original `DX8VertexBufferClass`
and `DX8IndexBufferClass`, fills the source quad topology and emits the native
`DX8_FVF_XYZDUV2` vertices. Its `Render` override currently rejects before
the source `Set_Index_Buffer` → `W3DShaderManager::setShader` →
`Set_Vertex_Buffer` → `Draw_Triangles` sequence. The source full path uses a
fixed 32x32 tile buffer, regardless of the bounded logical flat map; its
unpopulated cells are source-degenerate rather than adapter geometry.

The generated fixture must include the existing one-class source bitmap/atlas
input: `getTerrainTexture()` is source-owned atlas construction, not an
adapter placeholder. Each bounded render also retains the original outer
material/shader selection before terrain-pass setup. That reselects the
ordinary delayed source state after a terrain shader reset/aborted pass;
recovery must not force-clear `ShaderDirty` or mutate edge state directly.
`DX8Wrapper::Draw_Triangles` remains the source application point, as in the
full original owner.

Keep only default base terrain: one map-owned base atlas and alpha alias,
two terrain passes, identity world transform, canonical index/vertex buffer
binding, and no hidden-object draw. Require an active caller-owned source
frame and initialized shader manager. Missing map/edge/frame/buffers,
unsupported texture-disabled or non-default terrain modes, inactive shader,
source-buffer release, injected draw failure, and retry must fail or recover
without stale sampler/buffer owners.

Do not revive full terrain initialization, cloud/lightmap/noise shaders,
shroud projection, shorelines, roads, bridges, props, trees, bibs, water,
effects, dynamic lighting, partial map updates, W3DTerrainVisual loading,
retail data, public-GPU pixels or resize/recreation. Those are later
source-consumer slices; this slice is Recording-only.

## Tests and acceptance

Extend the existing generated flat-terrain probe with an original
`CameraClass`/`RenderInfoClass`, one-class owned atlas fixture and
caller-owned Recording frame. It must witness exactly two `DX8Wrapper::Draw`
indexed submissions after the F7 pass markers, native 16-bit ranges
(`first=0`, `count=6144`, `base=0`, `4096` vertices), the shared base/alpha
handle, and source-degenerate unfilled cell geometry. Injected Recording
failure at pass zero and after the first completed pass must abort with no
stale samplers, then retry to the same two-pass sequence. Keep negative
controls for no caller-owned frame, hidden terrain, texture-disabled/cloud/
light-map modes and post-release map buffers, plus normal teardown/re-entry.

Run focused GCC and Clang source/Recording controls plus identity,
provider-removal and dependency-ledger checks. Complete exact five non-LAN
suites and serial LAN only after the slice stabilizes. No retail root is
read; evidence uses aggregate generated-fixture counts only.

## Commit boundary

One commit: `delivery: M22 slice 07F8 submit original base terrain`.

## Result

Accepted for the bounded generated-fixture Recording submission only. See
the [slice evidence](../../evidence/cnc-generals-zero-hour/milestone_22_slice_07f8.md).
Retail recording and physical-pixel work remain outside this slice.
