# M29 renderer source and command migration ledger

This is a source/API contract ledger, not a claim of bgfx pixels. The checked
`legacy-api-mapping.tsv` classifies 504 source D3D/D3DX identifiers in 13
categories; `tools/renderer_inventory.py --check` fails for unknown or
multiply classified identifiers. The baseline M2/M7–M10 CPU providers and
their tests remain accepted. M30 must replace backend-specific evidence.

| Source category | M29 disposition and required edge | Current grade | Next physical proof |
|---|---|---|---|
| formats | `TextureFormat` preflight; BC1/2/3 remain CPU RGBA8 fallback when public support is absent; unknown formats reject | CPU runtime | M30 format queries/uploads |
| primitives/FVF | Immutable layouts and point-size-qualified point topology; invalid FVF rejects | CPU runtime | M30 buffers, topology and point pixels |
| fixed-function state | `PipelineKey` plus named shader features; unrepresented state rejects instead of being ignored | CPU runtime | M30 bgfx state/stencil/pipeline mapping |
| curved patches | Explicit unsupported error, no implicit triangle approximation | source inspected | No physical path unless new source evidence reopens contract |
| texture-stage state | `SamplerDesc` and shader variants retain combine/projected/bump semantics; unrepresented modes reject | CPU runtime | M30 sampler/uniform texture pixels |
| resources | Opaque generation-checked buffers/textures, bounded uploads, CPU fallbacks | CPU runtime | M30 public bgfx resource lifetime |
| render targets | Full-target pass clear/load and new ordered, clipped `ViewportClearDesc` with independent C/D/S selection | CPU runtime | M30 public ordered bgfx view clear and pixels |
| shader assembly | Repository-owned portable shader families; offline bgfx binary closure is M29 slice 03 | pending | M30 physical shader programs |
| D3DX math | Engine/source math types and coordinate conventions, no graphics type in public headers | CPU runtime | M30 transforms/pixels |
| image/font helpers | Existing CPU image/font providers and fail-closed decode | CPU runtime | M30 upload/display |
| device/lifecycle | `GpuDevice`/resize contract; bgfx public device ownership selected, SDL3 remains platform/input | source inspected | M30 init/reset/loss/present |
| lighting/materials | Bounded frame/material/object/effect uniform contract and named material shaders | CPU runtime | M30 shader binding/pixels |
| legacy wrapper | Source-local `DX8Wrapper` adapter, not a public API | CPU runtime | M22 original scene route after M30 |

The public bgfx API revision selected by the architecture decision exposes
views, rect/clear/framebuffer ordering, resources, state/stencil, uniforms,
texture binding and frame/reset operations; that API inspection is not a
substitute for backend behavior. The original `WW3D::Render` applies the
camera then calls `DX8Wrapper::Clear` inside an active frame. M29 supplies the
source-facing `OriginalGpuEdge::clear_source_viewport` translation and a CPU
draw/clear/draw test, while M22 retains ownership of wiring the complete
original scene call and retail acceptance. No retail content was used here.

Historical evidence classification: M2 inventory and public-header tests are
rerun directly; M7 recorder and M8–M10 UI/world/effects CPU tests are rerun as
affected command evidence. Prior SDL_GPU shader binaries and physical tests
are retained only as historical preservation evidence. bgfx physical evidence
remains M30; retail scene evidence remains M22.
