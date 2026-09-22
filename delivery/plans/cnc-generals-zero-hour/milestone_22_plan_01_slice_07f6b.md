# M22 plan 01 slice 07F6B: source terrain atlas ownership

## Goal and observable outcome

Requires accepted 07F0-F6A. A generated flat, unblended visual map follows the
canonical `WorldHeightMap::getTerrainTexture` transaction and owns the complete
source-mandatory texture set: packed base terrain atlas, alpha-terrain alias of
that same atlas, and BGRA alpha-edge atlas. Recording witnesses exact extents,
mip uploads, tile placement, shared-resource lifetime, rollback and re-entry.

## Dependency finding and scope

The original call first assigns authored tile/class atlas coordinates, creates
a `TerrainTextureClass` in `A1R5G5B5`, fills its selected mip chain, constructs
`AlphaTerrainTextureClass` as a non-owning alias of that same physical texture,
then creates and fills `AlphaEdgeTextureClass` in `A8R8G8B8`. Even an unblended
map executes this three-object transaction; omitting the alias or edge object
would change source ownership and later UV state.

Compile the canonical `TerrainTex.cpp` in the full original W3D target and add
only its Linux CPU/device branch. Reuse accepted `OriginalGpuEdge` creation,
upload, publication and teardown. Add a narrow alias-publication operation that
shares an already-owned handle without creating or destroying it twice and
rejects missing, cross-generation, duplicate or self-invalid aliases. Do not
add `Apply` material-state behavior, derived terrain submission,
`W3DTerrainVisual::load`, active blends/cliffs, or terrain pixels.

## Entry, state, errors and surfaces

The existing generated authored `Flat` map and read-only TGA fixture enter via
the original filesystem/config providers. The focused probe installs a
production-compatible Recording edge and calls the public map texture getters.
It verifies the 1x1 texture class packs at the original four-pixel inset, the
canonical `TEXTURE_WIDTH` remains 2048, base
atlas rows contain exact A1R5G5B5 conversions, the alpha wrapper resolves to the
same handle, the edge texture is separately BGRA, and mip dimensions/bytes
remain bounded.

Null maps, absent edge/display owner, malformed atlas dimensions, active
blend/cliff metadata, unsupported formats/usages, injected create/upload
failure and duplicate publication fail before the three-object transaction is
observable. A failed attempt leaves getters unpublished and permits retry.
Releasing the map retains the native base-then-alias-then-edge object order:
the base release leaves the shared handle alive for the alias, the alias
release destroys that handle once, and the edge release destroys its distinct
handle. A second map/edge generation repeats cleanly. An edge torn down while
an active procedural source still owns a published texture fails fast rather
than looping; normal map-before-edge teardown remains the required lifecycle.

Expected surfaces are canonical `TerrainTex.cpp`, its source header only if a
layout-neutral test accessor is unavoidable, the full-W3D source list,
`OriginalGpuEdge` shared-owner bookkeeping, the focused generated-map probe,
dependency ledger, plan/index and evidence. Retail inputs and retail symlink
content remain untouched.

## Tests and acceptance

Positive tests cover original coordinate packing, exact base/edge formats and
bytes, complete bounded mip ownership, alias handle identity, getter
idempotence, reverse teardown and two-generation re-entry. Negative tests cover
missing owners/map, unsupported active metadata, bounds/shape, injected
partial failures, duplicate/cross-generation aliasing and retry without stale
resources.

Run GCC and Clang focused source/Recording tests with leak detection, identity,
provider-removal and ledger controls, then exact five non-LAN suites and the
serial LAN split. This CPU/Recording atlas-owner slice makes no physical
terrain draw or pixel claim; F6A already proves both physical packed-format
backends.

The allocation oracle first executes one empty Recording/edge lifecycle before
capturing its baseline. This normalizes the already characterized four
process-scoped DX8/edge bootstrap allocations; every injected failure, retry,
atlas teardown and subsequent generation must still return both device
resources and source allocations to that exact post-bootstrap baseline.

## Commit boundary

One commit: `delivery: M22 slice 07F6B own source terrain atlas set`.

## Result

Complete. The canonical flat-map path now owns the 2048x128 packed base
atlas, a separately refcounted alpha wrapper that aliases the same physical
handle, and an independently owned 2048x1 alpha-edge atlas. Exact source
packing/mips, rollback at every physical allocation or upload stage,
two-generation re-entry, normal map-before-edge teardown and active-owner
fail-fast behavior all pass. Recording observes zero draws; material Apply,
terrain submission, `W3DTerrainVisual::load`, active effects and terrain
pixels remain pending.
