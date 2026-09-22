# M22 slice 07F1 evidence: map-owned original shroud CPU data

## Source and ownership result

- The canonical Linux `W3DShroud` branch now derives 7x7 cells from the
  accepted generated 8x8 logical `WorldHeightMap`, using the original authored
  cell-size formula and existing class storage without changing class layout.
- Current/final CPU levels start at the configured shroud minimum, preserve
  border reads, accept in-range set/fill/filter operations, and return all grid
  allocations on reset and destruction. Re-entry at double cell size produces
  the expected 4x4 grid in two fresh processes.
- Empty resource reacquire remains the source-equivalent no-resource success.
  Once a map grid is initialized, projection-resource reacquire, render and
  material installation reject explicitly; no draw, texture projection or
  physical shroud pixel is claimed.

## Negative and isolation controls

- Reads/writes before init, null maps, zero cell size, an oversized grid,
  out-of-range writes and duplicate init all reject without publishing or
  changing a valid grid.
- The generated map is prepared in a temporary read-only source root and test
  failures redact private output. Retail inputs and retail symlink content are
  untouched.
- Allocation counts return to the stable map-parser baseline after reset,
  re-entry and destructor; complete process teardown retains the accepted
  absolute allocation check.

## Acceptance gates

- Focused `original_w3d_shroud_data`: GCC Debug 1/1 and Clang Release 1/1.
- Exact-tree non-LAN: GCC Debug 192/192, GCC Release 192/192, Clang Release
  192/192, GCC ASan/UBSan/LSan 192/192 and Clang ASan/UBSan/LSan 192/192.
  Leak detection remains enabled for both sanitizer runs.
- LAN label under established local-socket permission: 4/4 in all five
  configurations.
- Source identity, provider-removal, all three dependency-ledger checks and
  `git diff --check` pass.

Map publication into the terrain owner, visual mesh, active shroud projection,
terrain/effect pixels, retail coverage and the full 07/08/09 boundaries remain
pending.
