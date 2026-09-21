# M21 plan 01 slice 01: original map and chunk state

## Result

Passed. The production original runtime and the W3D logical-map constructor now consume one canonical `OriginalMapLoader` implementation. An owned, byte-exact `CkMp` fixture exercises original chunk dispatch and publishes source-owned terrain, world, object, side, team, and script state. Missing and malformed inputs clear partially published state and fail closed.

## Source identity

- `OriginalMapLoader.cpp` owns the shared original `DataChunkInput` callback registrations for `HeightMapData`, `WorldInfo`, `ObjectsList`/`Object`, `PolygonTriggers`, and `SidesList`.
- `LinuxTerrainLogic::loadMap` opens the original cached stream, invokes that loader, then delegates the remaining waypoint/client work to original `TerrainLogic::loadMap`.
- W3D `WorldHeightMap` invokes the same loader for logical-only construction. The former W3D-local callback declarations and bodies were removed, so there is no second reduced parser.
- `original_simulation_map_identity` verifies both call sites, rejects the former duplicate callback names, and verifies that the production executable extracts `OriginalMapLoader.cpp.o` from the linked original provider archive.
- `original_simulation_map_provider_removal` removes that archive member and proves the production link fails without the required provider.

## Owned-map witnesses

The test constructs the documented original binary envelope (`CkMp`, an eight-entry table of contents, little-endian chunk headers, nested chunks) entirely from public schema values. It does not use a substitute production parser or entity model.

- Mission fixture: `HeightMapData` v4 publishes width 3, height 2, border 1, one `(1,1)` boundary, and six height samples.
- `WorldInfo` v1 is consumed through original `Dict::readFromXfer` state.
- `ObjectsList` v3 and nested `Object` v3 construct an original `MapObject` named `OwnedMissionObject` at `(10,20,30)`.
- `SidesList` v2 creates the authored neutral side; original validation creates its team; nested `PlayerScriptsList`/`ScriptList` retains original script-list state.
- A second skirmish fixture replaces the mission object with `OwnedSkirmishObject`, proving reset/re-entry replaces parser state rather than appending it.
- The focused witness prints only derived logical state: `M21 original map: dimensions=3x2 boundary=1 heights=6 malformed=closed`.

## Negative controls

- An inconsistent height sample count is rejected and leaves no map objects or sides.
- A truncated `CkMp` header is rejected.
- A null/missing stream is rejected with empty published state.
- The fixture includes an unknown future chunk, proving original optional-chunk dispatch can skip it without substituting behavior.
- Provider removal prevents the production executable from linking.

## Sanitizer findings resolved

Strict Clang ASan/UBSan exposed three reached original-source portability defects, all fixed in the shared source rather than hidden in the test:

- `SidesList::clear` called `ScriptList::deleteInstance` through null script-list pointers during parse rollback; teardown now guards those nullable original members.
- `DictPairData`'s packed header placed an eight-byte-aligned `DictPair` at a misaligned address on x86-64; its storage now carries `alignas(DictPair)`.
- The packed dictionary key used a one-value enum for arbitrary bit patterns, triggering invalid-enum UB; its storage key is now the source-width unsigned integer type while retaining the zero sentinel and packed layout.

## Validation

GCC Debug focused gate:

```text
ctest --preset linux-gcc-debug -R '^original_simulation_map' --output-on-failure
3/3 passed: original_simulation_map, original_simulation_map_identity,
original_simulation_map_provider_removal
```

Strict Clang Debug sanitizer gate:

```text
cmake --preset linux-clang-debug -DZH_ENABLE_ASAN=ON -DZH_ENABLE_UBSAN=ON
cmake --build --preset linux-clang-debug -j16 \
  --target original_simulation_map_tests zh_original_main
ASAN_OPTIONS=detect_leaks=0:abort_on_error=1:strict_string_checks=1 \
UBSAN_OPTIONS=halt_on_error=1:print_stacktrace=1 \
ctest --test-dir build/linux-clang-debug \
  -R '^original_simulation_map$' --output-on-failure
1/1 passed
```

Leak detection remains disabled for the focused original-engine process because its accepted process-global singleton lifetime is outside this slice; address, undefined-behavior, and strict-string checks remain enabled.

## Boundary

This slice proves original map/chunk state only. It does not claim gameplay object instantiation, command/script/AI advancement, renderer hardware/session behavior, full save/replay, or LAN acceptance.
