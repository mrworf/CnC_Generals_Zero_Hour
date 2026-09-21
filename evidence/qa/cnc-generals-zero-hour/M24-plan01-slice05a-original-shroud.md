# M24 slice 05A — original partition/shroud restoration

## Source cause and correction

The original `PartitionManager::xfer` restores serialized cell shroud, and the source load postprocess retains its exact CRC. The first dirty partition update then calls `Object::onPartitionCellChange` for cells whose derived touch list has not yet been rebuilt, producing redundant look/unlook operations and changing the serialized shroud. The Linux restore transition now rebuilds only unsaved object value/threat maps on that first touch; ordinary subsequent movement still uses the original cell-change path. No snapshot field or CRC component was excluded.

The negative late-corrupt partition case also exposed a partial-load reset fault: `GameLogic::processDestroyList` indexed its sleepy-update array with `-1` before source load postprocess could repair the queue. Failed-load cleanup now clears that partial queue before the original reset/rollback path. The pre-load source checkpoint and both candidate/prior save files remain unchanged after rejection.

## Validation

- Project-owned visible-shroud source scenario: original save/load/re-save/re-load retained exact full source CRC and checkpoint. A partition block with an unsupported version failed closed and rolled back (`rollback=1`), without transaction leftovers or save mutation.
- Read-only retail mission: first and repeated original save/load preserved full GameLogic and partition CRC, frame and source component checkpoint. Input corpus metadata was unchanged; all output was isolated under temporary XDG roots. No retail bytes, paths, names or hashes are included here.
- GCC/Clang Debug/Release: builds and focused `original_persistence_*` plus `original_simulation_*` suites passed (15/15 each). Following the circle arithmetic fix, the owned partition-restore test passed again (1/1 each) in all four presets.
- Clang Debug ASan/UBSan: strict owned visible-shroud exact load and corrupt rollback passed. The fixture newly reached two original `DiscreteCircle` left shifts of negative Bresenham values; equivalent multiplication removed both undefined operations without disabling UBSan. LeakSanitizer itself reports a sandbox/ptrace fatal error here, so this run used `ASAN_OPTIONS=detect_leaks=0` while retaining address checking and `UBSAN_OPTIONS=halt_on_error=1:print_stacktrace=1`.
- Original dependency ledger checker passed. Cumulative four-preset full CTest, retail skirmish/replay and first-tick gates remain later slices.

## Boundary

This slice does not claim the first retail tick or retail skirmish. The first tick has a separately traced shipped `GlobalData` reset defect in slice 05B; the skirmish load reaches the unresolved M22 original draw-provider boundary in slice 05C.
