# M22 plan 01 slice 08I1: general multi-tile terrain geometry ownership

## Goal and observable outcome

Requires accepted slice 08H. Restore the original
`HeightMapRenderObjClass` partition and update contract for a bounded visual
height map wider or taller than one 32-cell vertex-buffer tile. A generated
map crossing both axes owns the source-derived tile grid, one shared index
buffer, one vertex buffer and CPU backup per tile, exact edge-tile payloads,
and clean two-generation teardown. This closes geometry ownership only;
multi-tile draw traversal remains 08I2.

## Scope and non-scope

Included: checked cell-to-tile dimension calculation, fixed source tile
capacity, zero-initialized pointer arrays and backups, source row/column tile
order, per-tile update intersections and local offsets, all-tile GPU upload,
map/reference publication only after successful construction, partial
allocation/upload rollback, explicit last-row/last-column cell counts, and
re-entry after failure. The source-authored atlas/UV/alpha queries accepted by
08H remain the only terrain payload producer.

Excluded: terrain draw calls, camera recentering, dynamic-light refresh,
partial map mutation, shoreline/roads/bridges/trees/bibs/waypoints, retail
input, scene admission, and Vulkan pixels. No file or network authorization
applies; all fixtures are generated and temporary.

## Entry, state, and ownership

The focused probe constructs the canonical original terrain owner under an
active `RecordingGpuDevice`, opens a generated authored `WorldHeightMap`, and
calls the source `initHeightData`. The implementation derives
`ceil((width-1)/32) * ceil((height-1)/32)` tiles with overflow and configured
resource bounds checked before allocation. Each tile remains unpublished until
its allocation, backup population, upload, and edge binding succeed. On any
exception, every constructed buffer/backup and the shared index buffer are
released exactly once, map/atlas refs return to their prior state, tile counts
return to zero, and the same owner can retry.

`updateBlock` intersects the requested source cell rectangle with every
affected tile and writes using tile-local offsets while retaining map-global
positions and authored UV/alpha values. Empty intersections do not touch a
buffer. Edge tiles preserve their exact valid cell counts; unused fixed-capacity
storage is deterministic and never treated as authored geometry.

## Implementation surfaces

- Canonical CPU branch of
  `GeneralsMD/Code/GameEngineDevice/Source/W3DDevice/GameClient/HeightMap.cpp`.
- Focused generated original-rendering probe/test and its CMake registration.
- Source identity/provider and dependency-ledger expectations only if the
  accepted provider surface changes.
- `delivery/evidence/cnc-generals-zero-hour/milestone_22_slice_08i1.md`.

## Validation and error handling

Positive coverage uses shapes that cross X only, Y only, both axes, and end on
both exact and partial tile boundaries. Assert source row-major tile order,
tile counts, valid edge extents, representative seam/edge vertex payloads,
one shared index allocation, one VB/backup per tile, bounded uploads, two
device generations, and zero resources/refs after teardown.

Negative coverage rejects null/mismatched/degenerate and overflow/resource-
excess dimensions before publication; inject failure at shared-index create,
each tile create/upload/bind position, and authored-payload query. Assert exact
rollback, no stale array member, successful retry, and unchanged accepted
single-tile behavior. Run focused GCC and Clang tests, focused ASan+UBSan with
`detect_leaks=0`, strict host GCC and Clang LSan, the canonical nonretail filter
excluding `gpu|lan|retail`, source/provider/ABI and ledger checks, and
`git diff --check`. Physical Vulkan, serial LAN, and all six canonical suites
are required at aggregate 08I acceptance.

## Acceptance and commit boundary

The generated canonical owner can construct, update, fail, retry, and destroy
general multi-tile terrain geometry without drawing, leaking, reading retail
data, or relaxing later owners. One independently reviewable commit:
`delivery: M22 08I1 own multi-tile terrain geometry`.
