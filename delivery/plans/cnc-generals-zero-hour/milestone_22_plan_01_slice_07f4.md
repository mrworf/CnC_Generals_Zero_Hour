# M22 plan 01 slice 07F4: original flat terrain geometry ownership

## Goal and observable outcome

Requires accepted 07F0 through 07F3. A canonical
`HeightMapRenderObjClass` under the accepted original display and Recording edge
binds one generated flat/unblended visual `WorldHeightMap`, performs the source
zero-extra-pass preprocessing decision, owns bounded source index and vertex
buffers plus CPU vertex backup, records exact tile partition/count/layout and
source coordinates, updates the same map, frees, re-enters and tears down with
zero retained owners.

## Scope and non-scope

Implement only the Linux CPU branch's flat derived geometry ownership needed
before a terrain frame can exist. Preserve class layout and the native branch.
The source default requests extra-pass preprocessing; this child may accept it
only after the F3 metadata proves every extra-blend index is zero, producing the
source-equivalent empty list. Active blend/cliff metadata stays rejected.

Index topology, per-cell four-vertex layout, map-space XY, scaled height Z,
zero missing-bitmap UV/alpha and bounded tile partitioning follow the original
methods. GPU buffers use the already accepted original DX8/`OriginalGpuEdge`
adapter and retain source CPU backup ownership. Do not create terrain bitmap or
atlas textures, install materials/shaders, submit `Render`, publish
`W3DTerrainVisual::load`, project shroud, initialize water/effects, or claim a
physical terrain pixel. These remain typed later children.

## Entry, state, errors and surfaces

The focused probe establishes production-compatible `W3DDisplay` scene/assets,
a Recording `OriginalGpuEdge`, the probe-local accepted partition-cell
prerequisite, and the generated F3 flat visual map. Derived init must publish
the base map/shroud binding and all derived buffers atomically. For 8x8 input it
owns one vertex-buffer tile, the fixed source index topology, a matching CPU
backup and exact 7x7-cell source coordinates/heights. A bounded update preserves
handles/counts while changing the expected backup heights. Explicit free drops
all buffers and the map ref; a second init succeeds.

Null/mismatched/logical-only maps, duplicate init, zero partition size,
unsupported active visual metadata, invalid update rectangles and injected edge
buffer failures reject without partial publication or stale handles. Destruction
retires CPU and edge resources before display/edge teardown.

Expected surfaces are canonical `HeightMap.cpp` and the smallest source-facing
test access needed to verify protected geometry ownership, a focused source
probe/test, registration, dependency ledger, plan/index and evidence. Generated
inputs are read-only; retail inputs and symlink content are untouched.

## Tests and acceptance

Positive tests cover exact source index count/topology, one-tile partition,
vertex count/stride, corner coordinates/heights, zero absent-bitmap UV/alpha,
map ref ownership, bounded same-buffer height update, explicit free, re-entry,
two fresh processes and complete resource retirement. Negative tests cover all
entry/state/error cases above, Recording resource/byte bounds, and continued
failure of `Render`/texture resources.

Run focused GCC/Clang, identity/provider-removal/ledger controls, exact five
non-LAN suites with leak-capable GCC/Clang ASan+UBSan/LSan, and the established
LAN split. Recording must observe only the expected index/vertex resources and
updates, with zero draw commands; no physical pixel gate applies.

## Commit boundary

One commit: `delivery: M22 slice 07F4 own flat terrain geometry`.

## Result

Complete. The original derived owner now creates the fixed source index buffer,
one padded source vertex tile and matching CPU backup, populating exactly the
generated map's 7x7 cells and preserving the remaining capacity as padding.
Recording observes two buffers, bounded re-upload, injected-create rollback,
free/cache retirement and re-entry, with no draw. Atlas, material, render and
pixel work remains pending. Evidence:
[milestone_22_slice_07f4.md](../../evidence/cnc-generals-zero-hour/milestone_22_slice_07f4.md).
