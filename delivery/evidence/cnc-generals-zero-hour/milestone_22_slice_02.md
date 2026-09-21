# M22 slice 02 — original GameClient CPU presentation ownership

Outcome: the original WW3D scene/light/material providers and GameClient
display ownership slots, asset manager, scene, track system, shadow manager,
and shroud-pass vtables compile and link with the ten full original draw
classes. `zh_original_w3d_full_probe` is a mutually exclusive full-behavior
link configuration; production `zh_original_main` still uses its accepted
M20/M21 schema configuration. No retail scene, recording frame, Vulkan
render, or production display initialization is claimed by this CPU slice.

The owned fixture constructs original `W3DAssetManager`/`RTS3DScene` and
test-publishes their original `W3DDisplay` slots, attaches/detaches an
original `Null3DObjClass`, exercises original TerrainLogic+terrain-track
init/bind/edge/cap/unbind/reset/shutdown, and checks source-owned original
shadow light/color/queue and volume/projection/decal routing. Physical 2D
status-circle creation, active track flush, and shroud material installation
fail as `OriginalW3DDeviceUnavailable`, not silent success. Original derived
shadow geometry/material and original scene device state require slice 04;
required retail resource failure/reset/retry belongs at the scenario
transaction in slice 05, not a global rewrite of AssetManager semantics.

The fixture exposed a genuine 64-bit original WWLib pool error: the first
node overlapped the block-list pointer. Original `mempool.h` now advances by
`sizeof(void*)`; a four-block allocation/return count and actual scene
teardown witness passes on GCC and Clang. The original `W3DDefaultDraw`
headless schema retains its full instance fields; all ten class size and
alignment checks agree between the actual schema/full compile flags. The
CPU color boundary also checks original DX8 ARGB channel order and x87
truncation rather than treating colors as an independent adapter policy. The
original-source identity test reads compile commands plus linked objects,
rejects mixed schema/full objects and removal of every full draw, display,
asset, scene, shadow, track, shroud, or original WW3D scene/light/material
provider.

Validation: `cmake --build --preset linux-{gcc,clang}-debug -j8` passed;
`cmake --build --preset linux-{gcc,clang}-debug --target
zh_original_w3d_full_probe original_w3d_presentation_tests
original_w3d_abi_schema original_w3d_abi_full` passed; full CTest with host
UDP socket access passed 135/135 on GCC Debug and 135/135 on Clang Debug.
Without socket access the two existing LAN tests fail with `Operation not
permitted`; the same two pass 2/2 with host socket access on both compilers.
The source-dependency ledger validation and `git diff --check` passed.

No original retail symlink content was modified. Tests use an owned in-memory
CPU fixture, not a substitute renderer or retail acceptance scene.
