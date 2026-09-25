# M22 slice 08L3B0: fresh pathfinder owner and wall admission

`Pathfinder::addWallPiece` now reports rejection for null, duplicate and
full-capacity registration, and `removeWallPiece` reports exact-ID removal
while clearing the retired slot. Wall pieces cannot displace an occupied
bridge-owned numeric wall layer, and bridge acquisition cannot take that slot
after a wall has been registered.

`Pathfinder::tryNewMapFresh` owns one fresh derived-map attempt. It rejects an
already-ready/map-allocated owner and validates registered wall IDs before
allocation. On an allocation or classification failure, it frees only new
zone blocks/tables, ground cells, row pointers and bridge/wall layer cells;
restores the earlier extent, wall height and readiness; and retains the
accepted bridge pointers, layer numbers, terrain list and wall IDs. The
caller can also roll back a successful but uncommitted fresh map. Normal
`Pathfinder::reset`, ordinary `newMap` and gameplay object destruction remain
unchanged. The full authored map-attempt composition remains 08L3B.

The generated direct-source witness uses a project-owned modeled W3D bridge,
two live bridge layers and 127 registered wall Objects. It checks null,
duplicate, nonmember and capacity rejection, exact remove/re-add, and eight
injected boundaries after zone, ground, rows, both bridge layer allocations,
wall allocation, classification and readiness. A stale registered wall ID is
rejected before any derived allocation. Every failure has immediate
derived residual zero, unchanged pre-existing bridge identities and wall
count, and a clean in-process retry. Two positive generations and missing
wall provider rejection complete the focused route. The source fixture is
read-only and no retail data is admitted.

Final-state validation: six complete GCC/Clang Debug, Release and ASan+UBSan
builds passed. All six asset-free canonical nonretail suites passed 264/264
(`-LE gpu|lan|retail`; sanitizer suites used
`ASAN_OPTIONS=detect_leaks=0`). The final generated pathfinder witness passed
again under both sanitizer configurations, and the four focused construction,
borrowed-file, bridge and pathfinder owner tests passed 4/4 under strict host
LSan with GCC and Clang. The isolated physical Vulkan display/factory controls
passed 2/2 with validation, and serial host LAN passed 4/4 in each of the six
configurations. The original dependency ledger checker and `git diff --check`
passed on the final source tree. Earlier mixed-state checks overlapped binary
linking and a ledger update; they were not used as acceptance results and
were superseded by the clean frozen-tree runs. The unrelated
`tests/renderer/test_bgfx_device.cpp` diagnostic remained unstaged.
