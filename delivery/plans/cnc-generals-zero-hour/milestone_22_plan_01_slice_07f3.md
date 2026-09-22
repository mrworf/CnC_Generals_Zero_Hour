# M22 plan 01 slice 07F3: original visual terrain tile metadata

## Goal and observable outcome

Requires accepted 07F0 through 07F2. A canonical `WorldHeightMap` visual-map
construction path owns the source per-cell tile, blend, extra-blend, cliff and
flip metadata for one generated flat/unblended map. Bounded source queries
return deterministic tile class, UV, alpha, flip and cliff decisions needed by
the next derived terrain-mesh child, then destruction and re-entry return all
CPU allocations.

## Dependency finding and scope

Native `HeightMapRenderObjClass::initHeightData` immediately invokes
`updateBlock`. Its source vertex construction calls `getUVData`,
`getAlphaUVData`, flip/cliff queries and texture-class lookup; the accepted
logical-only F0 owner intentionally has none of those arrays and rejects visual
construction. Therefore derived mesh allocation cannot safely precede this
metadata child.

Implement only the Linux CPU branch's bounded source-equivalent visual metadata
parser and flat/unblended query surface. Preserve class layout and the native
branch. Do not open terrain texture files, construct tile bitmaps or GPU
textures, allocate derived vertex/index buffers, publish
`W3DTerrainVisual::load`, project shroud, or claim terrain pixels. Those remain
typed later children. Retail inputs and symlink content remain read-only and
are not required.

## Entry, state, errors and surfaces

A generated chunky map supplies matching `HeightMapData` and a minimal valid
`BlendTileData` payload using an authored single texture class and no blend or
cliff entries. Visual construction publishes only after both chunks validate.
The owner records bounded cell arrays, bit widths and texture-class metadata;
flat queries accept in-range cells and reject unsupported texture-resource
generation explicitly.

Malformed/truncated chunks, missing or duplicate visual metadata, length and
index mismatches, excessive counts, unsupported blend/cliff records and
out-of-range query inputs reject transactionally. A failed visual parse must
not alter the accepted logical-only constructor. Re-entry uses two fresh
processes and verifies allocation retirement.

Expected surfaces are canonical `WorldHeightMap.cpp`, the smallest shared map
loader representation needed to preserve source parsing, one focused source
probe/test, registration, dependency ledger, plan/index and evidence.

## Tests and acceptance

Positive tests cover exact source dimensions/heights plus tile indices,
texture-class identity, no-blend alpha/UV, false flip/cliff state, ownership,
destruction and fresh-process re-entry. Negative tests cover missing and
duplicate `BlendTileData`, truncation, mismatched lengths, invalid indices and
counts, unsupported active blend/cliff entries, out-of-range queries and all
texture/resource-producing calls. The F0 logical-only positive and visual
negative contracts remain independently covered.

Run focused GCC/Clang, identity/provider-removal/ledger controls, exact five
non-LAN suites with leak-capable GCC/Clang ASan+UBSan/LSan, and the established
LAN split. No physical pixel gate applies because this child owns CPU metadata
and emits no GPU resource or command.

## Commit boundary

One commit: `delivery: M22 slice 07F3 own visual terrain tile metadata`.

## Result

Complete. The canonical visual-map constructor now owns the bounded v8 flat,
unblended cell metadata required by source mesh generation, while logical-only
construction remains independently unchanged. With no tile bitmap loaded, its
zero UV/alpha result is explicitly the native missing-texture result for this
flat fixture, not a general visual-map behavior. Active blends/cliffs and all
texture/atlas resources remain fail-closed. Evidence:
[milestone_22_slice_07f3.md](../../evidence/cnc-generals-zero-hour/milestone_22_slice_07f3.md).
