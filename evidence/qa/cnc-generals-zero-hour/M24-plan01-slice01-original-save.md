# M24 slice 01 — original save publication

Grade: production original-source integration, project-owned fixture; not yet full M24 acceptance or retail evidence.

`zh_original_main` enters the M21 mission scenario, runs original selection/move/attack/invalid-command/victory logic, and calls `GameState::saveGame`. `GameState::init` registers its source snapshot blocks; the new test finds `CHUNK_GameStateMap`, `CHUNK_Players`, `CHUNK_GameLogic`, `CHUNK_ScriptEngine` and `CHUNK_SidesList` in the resulting XDG save. `GameState::getSaveGameInfoFromFile` round-trips normal type, description and map label. An empty save name uses the original numbered autosave filename. The source input tree is unchanged.

Linux publication now validates the leaf, writes a unique transaction file in the XDG save directory and atomically renames it only after source Xfer traversal and close succeed. Embedded map size is checked before allocation. A missing or oversized embedded map fails after snapshot writing has started; the previous save stays byte-identical and no transaction file remains. Invalid `../` basename and write-denied XDG save directory also preserve the prior save. The temporary-file cleanup precedes any UI diagnostic callback, so a noninteractive callback failure cannot strand it. Original XferSave checks `fclose` failure.

Validation on `linux-gcc-debug`:

- `cmake --build --preset linux-gcc-debug --target zh_original_main -j 8` — pass.
- `ctest --preset linux-gcc-debug -R '^original_persistence_save$' --output-on-failure` — 1/1 pass.
- `ctest --preset linux-gcc-debug -L 'original-persistence|original-simulation|original-production-entry' --output-on-failure` — 16/16 pass before the additional write-denial assertion; focused test repeated and passed afterward.

The test uses a sparse project-owned oversized map and retains no private data. Full four-preset, sanitizer, load/replay/CRC and retail acceptance are reserved for later M24 slices.
