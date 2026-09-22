# M22 plan 01 slice 07F5: original terrain source-tile bitmap ownership

## Goal and observable outcome

Requires accepted 07F0 through 07F4. During canonical visual
`WorldHeightMap` construction, the one generated texture-class record resolves
through the original `TerrainTypeCollection` and filesystem path, decodes one
read-only authored TGA source tile into canonical `TileData`, generates its
source mip chain, answers bounded CPU tile-data queries, and returns all tile
owners on map destruction and fresh-process re-entry.

## Dependency finding and scope

Native `ParseBlendTileData` invokes `readTexClass` for every texture class.
That method resolves the class name through `TheTerrainTypes`, opens
`Art/Terrain/<texture>` through `TheFileSystem`, counts/reads TGA tiles, and
populates `TileData` before `getTerrainTexture` can assign atlas positions.
Therefore bitmap ownership is mandatory before atlas or material work.

Implement only this Linux CPU source-bitmap boundary using the original class,
path decision, TGA layout and `TileData::updateMips`. Preserve class layout and
native behavior. Do not assign atlas positions, construct base/alpha/edge
`TextureClass` resources, change F3's absent-bitmap behavior for maps whose
class cannot resolve, install terrain material/shader, submit `Render`, publish
`W3DTerrainVisual::load`, or claim terrain pixels. Those remain later children.

## Entry, state, errors and surfaces

The focused test supplies generated read-only Terrain configuration naming one
source texture and one deterministic uncompressed 64x64 RGBA TGA under the
source path. Visual-map construction owns exactly one `TileData`; full-size and
all source mip widths return deterministic corner/average bytes. Destruction
releases the tile before filesystem/service teardown and a fresh process repeats
the lifecycle.

Missing terrain collection, unknown class, missing/unreadable image, invalid
TGA type/depth/dimensions, truncation, excessive tile count and mismatched
class first/count/width reject without a published bitmap owner. Logical-only
construction remains independent. A visual map explicitly using an unresolved
class retains the accepted F3 metadata-only zero-UV boundary rather than
silently substituting a texture.

Expected surfaces are canonical `WorldHeightMap.cpp` and existing `TileData`,
a focused source probe/test, generated config/image fixture, registration,
dependency ledger, plan/index and evidence. Retail inputs and retail symlink
content are untouched.

## Tests and acceptance

Positive tests cover source class/path identity, one owned tile, exact RGBA
bytes, 32/16/8/4/2/1 mip values, two in-process owners, destruction and two
fresh processes. Negative tests cover every entry/error above, allocation
retirement after each rejection, unchanged logical-only behavior, no atlas
resources and zero Recording commands.

Run focused GCC/Clang, identity/provider-removal/ledger controls, exact five
non-LAN suites with leak-capable GCC/Clang ASan+UBSan/LSan, and the established
LAN split. No physical pixel gate applies because this child owns CPU bitmap
data only.

## Commit boundary

One commit: `delivery: M22 slice 07F5 own terrain source bitmap`.

## Result

Complete. Canonical visual-map construction now resolves one authored terrain
class through the original filesystem, owns one decoded 64x64 source tile and
its seven-level mip chain, and retires that owner on teardown and re-entry.
Missing, malformed, unsupported and oversized authored images fail closed;
duplicate authored names retain the source replacement rule. The accepted F3
metadata-only fixture now names an explicitly unresolved four-byte class, and
its truncation control crosses the actual payload boundary, removing its
accidental dependency on baseline terrain configuration. Atlas textures,
materials, draw submission and pixels remain pending. Evidence:
[milestone_22_slice_07f5.md](../../evidence/cnc-generals-zero-hour/milestone_22_slice_07f5.md).
