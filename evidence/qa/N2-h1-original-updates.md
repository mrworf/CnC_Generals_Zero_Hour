# N2 plan02 slice03 — H1 original updates

Source base: `f365a3fd` plus this slice. Renderer-off Debug configurations,
generated H1 map, separate user storage; no retail input. Slice02 admitted
the map and actors. This report qualifies the bounded original 120-update
path, not three-lifetime byte comparison, fault terminals or supplied-data
startup. The fixture calls original `InitRandom(0x5A170001)` after engine
initialization and before the pending map start; `GameEngine::init` otherwise
uses a time-derived seed.

`GameLogic::startNewGame(FALSE)` leaves a pending map start. The first
`GameEngine::update` enters original `GameLogic::update`, completes the map
load, runs `ScriptEngine::update` and exits at source frame 1. The authored
active, non-subroutine H1 group has a true condition and `SET_FLAG` action;
the fixture reads `H1Started` true after that original script processing and
through every subsequent update. A read-only lookup does not allocate a new
flag or invoke the action.

The parsed 1000ms `DeletionUpdate` schedules source frame 30 from frame-zero
actor creation. For each of 120 subsequent `GameEngine::update` calls, the
fixture records frame before and after and requires exactly one increment.
It tracks both source-assigned actor IDs. The expiring actor is present before
the update entered at frame 30 and absent after its end-of-frame destruction;
the persistent actor remains present for all 120 updates. The authored H1
script performs no destruction, and the expiring actor has one behavior that
can request it: `DeletionUpdate::update` calls `destroyObject`, and
`GameLogic::processDestroyList` retires it in that frame. This source path and
the exact deadline bracket distinguish the scheduled callback from harness
deletion. The harness invokes neither the module callback nor object removal.

A separate fixture authors a road marker, which reaches the native terrain
provider's explicit unsupported-road boundary and rejects. That root retires;
the corrected H1 map then completes the same 120-update run in the same
process. The headless provider does not report a fabricated road success.

```sh
cmake --build build/original-core-gcc --target original_native_headless_root_fixture -j4
ctest --test-dir build/original-core-gcc -R '^original_native_headless_root_h1-ticks(-negative)?$' --output-on-failure
cmake --build build/original-core-gcc-sanitize --target original_native_headless_root_fixture -j4
ctest --test-dir build/original-core-gcc-sanitize -R '^original_native_headless_root_(h1-map|h1-map-negative|h1-ticks|h1-ticks-negative)$' --output-on-failure
cmake --build build/original-core-clang-sanitize --target original_native_headless_root_fixture -j4
ctest --test-dir build/original-core-clang-sanitize -R '^original_native_headless_root_(h1-map|h1-map-negative|h1-ticks|h1-ticks-negative)$' --output-on-failure
```

Normal GCC: both tick cases and all 44 related native-root, original-world
and native-terrain regressions passed. GCC ASan/UBSan/LSan: four H1 cases
passed; the final fixed-seed tick cases passed on rerun. Clang
ASan/UBSan/LSan: four H1 cases passed; the final fixed-seed tick cases passed
on rerun. Full frozen N2 matrix and canonical byte comparison remain pending.
