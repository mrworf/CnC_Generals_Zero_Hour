# M24 slice 05: exact original partition/shroud restore

## Goal and observable outcome

A real retail mission/skirmish save restores original PartitionManager cells, object sighting state and full GameLogic CRC exactly, including a repeated save/load. The original source update and postprocess path remains authoritative.

## Scope / non-scope

Own the source PartitionManager/PartitionCell/GameState post-load ordering or state correction required by evidence. Do not skip partition CRC, discard shroud fields, freeze the game before first tick, rewrite saves to a toy format, or alter renderer/M22 hardware.

## Dependencies, entry and state

Slice 04 proves shipped special powers survive reset. Retail mission load reaches completion with identical frame, object/player/team counts and GameLogic/PlayerList/SidesList/ScriptEngine/AI component CRCs, but PartitionManager CRC changes across approximately 5.18 million of 6.38 million cells; a second round-trip changes it again. Source `PartitionManager::xfer` serializes cell shroud, then `GameState::gameStatePostProcessLoad` calls `PartitionManager::update`. Diagnose cell-level before/after state and source dirty/sighting rebuild, then repair the exact source cause.

## Permissions, validation and errors

Read-only retail roots; all saves/extracted maps under isolated XDG. Preserve bounded Xfer and rollback behavior from slices 01–02. Test unknown/corrupt partition versions and forced postprocess failure without changing the live source checkpoint. Never accept a result by excluding the partition from the checksum.

## Expected surfaces

Original `PartitionManager`/`PartitionCell`, `GameState` postprocess only as necessary, targeted source-owned fixture, gated retail mission/skirmish test, ledger and slice evidence.

## Tests and commands

- Characterize exact cell/shroud and dirty-list transition before and after source load/postprocess and repeated load; retain a failing control before the correction.
- Positive retail save/load/re-save/re-load with identical full source CRC and object/player/team/script/AI/partition checkpoints; negative corrupt/faulted save remains transactional.
- Focused project-owned original persistence and simulation tests in all presets; Clang ASan/UBSan; read-only retail metadata isolation.

## Acceptance / commit

The real source shroud state and full CRC are identical after first and repeated loads, with no data omission or out-of-scope source change. Commit as `delivery: M24 slice 05 restore original partition state`.
