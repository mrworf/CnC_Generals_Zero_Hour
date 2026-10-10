# N2 plan02 slice02 — generated H1 map admission

Source base: `2f75d080d46073fb9d4df0e4417ff94d6b10e48b`, with the
slice02 changes committed together with this report. Configuration: renderer
off, Debug, local generated input, separate user storage. No retail input was
used. This is H1 map admission only; 120 steady updates, repeated checkpoints,
fault terminals and supplied-data startup remain unaccepted.

## Reached original path

`NativeHeadlessGameEngine::createGameLogic` supplies a `GameLogic` subclass
whose terrain factory retains original `NativeTerrainLogic` and CPU
`NativeTerrainVisual` processing. The fixture requests
`GameLogic::startNewGame(FALSE)`, observes its pending-load state, then enters
`GameEngine::update` → `GameLogic::update` → original `startNewGame` map load.
`NativeTerrainLogic::loadMap` reaches `NativeLogicalWorld::load`, the original
side/script parsers, original object definition/module factory and the loaded
object path. Headless checks skip only absent load-screen, transition, control
bar, replay controls, asset display preload and visual shroud presentation.
Logical terrain, radar sampling, partition cells, actor creation and script
registration still run. The fixture retires the entire loaded engine.

The generated map is 65×65 with the authored two-cell boundary and distinct
raw plateaus 20 and 24. Its active side and team own exactly one persistent
and one expiring actor at distinct interior positions; both receive distinct
source IDs. The persistent actor has no deletion behavior. The expiring actor
has exactly one original `DeletionUpdate` module from parsed INI. The named
script is registered. These are direct runtime assertions before steady ticks.

## Rejection, correction and ownership

The same test process rejects a truncated map, corrects it and admits H1;
rejects a nonfinite actor coordinate encoded as a defined 32-bit raw field,
corrects it and admits H1; and rejects an unknown behavior module, restores
the object definition and admits H1. Every rejection retires the original
root. `ThingTemplate::parseModuleName` now turns a missing module data entry
into the existing INI rejection instead of dereferencing it. `ScriptList::reset`
skips absent side script lists. `NativeTerrainLogic` destructor runs its own
reset so loaded `MapObject` properties retire before pool shutdown. After each
successful loaded-world retirement, the Object, MapObject, ScriptList and
DeletionUpdate pools have zero used units. GCC/Clang leak sanitizer reports
are clean for both H1 cases.

## Reproduction and bounds

```sh
cmake --build build/original-core-gcc --target original_native_headless_root_fixture -j4
ctest --test-dir build/original-core-gcc -R '^original_native_headless_root_(h1-map|h1-map-negative)$' --output-on-failure
cmake --build build/original-core-gcc-sanitize --target original_native_headless_root_fixture -j4
ctest --test-dir build/original-core-gcc-sanitize -R '^original_native_headless_root_(h1-map|h1-map-negative)$' --output-on-failure
cmake --build build/original-core-clang-sanitize --target original_native_headless_root_fixture -j4
ctest --test-dir build/original-core-clang-sanitize -R '^original_native_headless_root_(h1-map|h1-map-negative)$' --output-on-failure
```

Normal GCC: 2/2 H1 cases and 43/43 related native-root, logical-world and
terrain cases passed. GCC ASan/UBSan/LSan: 2/2 H1 cases passed. Clang
ASan/UBSan/LSan: 2/2 H1 cases passed. The final exact actor-ID, module-count
and zero-used-pool assertions passed all three configurations. No full N2
frozen matrix is claimed.
