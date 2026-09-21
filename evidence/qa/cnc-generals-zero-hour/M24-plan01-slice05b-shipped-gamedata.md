# M24 slice 05B — original shipped GameData baseline

## Source behavior

The original `GlobalData::newOverride` calls an unimplemented assignment. On Linux this left each shipped/map layer with constructor defaults rather than inherited fields; shallow-copying its `WeaponBonusSet` pointer would instead create double ownership. The source override now copies all ordinary members and deep-copies the owned bonus set. After the installed default and shipped INIs finish, Linux promotes the shipped top value into the subsystem-owned root and destroys the temporary layer; later map.ini overrides still use `INI_LOAD_CREATE_OVERRIDES` and are removed by the original reset traversal.

The owned fixture proves default `PartitionCellSize=16`, shipped `40`, transient map `5`, and reset/repeated reset to `40`. It also checks an inherited unrelated default field and rejects malformed shipped data. If installed parsing fails after linking an override, the Linux loader discards that partial chain before subsystem-root destruction. The fixture is read-only and all outputs use isolated XDG.

## Validation

- GCC/Clang Debug and Release: original persistence/simulation focused suite, including source identity and provider-removal tests, passed 16/16 per preset. Strict Clang ASan/UBSan owned baseline/invalid-definition test passed with address checking and `UBSAN_OPTIONS=halt_on_error=1:print_stacktrace=1`; LeakSanitizer is unavailable under sandbox ptrace and `detect_leaks=0` was used.
- Original dependency ledger and `git diff --check` passed. Full four-preset CTest and retail save/replay acceptance remain the cumulative slice 06 gate.

## Retail dependency reached

Correct source `GlobalData` copying exposes an optimized-tree drawable during retail mission construction, before the first tick: `GameLogic::startNewGame(FALSE):1868 -> ThingFactory::newDrawable -> Drawable::Drawable -> ModuleFactory::newModule(MODULETYPE_DRAW):648`. The original draw factory has no `m_createProc` and throws `ERROR_INVALID_D3D`. This is the same M22 §9 original draw-provider dependency already reached during skirmish load, not a GameData value failure. The earlier source tick without the copied defaults reached `ScriptActions::doBorderSwitch -> PartitionManager::init` with `m_cellSize=1` and a 5371×3871 grid; this slice removes the shipped-baseline loss but cannot claim real retail tick acceptance before M22 closure. No tree suppression or synthetic draw provider was added. Retail mission first-tick and skirmish exact gates remain slices 05C/05D.
