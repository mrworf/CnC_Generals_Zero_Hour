# M15 slice 03: process, retail, stress, and sanitizer acceptance

## Goal and observable outcome

The built `zh_main` process can execute representative single-player scenarios from an arbitrary CWD, with isolated XDG roots and clear errors, while automated stress, compiler/preset determinism, sanitizers, and opt-in English retail checks provide durable M15 acceptance evidence.

## Scope

- Add a `--single-player` process mode with explicit scenario/mode/faction/map/tick/outcome controls and existing data-root selection.
- Add process tests for arbitrary CWD/XDG isolation, missing/corrupt data recovery, and clean exit.
- Add long-game and repeated load/unload acceptance tests with bounded workloads.
- Add opt-in retail M15 checks for representative missions/maps plus logical text, speech, music, effects, and movies.
- Execute four canonical presets, cross-preset CRC comparison, and Clang ASan+UBSan.
- Record stable environment/context/results in M15 QA evidence without private paths or retail data.

## Non-scope

LAN complete matches (M16), distro packaging (M17), and Windows imports (M18).

## Dependencies and order

Requires slices 01 and 02. This is the final M15 acceptance slice.

## Entry point and end-to-end behavior

`zh_main --single-player ...` parses bounded arguments, resolves data/XDG roots independently of CWD, runs `GameFlow`, writes only owned state, prints a stable summary, and returns distinct usage/data/runtime exit codes. Retail tests are registered only with `ZH_ENABLE_RETAIL_TESTS=ON`.

## Data/state transitions

Each process owns an isolated state directory. Repeated runs retain progression and replace saves/replays atomically. Stress runs repeatedly construct/play/save/load/replay/unload sessions. Failed runs preserve prior valid files.

## Authorization and permissions

No privilege or network. Ordinary tests need no display, audio device, GPU, or retail content. Retail tests consume explicit read-only roots; evidence redacts absolute paths.

## Validation and recovery

Positive: arbitrary-CWD campaign/skirmish/user-map, all factions, long game, repeated cycles, four-preset CRC identity, sanitizer pass, selected English retail categories. Negative: bad CLI, absent required data/map, corrupt owned save/replay, unwritable XDG roots, optional media absence, clean partial-failure shutdown.

## Expected implementation surfaces

`src/bootstrap/main.cpp`, `tests/singleplayer/test_process.py`, `tests/singleplayer/test_acceptance.cpp`, optional retail test source/manifest wiring, `CMakeLists.txt`, user documentation, milestone plan checkpoint, and `evidence/qa/cnc-generals-zero-hour/M15-single-player.md`.

## Validation commands

- Configure/build/test all four canonical presets, excluding label `lan` for the M15 mandatory suite.
- `python3 tools/compare_determinism.py` or repository-equivalent report comparison across presets.
- Configure/build/test `linux-clang-sanitized` with ASan+UBSan.
- Configure/build the explicit retail test build and run M15 retail checks.

## Acceptance criteria

- Arbitrary-CWD/XDG process execution and negative diagnostics pass.
- Representative modes/factions and selected English content categories are covered.
- Long/repeated tests complete without leaks, hangs, mutation-on-error, or CRC drift.
- Four presets and sanitizers pass; evidence contains stable logical context only.

## Commit boundary

Commit process integration, stress/retail tests, docs, evidence, and delivery checkpoint as `delivery: M15 slice 03 accept playable single-player`.
