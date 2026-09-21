# M21 plan 01 slice 03: original simulation evidence

## Accepted behavior

The production `zh_original_main` scenario profile loads the owned map through the original M21 map path and then submits original `GameMessage` commands to `TheCommandList`. The exercised sequence is selected-group creation, move, invalid force-attack, valid force-attack, and player self-destruction. Seventeen calls to the original `GameEngine::update` advance game frames, AI, scripts, command dispatch, objects, players, and victory conditions.

Both fixture modes pass:

- Mission observes player B's defeat through `VictoryConditions::hasSinglePlayerBeenDefeated`.
- Skirmish observes player A's victory through `VictoryConditions::hasAchievedVictory`.
- Movement is accepted from the original `AIUpdateInterface` moving state (the short bounded window does not invent a coordinate change).
- Attack is accepted from the original AI command-source transition to `CMD_FROM_PLAYER`; the invalid target first leaves the source at `CMD_FROM_AI`.
- AI and script counts are file-local observer instrumentation only. No original class member, serialized state, or layout was changed, and acceptance also requires the existing object/player/AI/victory transitions above.

`VictoryConditions.cpp` needed one narrow reached-path correction. A directly launched Linux `GAME_SKIRMISH` does not traverse the Windows shell that would install `TheSkirmishGameInfo`, so `Recorder::isMultiplayer()` remains false. The original victory cache/update previously returned before reading source-owned players. Direct `GAME_SKIRMISH` now enables that existing original cache/update path while all recorder multiplayer semantics remain unchanged.

Other reached corrections preserve original semantics while making already-valid partial objects safe: bodyless drawables retain their pristine ambient state, bodyless object CRC uses the original neutral values, and the original index-based sleepy-update heap avoids forming out-of-range vector pointers under checked/sanitized standard libraries.

## Determinism and negative controls

For each mission and skirmish run, the full logical checkpoint is compared under independent client/audio random burns `(0,0)`, `(19,0)`, `(0,23)`, and `(31,37)`. All checkpoints are exactly equal. The owned source tree is made read-only and its bytes are compared before and after every matrix.

The behavior test rejects missing actor, target, owner, player, AI provider, non-advancing frames, absent AI/script work, invalid-command mutation, absent attack acceptance, and absent terminal state. Identity requires production references to the original messages and linked `GameLogic`, `GameLogicDispatch`, `AI`, `AIUpdate`, `ScriptEngine`, and `VictoryConditions` objects. Provider removal deletes `GameLogicDispatch.cpp.o` from an otherwise identical link and requires the original `logicMessageDispatcher` dependency to fail closed.

## Validation

- GCC Debug focused suite: six of six passed: `original_simulation_setup`, `original_simulation_setup_identity`, `original_simulation_setup_provider_removal`, `original_simulation_behavior`, `original_simulation_behavior_identity`, and `original_simulation_behavior_provider_removal`.
- Clang Debug ASan/UBSan focused suite with `ASAN_OPTIONS=detect_leaks=0`: the same six of six passed. Leak detection remains disabled only for the repository's documented original-engine static lifetime limitation; address and undefined behavior instrumentation remained enabled.
- `git diff --check`: passed.

This slice makes no renderer hardware/session, full save/replay, or LAN acceptance claim.
