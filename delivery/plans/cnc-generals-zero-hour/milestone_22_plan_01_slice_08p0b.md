# M22 plan 01 slice 08P0B: source tree resource preparation

## Goal and boundary

After accepted 08P0A and the separate 08J1 generated-map correction, prepare
source-owned tree render resources without
enabling the physical factory proc or claiming a frame. The original
`W3DTreeBuffer` type admission resolves a WW3D render object, extracts a mesh
and bounds, packs tree textures and tiles, then fills source vertex/index
buffers. Its current implementation uses raw `DX8Wrapper`/D3D shader,
texture and buffer operations and contains ambiguous `0` error returns and
partial publication. The CPU-only terrain currently has no tree buffer.

Translate only these reached source resources to checked CPU extraction and
Recording ownership: model and texture identity, type bounds/offset,
atlas/tile limits, per-tree mesh vertices/indices, 16-bit index/capacity
bounds and resource upload order. Preserve source winding, UVs, color and
per-type texture decisions; no placeholder texture, hidden skip or generic
triangle substitute. Asset absence, malformed/non-mesh models, exhausted
type/tile/vertex/index capacity and partial allocation/upload must report
failure before type/instance publication. Unwind in reverse and permit clean
retry with the same terrain and with a fresh map generation. Leave existing
model, terrain and shadow resources untouched on failure.

The factory remains fail-closed and no tree draw/shroud/shadow frame is
asserted. Do not load the raw Windows `W3DTreeBuffer.cpp` Direct3D path into
the Linux full graph or weaken any public GPU-edge validation.

## Validation and commit

Generated mesh/type/atlas fixtures prove exact typed geometry and Recording
resource totals, failure at each allocation/upload boundary, owner identity,
zero immediate residuals, removal/retry and two generations. Negative foreign
asset/terrain, non-mesh, missing texture and over-capacity inputs reject.
Run six complete builds and canonical nonretail suites, focused strict host
LSan, physical Vulkan 2/2, serial LAN 4/4 each six, ledger and diff checks
on final source as in 08P0A. Commit one slice:
`delivery: M22 08P0B prepare source tree resources`.
