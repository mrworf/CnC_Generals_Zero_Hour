# M15 slice 03: process, retail, stress, and sanitizer acceptance

## Goal and observable outcome

The built `zh_main` process can execute representative single-player scenarios from an arbitrary CWD, with isolated XDG roots and clear errors, while automated stress, compiler/preset determinism, sanitizers, and opt-in English retail checks provide durable M15 acceptance evidence.

## Acceptance gap discovered during implementation

The slice initially connected the native subsystem seams to the repository's project-owned deterministic scenario model. That model is useful integration coverage, but it is not the original Zero Hour gameplay/simulation, INI behavior, campaign scripting, skirmish AI, or mission progression required by the milestone contract. Synthetic campaign/skirmish names and successful retail VFS mounting must not be described as playable Zero Hour or representative retail mission completion.

Before any completion claim, investigate the real legacy engine path from retail VFS data through INI/object/AI/map/mission loading, `GameLogic`/script execution, victory/defeat, and the executable loop. Record the concrete missing source targets and interfaces. Implement the bridge only if it is bounded by M15 and the already accepted provider interfaces; otherwise retain the valid CLI/XDG/persistence/stress/corpus evidence, record an explicit planning/architecture blocker, and leave M15 incomplete. Sanitizer success qualifies only the code that actually ran and cannot close this gameplay gap.

## Scope

- Add a `--single-player` process mode with explicit scenario/mode/faction/map/tick/outcome controls and existing data-root selection.
- Add process tests for arbitrary CWD/XDG isolation, missing/corrupt data recovery, and clean exit.
- Add long-game and repeated load/unload acceptance tests with bounded workloads.
- Add opt-in retail M15 checks for representative missions/maps plus logical text, speech, music, effects, and movies.
- Prove those checks execute the original source gameplay engine rather than the project-owned deterministic integration model; retail corpus presence alone is insufficient.
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
- Representative campaign/skirmish acceptance is produced by original gameplay/simulation and mission scripting; synthetic orchestration is explicitly non-accepting.
- Long/repeated tests complete without leaks, hangs, mutation-on-error, or CRC drift.
- Four presets and sanitizers pass; evidence contains stable logical context only.

## Commit boundary

Because source gameplay is not linked, do not create the originally planned acceptance commit. Commit the valid process integration, stress/retail tests, docs, and blocker evidence as a stable partial checkpoint named `delivery: M15 checkpoint integration harness and gameplay gap`; leave this slice and M15 incomplete.
