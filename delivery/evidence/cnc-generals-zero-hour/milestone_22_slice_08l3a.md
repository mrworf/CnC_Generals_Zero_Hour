# M22 slice 08L3A: atomic bridge/tower/layer admission

`Bridge(Object*)` now initializes its list link and layer before any failure,
validates box geometry, road type and every nonempty configured tower name,
then constructs towers with behavior/body interface checks. A failed tower or
injected later failure clears bidirectional bridge/tower links and retires
successful tower Objects in reverse via the accepted construction rollback,
without gameplay destroy hooks. Empty tower names remain optional.

`TerrainLogic::addLandmarkBridgeToLogic` now returns a Boolean admission
result and links its Bridge only after acquiring a non-ground pathfinder
layer. A failed or exhausted layer, or a post-layer fault, retires the tower
graph and exactly that layer. `Pathfinder::rollbackBridgeLayer` is
pointer-matched, including numeric `LAYER_WALL` when it belongs to a Bridge;
an actual wall layer or another Bridge cannot be reset through it. The input
Object remains caller-owned on failure for 08L3B composition. The normal
`TerrainLogic::deleteBridge` gameplay path was not changed.

The generated direct-source witness uses a project-emitted W3D model and
owned bridge/road/tower definitions. It checks coherent source IDs and
bidirectional behavior links, four configured towers and a non-ground layer,
two generations, optional tower names, and clean retries. Missing road type,
tower template, bridge/tower interface and malformed geometry, faults after
each of four towers and after layer acquisition, and real 14-layer exhaustion
all leave no new terrain Bridge, tower link, Object or Drawable owner after
the caller rolls back its input. The failure marker is emitted immediately
before graphics teardown. The exhaustion witness confirms all pre-existing
Bridge pointers and layers remain intact, including the final numeric wall
slot. No authored MapObject traversal or retail source was admitted.

Final-state validation: all six complete GCC/Clang Debug, Release and
ASan+UBSan builds succeeded. All six asset-free canonical nonretail suites
passed 263/263 (`-LE gpu|lan|retail`; sanitizer runs used
`ASAN_OPTIONS=detect_leaks=0`). Focused generated bridge, accepted
construction and borrowed-file tests passed 3/3 under both strict host LSan
configurations. The non-sanitized physical Vulkan W3D display and factory
bootstrap controls passed 2/2 with validation; serial host LAN passed 4/4
for each of the six configurations. The original dependency ledger checker
and `git diff --check` passed on the final source tree. The unrelated
`tests/renderer/test_bgfx_device.cpp` diagnostic remained unstaged.
