# M24 slice 05A: exact original mission partition/shroud restore

## Goal and observable outcome

A real retail mission save restores original PartitionManager cells, object sighting state and full GameLogic CRC exactly, including a repeated save/load. A project-owned visible-shroud scenario also round-trips exactly and a late corrupt Partition chunk rolls back. The original source update and postprocess path remains authoritative.

## Scope / non-scope

Own the source PartitionManager/PartitionCell/GameState post-load ordering or state correction required by evidence. Do not skip partition CRC, discard shroud fields, freeze the game before first tick, rewrite saves to a toy format, or alter renderer/M22 hardware. Retail first-tick GameData baseline is slice 05B; skirmish exact load remains slice 05C after the M22 original draw-provider decision, neither an acceptance claim of this slice.

## Dependencies, entry and state

Slice 04 proves shipped special powers survive reset. Retail mission load reaches completion with identical frame, object/player/team counts and GameLogic/PlayerList/SidesList/ScriptEngine/AI component CRCs, but PartitionManager CRC changes across approximately 5.18 million of 6.38 million cells; a second round-trip changes it again. Source `PartitionManager::xfer` serializes cell shroud, then `GameState::gameStatePostProcessLoad` calls `PartitionManager::update`. Stage CRCs prove source snapshot/postprocess retain exact shroud but first dirty update calls `Object::onPartitionCellChange` against restored cells and queues 146 redundant unlooks. Repair that source restore transition while rebuilding unsaved derived maps.

During the retail skirmish probe, a complete original load reached `GameStateMap::xfer -> GameLogic::startNewGame(TRUE) -> ThingFactory::newDrawable -> ModuleFactory::newModule(MODULETYPE_DRAW)` and failed on the M22 unavailable physical draw provider (`m_createProc == NULL`, `ERROR_INVALID_D3D`). That is a new exact dependency, not permission to bypass source rendering. Slice 05C owns skirmish acceptance after M22; 05A remains independently deliverable.

## Permissions, validation and errors

Read-only retail roots; all saves/extracted maps under isolated XDG. Preserve bounded Xfer and rollback behavior from slices 01–02. Test unknown/corrupt partition versions and forced postprocess failure without changing the live source checkpoint. Never accept a result by excluding the partition from the checksum.

## Expected surfaces

Original `PartitionManager`/`PartitionCell`, `GameState` postprocess only as necessary, targeted source-owned fixture, gated retail mission/skirmish test, ledger and slice evidence.

## Tests and commands

- Characterize exact cell/shroud and dirty-list transition before and after source load/postprocess and repeated load; retain a failing control before the correction.
- Positive retail mission save/load/re-save/re-load with identical full source CRC and object/player/team/script/AI/partition checkpoints; project-owned visible-shroud exact roundtrip and negative late corrupt/faulted save remain transactional.
- Focused project-owned original persistence and simulation tests in all presets; Clang ASan/UBSan; read-only retail metadata isolation.

## Acceptance / commit

The real source mission shroud state and full CRC are identical after first and repeated loads, with no data omission or out-of-scope source change. Skirmish and cumulative M24 remain pending. Commit as `delivery: M24 slice 05A restore original mission partition state`.
