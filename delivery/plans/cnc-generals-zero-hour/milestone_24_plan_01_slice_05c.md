# M24 slice 05C: original retail mission first tick after draw-provider closure

## Goal and observable outcome

The real retail mission enters through original two-phase `GameLogic::startNewGame`, saves/loads with exact source checkpoint, then advances its first original `GameEngine::update` without huge partition allocation, retaining shipped `PartitionCellSize = 40.0` and bounded partition dimensions. No tick or drawable is bypassed.

## Dependencies and blocker

Slice 05B and M22 original renderer backend §9 closure are prerequisites. A correct `GlobalData` override copy reaches an optimized-tree drawable in `GameLogic::startNewGame(FALSE):1868 -> ThingFactory::newDrawable -> Drawable::Drawable -> ModuleFactory::newModule(MODULETYPE_DRAW)`. The source draw factory has no `m_createProc`, throwing `ERROR_INVALID_D3D` before the tick. Without correct copy, the later tick previously reached `ScriptActions::doBorderSwitch -> PartitionManager::init` using a 1-unit cell and approximately 20.8 million cells. Both behaviors are source-traced; neither is waived.

## Scope, permission and errors

After M22 closure, own only M24 first-tick integration and any newly evidenced original persistence/runtime defect. Do not introduce synthetic draw modules, skip optimized trees, alter M22 source/API, freeze the game, or assert equal CRC across different frames. Retail roots remain read-only; all saves/maps go to isolated XDG. A failed load retains exact pre-load state and no transaction temporary.

## Tests and acceptance

Run a gated read-only retail mission save/load and first original update; record frame, full source CRC, partition cell size and bounded X/Y cell counts, then repeat lifecycle. Cover invalid/corrupt source save and metadata invariance. Validate in GCC/Clang Debug/Release, Clang ASan/UBSan, ledger and applicable source-identity controls. Commit one coherent 05C slice only after real draw-provider closure and positive/negative source evidence; until then remain pending with exact M22 dependency.
