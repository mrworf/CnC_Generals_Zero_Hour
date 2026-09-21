# M24 slice 02 — transactional original load

Grade: production original-source integration with a project-owned real CkMp scenario; not yet replay, retail, or cumulative M24 acceptance.

The production Linux entry calls original `GameState::saveGame` and `loadGame` around the M21 scenario. The test compares source frame, object/player/team counts, complete GameLogic CRC, and diagnostic component CRCs before and after load. Valid source Xfer traversal, map extraction, object reconstruction, postprocess and staged simulation RNG return the exact checkpoint. This does not substitute the M5 `ZHSG` fixture.

Original `XferLoad` now bounds nested block descriptors and reads. Non-destructive GameState preflight validates required block envelope, map destination ownership/collision, GameLogic TOC templates and object IDs. After preflight, a source-Xfer live snapshot allows rollback through original reset/load/postprocess if the target load fails after reset, during traversal, or after postprocess. Original map extraction is size-bounded and atomically published in XDG; scratch cleanup removes only process-owned maps, not unrelated user `.map` files.

Negative controls mutate actual original save bytes: truncated/trailing file, unknown GameLogic version, oversized embedded-map block, unknown thing-template reference, and load filename traversal. Forced after-reset, traversal, and postprocess faults each restore the exact pre-load checkpoint and leave no rollback file. A pre-existing XDG map with the target name remains unchanged after rejection; an unrelated XDG map survives a successful load. Save write denial and source-input read-only checks remain in the same test.

Validation:

- `cmake --build build/linux-gcc-debug --target zh_original_main -j 8` — pass.
- `ctest --test-dir build/linux-gcc-debug -R '^original_persistence_save$' --output-on-failure` — 1/1 pass.
- `ctest --test-dir build/linux-gcc-debug -L 'original-(simulation|persistence|production|lifecycle)' --output-on-failure -j 4` — 21/21 pass.
- `cmake --build build/linux-clang-sanitized --target zh_original_main -j 8` — pass after correcting a Clang narrowing error in the new checkpoint hook.
- `ASAN_OPTIONS=detect_leaks=0 ctest --test-dir build/linux-clang-sanitized -R '^original_persistence_save$' --output-on-failure` — 1/1 pass. Leak detection is disabled for this legacy original-engine runtime; ASan/UBSan remain enabled.

Four-preset/full-suite, cross-preset replay and gated read-only retail checks remain for slice 04.
