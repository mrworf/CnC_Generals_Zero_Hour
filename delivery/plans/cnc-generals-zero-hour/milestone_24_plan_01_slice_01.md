# M24 slice 01: original snapshot publication

## Goal and observable outcome

An M21 production scenario saves through original `GameState::saveGame` and all original `SNAPSHOT_SAVELOAD` blocks to an XDG save file; autosave metadata is readable. A failed write/invalid filename never replaces an existing save.

## Scope / non-scope

Own original XferSave/GameState/GameStateMap save path, bounded source-established fields/map embedding, filename/output transaction, source-owned save witnesses and default tests. Do not implement load/replay or M22 rendering. `ZHSG` is fixture-only.

## Dependencies, entry and state

Requires M21 scenario and M5 codec provider. Production bounded headless scenario entry requests save after original command/update; `GameState` xfers original map, players, objects, scripts, AI, client and partition blocks. State remains live after save. XDG data holds the committed save and any temporary is private/removed on error.

## Permissions, validation and errors

No user authorization is needed in a local game. Validate basename, type/version, lengths and total map size before allocation. Reject traversal/write failure, path traversal, missing embedded map and write denial noninteractively. Never write retail/CWD or clobber an existing save on failure. Source warnings and diagnostics remain bounded.

## Expected surfaces

`GameState.cpp/.h`, `GameStateMap.cpp`, `XferSave.cpp`, Linux production scenario hook, CMake/test `tests/original_persistence/*`, ledger only as reached behavior changes. Investigate actual original state dependencies before editing.

## Tests and commands

- Positive: owned mission/skirmish save, all required original block tokens, real map/object/player/script/AI fields, metadata/autosave filename and unchanged live checkpoint; XDG-only path.
- Negative: invalid leaf, missing map, bounded oversized map, injected short write/denied directory, old save unchanged and no temporary leak.
- Focused CTest `-L original-persistence`, related `original-simulation|original-production-entry`, applicable build; four-preset/full-suite at milestone gate.

## Acceptance / commit

Source-linked snapshot save and atomic publication pass positive/negative tests. Commit behavior, tests, plan update and slice evidence together as `delivery: M24 slice 01 publish original saves`.
