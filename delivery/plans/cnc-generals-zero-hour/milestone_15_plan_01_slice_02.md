# M15 slice 02: complete frontend and gameplay flow integration

## Goal and observable outcome

Campaign, skirmish, and user-map sessions traverse the selected English main menu, loading, play, pause/options, save/load/replay, movie, audio, victory/defeat, and return/exit flows while producing renderer/world/effects/UI/media commands through completed subsystem interfaces.

## Scope

- Add a coordinator that joins M8-M10 and M12-M14 seams to the session lifecycle.
- Cover USA, China, and GLA skirmish identities and representative campaign identities.
- Record selected-locale UI flows and loading/outcome overlays.
- Emit representative terrain/object/effect commands via backend-neutral recorders.
- Request logical speech/music/effect/movie paths through the VFS; absent optional audio/movie records a warning and continues silently/without a movie.
- Propagate pause/focus/options to UI, audio, and video state and cleanly unload between sessions.
- Add positive and negative command-stream/integration tests without requiring retail assets or devices.

## Non-scope

Real retail mission scripting, real-device pixel acceptance already owned by M14, LAN complete matches, and Windows persistence import.

## Dependencies and order

Requires slice 01 and accepted M8, M9, M10, M12, M13, and M14 interfaces.

## Entry point and end-to-end behavior

`singleplayer::GameFlow` starts a `Session`, records main-menu/loading commands, initializes optional media, advances gameplay and render scenes, applies pause/options/focus, performs save/load/replay commands, records outcome/progression, unloads resources, and returns to the menu or exits.

## Data/state transitions

Flow phases mirror the session state. Subsystem warnings are accumulated in bounded diagnostics. Pausing freezes gameplay/media advancement; resuming continues it. Each unload destroys/reinitializes per-session resources while preserving progression and options.

## Authorization and permissions

No network or elevated permission. Input assets are addressed only by normalized VFS logical paths. Media remains optional; required map/scenario inputs fail closed.

## Validation and recovery

Positive: campaign, three-faction skirmish, and user-map command sequences; loading/pause/options/save/replay/outcome/movie/audio markers; repeated flow reuse. Negative: missing optional media continues with warning, malformed logical paths fail, missing required map is actionable, and focus/pause prevents progression.

## Expected implementation surfaces

`include/zh/singleplayer/game_flow.h`, `src/singleplayer/game_flow.cpp`, `tests/singleplayer/test_game_flow.cpp`, `CMakeLists.txt`.

## Validation commands

- Build/run `singleplayer_game_flow_tests` in GCC Debug and Clang Debug.
- Run related `ui|renderer-contract|audio|video|determinism` tests.

## Acceptance criteria

- Each required frontend/gameplay phase is visible in a stable command/event transcript.
- All factions and representative modes are covered.
- Optional media absence is safe and required content absence is actionable.
- Repeated load/unload leaves no live session resources and clean exit is idempotent.

## Commit boundary

Commit the integrated frontend/gameplay behavior and tests as `delivery: M15 slice 02 connect single-player flows`.
