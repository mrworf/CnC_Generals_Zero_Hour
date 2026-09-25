# M22 slice 08L3B: generated authored bridge/wall map attempt

`GameLogic::startNewGame` now has a conjunctive generated-only map phase that
preflights bounded bridge/wall MapObjects, constructs neutral-team Objects in
source order, positions them, attaches the accepted 08L3A terrain bridge or
08L3B0 wall owner, applies bounded Object-local properties, refreshes radar
and commits a fresh derived pathfinder map. The ordinary map loop is unchanged
outside that route; this is not retail admission, ordinary-object traversal
or a visual claim.

The property updater can publish audio/upgrades globally. The generated
attempt therefore rejects those keys before constructing anything and admits
only `originalOwner`, `objectName`, `objectMaxHPs` and
`objectInitialHealth` with their expected types. Duplicate terrain bridge
admission rejects before mutation. On fault, fresh derived allocations are
rolled back first, then wall registrations or exact terrain bridge/layer/tower
owners, then each Object/Drawable through construction cleanup in reverse
source order; the prior radar queue frame is restored. Accepted pre-existing
bridge layers and world list heads remain intact, without gameplay destroy
hooks or world reset.

The read-only project-owned fixture models two bridge towers, one authored
bridge, one wall, terrain, road mapping, W3D model and bounded properties.
Mission and skirmish positive runs prove source order, property effects,
prior-layer identity, deterministic IDs and zero Recording/graphics teardown.
Ten injected construction/publication stages plus duplicate owner,
unsupported property and five absent/malformed dependency cases report
immediate pre-teardown zero Object/Drawable/list/terrain/wall/pathfinder/radar
residuals. A later mission run checks clean process retry. No original retail
symlink or source content is changed.

Validation on the frozen source tree: six complete GCC/Clang Debug, Release
and ASan+UBSan builds passed. All six canonical asset-free nonretail suites
passed 265/265 (`-LE gpu|lan|retail`; sanitizer suites used
`ASAN_OPTIONS=detect_leaks=0`). The generated map-attempt witness passed an
explicit focused rerun under each sanitizer, and both strict host LSan focused
groups passed 5/5. Serial host LAN passed 4/4 in each of six configurations.
The isolated physical Vulkan display/factory controls passed 2/2 with the
Khronos validation layer available (NVIDIA RTX 4070, Vulkan API 1.4.351,
NVIDIA driver 615.71.09, layer 1.4.357). The original dependency ledger
checker and `git diff --check` passed. The unrelated
`tests/renderer/test_bgfx_device.cpp` diagnostic is unstaged.
