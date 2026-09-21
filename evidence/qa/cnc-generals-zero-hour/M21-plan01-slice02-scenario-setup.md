# M21 plan 01 slice 02: original mission and skirmish setup

## Result

Passed. The production Linux executable has an explicitly bounded M21 scenario profile that enters the original two-phase `GameLogic::startNewGame` path for owned mission and skirmish maps. Both modes reach ready-to-update original player, team, object/module, script/AI-service, client preload, terrain-prop, and recorder-control state without opening a physical device.

## Source-owned witnesses

Each scenario loads the owned `CkMp` map through slice 01, then calls the original `GameLogic::startNewGame` entry twice as required by its source-defined deferred-start protocol. The accepted checkpoint is derived after the second call:

```text
players=2 teams=2 objects=1 props=1
model-preloads=0 texture-preloads=38 recorder-controls=1 devices=0
```

- Player and team counts come from original `PlayerList`/`SidesList` after original `TeamFactory` setup.
- The logic object is an original `Object` using real `InactiveBody` and `DestroyDie` modules, owned by the original neutral team.
- The Linux client now constructs the original device-independent `Drawable` required by `GameLogic::sendObjectCreated`; no W3D draw module or graphics device is requested.
- A second map object has source `KINDOF_PROP`; original `GameLogic` routes it through `TerrainVisual::addProp`, whose Linux adapter validates and owns the logical effect count.
- Original `GameClient::preloadAssets` reaches the Linux display adapter and records 38 texture requests. No model was required by this fixture, so `model-preloads=0` is an accurate source result rather than a fabricated success value.
- Original `RecorderClass::initControls` increments CPU-owned control state after applying the replay-control window operation.
- Mission reports mode 0 (`GAME_SINGLE_PLAYER`); skirmish reports mode 2 (`GAME_SKIRMISH`).

## Negative controls

- Missing map: the Linux terrain adapter propagates a named failure because original `startNewGame` does not inspect the historical boolean result.
- Malformed/truncated map: parsing fails before a setup checkpoint.
- Missing map object template: required non-waypoint/non-light/non-scorch objects fail before original object creation rather than being silently reported as success.
- Missing behavior module: original INI/module parsing fails before scenario entry.
- Removing `Drawable.cpp.o` from the linked original provider archive makes production relinking fail on the required `Drawable` symbols.
- Owned input roots are made read-only and byte-compared after both modes and all negative cases.

## Reached original fixes

Strict sanitizer execution exposed lifecycle defects that only become reachable once a real scenario exists:

- Linux now supplies the original CPU `Drawable` constructor instead of a null-success client provider.
- `PlayerList` destroys live teams while player callbacks remain valid, then clears its global pointer; this matches the actual reverse subsystem order.
- `ScriptList::reset` tolerates sides without an attached script list.
- `SidesList` now clears owned side/team/script state in its destructor and clears its global pointer.
- Linux terrain and terrain-visual adapters reset CPU-owned map/filename state from their destructors.
- On Linux, `GameEngine::execute` preserves the primary thrown update error instead of replacing it with the generic release-crash diagnostic; Windows behavior is unchanged.

## Validation

GCC Debug focused gate:

```text
ctest --test-dir build/linux-gcc-debug \
  -R '^original_simulation_setup' --output-on-failure
3/3 passed: setup, identity, provider removal
```

Related GCC Debug lifecycle gate:

```text
ctest --test-dir build/linux-gcc-debug \
  -R '^(original_simulation_(setup|setup_identity|setup_provider_removal)|original_lifecycle|original_headless_update|original_production_entry)' \
  --output-on-failure
14/14 passed
```

Strict Clang Debug sanitizer gate:

```text
ASAN_OPTIONS=detect_leaks=0:abort_on_error=1:strict_string_checks=1 \
UBSAN_OPTIONS=halt_on_error=1:print_stacktrace=1 \
ctest --test-dir build/linux-clang-debug \
  -R '^original_simulation_setup' --output-on-failure
3/3 passed
```

Leak detection remains disabled for the accepted original-process global-lifetime limitation; ASan address checks, UBSan, strict string checks, explicit negative-state publication checks, and read-only input verification remain enabled.

## Boundary

This slice establishes a ready-to-update original mission/skirmish state and reached CPU consumers. It does not claim multi-tick command/script/AI/victory behavior, renderer hardware/session behavior, full save/replay, or LAN acceptance.
