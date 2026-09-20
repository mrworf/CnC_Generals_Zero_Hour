# M15 delivery plan: playable single-player

## Authority and transaction

- Milestone contract: `delivery/milestones/cnc-generals-zero-hour/M15-single-player.md`
- Product authority: `docs/zero-hour-linux-port-plan.md`, especially sections 1, 6, 8/M15, and 10.
- Transaction start: `990caffe4a40647b44b586f1fe01e4c42aa0b11d`.
- Product id: `cnc-generals-zero-hour`.
- Retail data remains read-only external input. Plans, tests, evidence, and commits must contain no retail bytes, hashes, inventories, or private absolute paths.

## Outcome

Deliver one native single-player application seam that starts independently of the current directory, resolves XDG-owned writable state, validates selected data, and drives campaign, skirmish, and user-map sessions through loading, play, pause/options, save/load, replay, progression, victory/defeat, media, and clean shutdown. Linux-created persistence and replays remain the required compatibility boundary; LAN completion and Windows import are excluded.

## Design decisions

- Add a `zh_singleplayer` orchestration library rather than embedding game flow in `main.cpp`; both process entry and deterministic tests use the same API.
- Model campaign, skirmish, and user-map as explicit requests with bounded ticks and scenario identity. Campaign and skirmish offer deterministic built-in acceptance scenarios; user maps require a normalized VFS path.
- Use the existing fixed-width persistence/replay and deterministic simulation facilities. Owned files are written atomically below XDG data/state roots, and corrupt files are rejected without mutating the active session.
- Join existing UI/world/effects/audio/video seams through backend-neutral recorders and VFS logical paths. Missing optional sound or movie media is recoverable; missing required scenario data fails with a named subsystem/path.
- Keep ordinary acceptance asset-free. Retail acceptance is separately opt-in and reports stable logical categories only.

## Slice index

1. [Slice 01 — session lifecycle and owned persistence](milestone_15_plan_01_slice_01.md)
2. [Slice 02 — complete frontend and gameplay flow integration](milestone_15_plan_01_slice_02.md)
3. [Slice 03 — process, retail, stress, and sanitizer acceptance](milestone_15_plan_01_slice_03.md)

## Milestone validation

- Configure/build all four canonical presets and run the complete non-LAN mandatory suite.
- Compare controlled determinism reports across all four presets.
- Configure/build/run a Clang ASan+UBSan test build.
- Run the single-player executable from a directory unrelated to source/build with isolated XDG roots.
- Run representative campaign missions and all-faction skirmishes, a user-map flow, corrupt/missing input recovery, repeated load/unload, and a bounded long game.
- In the opt-in retail build, validate selected English text, speech, music, effects, and movies without recording private paths or corpus contents.

## Completion boundary

M15 is complete only when all three slice commits exist and mandatory validation passes. LAN complete-match behavior remains M16. Windows 1.04 persistence import remains M18.

## Delivery checkpoint

- Slice 01: `4bf8002` — XDG-owned lifecycle, Linux persistence/replay, progression, and recovery.
- Slice 02: `a450164` — backend-neutral UI/world/effects/audio/video integration flows.
- Slice 03: blocked — the integration smoke/process/stress/corpus work is valid, but source gameplay-engine execution is absent; see the acceptance evidence.
- Milestone acceptance evidence: `evidence/qa/cnc-generals-zero-hour/M15-single-player.md`.
