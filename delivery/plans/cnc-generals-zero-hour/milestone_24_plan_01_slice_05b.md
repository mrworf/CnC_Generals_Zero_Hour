# M24 slice 05B: shipped GameData baseline and original first mission tick

## Goal and observable outcome

The original retail mission runs its first source `GameEngine::update` without a huge partition allocation, while shipped `PartitionCellSize = 40.0` remains effective after original reset and a temporary map override still clears on reset. Source frame/CRC and lifecycle observations remain deterministic; the tick is not bypassed.

## Scope / non-scope

Own the Linux lifetime of shipped `GlobalData` definitions and the first-tick source test/harness needed to exercise it. Preserve source override semantics, ownership, copy behavior and transient map.ini overrides. Do not change M22 draw factories, substitute a physical renderer, reduce terrain boundaries artificially, or weaken exact source CRC. Retail skirmish remains 05C.

## Dependencies, source trace and state

Slice 05A is required. `GameEngine::init` loads shipped default GameData and then shipped GameData with `INI_LOAD_CREATE_OVERRIDES`; the `GlobalData` root starts with `m_partitionCellSize = 0`, and `GlobalData::reset` deletes all overrides. The retail INI sources contain `PartitionCellSize = 40.0`, but after reset the source value is zero; `PartitionManager::init` clamps this to 1. On first mission tick, `ScriptActions::doBorderSwitch -> TerrainLogic::setActiveBoundary -> PartitionManager::init` then requests a 5371 × 3871 cell grid (~20.8 million cells), faulting in `PartitionCell` construction. Identify and preserve the shipped baseline without reclassifying later map overrides as durable. Investigate any further first-tick source failures revealed after correction.

## Entry, transitions, permissions and recovery

The original Linux bootstrap reads shipped INIs, enters the retail mission, resets/loads the source state, and advances one original update. During reset, shipped definitions persist; map-specific overrides are discarded. No write to retail roots is allowed; XDG owns output. A bad or missing GameData definition must use original parser/validation semantics and must not leave a mixed override chain, leaked/double-owned data, or partial source checkpoint.

## Expected surfaces

`GlobalData` lifetime or Linux bootstrap, a source-owned default/shipped/map-override lifecycle test, gated first-tick retail mission test, original dependency ledger and slice evidence. No M22-specific source/API change.

## Tests and acceptance

- Prove source default before shipped load, shipped `PartitionCellSize = 40.0` after reset, and transient map override reset to shipped value, including a negative invalid/missing-definition path and repeated reset/lifecycle.
- Read-only retail mission enters, saves/loads and runs the first original `GameEngine::update` without 1-unit/20.8-million-cell allocation; record frame, source CRC and bounded partition dimensions, then repeat lifecycle. Do not assert a CRC is equal across different frame counts.
- Run focused persistence/simulation tests in GCC/Clang Debug/Release, Clang ASan/UBSan, source-ledger and retail metadata checks. Stop on a newly reached source failure with exact stack and ownership analysis.

Commit one coherent slice as `delivery: M24 slice 05B retain shipped GameData for first tick` only after positive and negative evidence passes.
