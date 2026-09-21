# M24 slice 05D: original retail skirmish persistence after draw-provider closure

## Goal and observable outcome

The complete original retail skirmish save/load/re-save/re-load path reproduces exact frame, object/player/team/script/AI/partition and full GameLogic CRC without changing retail inputs. This is not eligible for implementation or acceptance until the original draw-provider contract in M22 is resolved.

## Scope / non-scope

Own skirmish GameInfo/MapCache setup and persistence integration only after the required original source draw provider exists. Do not add a synthetic `MODULETYPE_DRAW`, skip map objects, suppress `ERROR_INVALID_D3D`, alter renderer/M22 source, or accept a fixture-only skirmish witness as retail completion.

## Dependencies and blocker evidence

Slice 05B and M22 original renderer backend §9 closure are prerequisites; 05C mission first-tick may be validated in parallel but is also required before cumulative 06. The read-only retail skirmish load reached `GameStateMap::xfer -> GameLogic::startNewGame(TRUE) -> ThingFactory::newDrawable -> Drawable::Drawable -> ModuleFactory::newModule(MODULETYPE_DRAW)`; `ModuleFactory.cpp:648` throws `ERROR_INVALID_D3D` because `m_createProc` is null for the unavailable physical draw provider. Earlier missing MapCache/GameInfo and local-slot setup assumptions have been characterized, but source load cannot finish without the M22 provider. Parent owns dependency/workflow status.

## Permissions, validation and errors

Retail roots remain read-only; all save/map extraction and replay output stays under isolated XDG. Fail closed on unavailable original provider and preserve the source exception as a blocker. No private retail asset bytes, names, hashes or paths in committed evidence.

## Expected surfaces and tests

After M22 closure, retest original skirmish GameInfo/MapCache setup, source map reconstruction, full exact CRC after first and repeated load, original save failure rollback and input metadata invariance in all four presets and Clang ASan/UBSan. Preserve M21 setup/reentry, M24 source-owned replay and mission tests. Update source ledger and skirmish evidence only for paths actually exercised.

## Acceptance / commit

An original retail skirmish roundtrip passes full exact source CRC and recovery gates without bypass or renderer substitution. Then commit one coherent 05D slice; otherwise remain pending with the exact M22 blocker.
