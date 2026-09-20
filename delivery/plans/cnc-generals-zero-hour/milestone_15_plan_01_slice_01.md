# M15 slice 01: session lifecycle and owned persistence

## Goal and observable outcome

A caller can start a bounded campaign, skirmish, or user-map session from any current directory, use explicit/XDG writable roots, advance deterministic gameplay, save/load and write/verify a replay, record progression, and shut down cleanly. Corrupt or missing owned files are rejected with actionable diagnostics and do not mutate the active session.

## Scope

- Introduce the single-player request/session API and deterministic lifecycle state machine.
- Resolve config/data/state/cache directories from the existing XDG contract.
- Create only application-owned save, replay, progression, and log directories.
- Drive the M5 simulation/persistence codecs for checkpoints, save/load continuation, replay verification, victory/defeat, and progression.
- Use atomic writes and bounded reads; keep recovered session state unchanged on failed decode.
- Add positive tests for all modes and negative tests for invalid requests, missing user maps, corrupt state, and unwritable destinations.

## Non-scope

UI/render/audio/video integration, retail media, LAN sessions, Windows file import, and real-GPU presentation.

## Dependencies and order

Requires completed M5 and M6 platform paths. It is the foundation for slices 02 and 03.

## Entry point and end-to-end behavior

`singleplayer::Session` accepts a validated request, VFS reference, and environment lookup. `start()` resolves/creates owned roots and selects the scenario. `advance()` produces deterministic checkpoints. `save()`, `load()`, and `write_replay()` operate below owned data. `finish()` persists progression and returns victory/defeat. `shutdown()` is idempotent.

## Data/state transitions

`created -> loading -> playing <-> paused -> victory|defeat -> stopped`; failures enter `error` only when the session cannot safely continue. Loading corrupt save/replay data leaves the pre-call snapshot and state intact. Progression is versioned project-owned text and is replaced atomically.

## Authorization and permissions

No privilege or network is required. Retail VFS roots are read-only. Writes are restricted to explicit/XDG application roots; relative override roots and path traversal are rejected.

## Validation and recovery

Positive: campaign/skirmish/user-map start, deterministic advance, save/load equivalence, replay verification, progression, repeat shutdown. Negative: missing map, malformed save/replay, path traversal, relative state root, unwritable destination, invalid transition. Diagnostics name the subsystem and logical/destination path without leaking private roots into committed evidence.

## Expected implementation surfaces

`include/zh/singleplayer/session.h`, `src/singleplayer/session.cpp`, `tests/singleplayer/test_session.cpp`, `CMakeLists.txt`, and these delivery plans.

## Validation commands

- `cmake --build --preset linux-gcc-debug --target singleplayer_session_tests`
- `ctest --preset linux-gcc-debug -R '^singleplayer_session$' --output-on-failure`
- Related persistence/determinism tests in the same preset.

## Acceptance criteria

- All state transitions and three modes are independently exercisable.
- Linux save/replay round trips preserve the deterministic snapshot and CRC checkpoints.
- Recovery tests prove failed reads do not mutate the session.
- All writes remain beneath owned roots and clean shutdown is idempotent.

## Commit boundary

Commit the complete lifecycle/persistence behavior, tests, governing plan, and all predeclared slice plans together as `delivery: M15 slice 01 integrate single-player state`.
