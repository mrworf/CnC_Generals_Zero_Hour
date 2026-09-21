# M24: original saves, replay, and deterministic state

## Outcome

The production Linux original engine saves and restores a real source-engine scenario, including map, objects, player/team, scripts and AI state; records and replays original commands; and yields matching source-owned checkpoints and CRCs on the four x86-64 presets. Invalid files leave the live game and recorder unchanged.

## Delivery-goal context

- Goal: `workflow/delivery/state.yaml`; product ID: `cnc-generals-zero-hour`.
- Active packet: `delivery/milestones/cnc-generals-zero-hour/status.yaml`, packet fingerprint `sha256:21ce030a34f700bbcf7773983454f0b741f384956f1ff6568d7fc6782bc5bfe9`.
- Source transaction: `cb567c223beab5d63fa2f13d66718e670271d3d0`; planning transaction: `cd2389e40aa89b1c2d1e02be638d084a95438f2d`.
- Fixed goal membership: M26, M27, M28, M20, M21, M22, M23, M24, M25, M15, M16, M17, M18. This plan owns M24 only.
- Resume mode: new; transaction-start `956810011be853e7fee7d23f70db6872a44386be`.

## Governing contracts

- [M24](../../milestones/cnc-generals-zero-hour/M24-original-persistence.md), [base plan](../../../docs/zero-hour-linux-port-plan.md) §§1–6, 10, [migration supplement](../../../docs/zero-hour-source-engine-migration.md) SE-007/SE-010, and [runtime reconciliation](../../../docs/zero-hour-runtime-closure-reconciliation.md) RC-002/003/004/007–010.
- Accepted direct providers: M21 original map/simulation and M5 native component codec/fixture evidence. M20 owns source `GameState`, `GameStateMap`, `Xfer*`, `Recorder` and CRC availability during lifecycle. M27 owns fixed-width/random primitives.
- No applicable `AGENTS.md` was found. Four CMake presets and full asset-free CTest are canonical validation; Clang Debug ASan/UBSan and separately gated read-only retail are additional acceptance.

## Current-state findings

- M21 production `zh_original_main` can enter, advance and re-enter original mission/skirmish scenarios with source-owned checkpoints. The headless profile has no physical device and uses isolated XDG roots.
- Original `GameState::init` registers 17 snapshot blocks and six deep-CRC blocks. `GameState::loadGame` currently resets the engine before parsing; failures clear it again, so the requested unchanged-live-state guarantee is not present. `GameStateMap` writes an extracted map directly and has unbounded allocation/error paths.
- Original `XferLoad` throws on short data reads but does not enforce block extents and its `beginBlock` returns zero on a short descriptor. Original `Recorder::playbackFile` changes mode/clears game before header validation; header and command readers have unchecked reads and native-width/string assumptions.
- M5 `ZHSG` remains a component fixture, not a game save or source-engine witness. No M24 production save/replay scenario tests yet exist. M22 hardware rendering is not an M24 dependency and its currently blocked work is excluded.

## Decisions

- Keep original `GameState`/`GameStateMap`/`Xfer`/`Recorder` traversal, block sequence, command dispatch and CRC authoritative. Narrow Linux portability/bounds corrections may be shared by original targets; do not replace source state with `ZHSG` or a parallel snapshot model.
- Validate a candidate save before destructive reset, then retain a source-Xfer backup of the live state for rollback through the same original load/postprocess path. A failure during reset, traversal or postprocess restores that backup and its map; no mixed state or scratch-map leak is acceptable. If source traversal exposes a state that cannot be recovered transactionally, stop and report the exact authority/dependency issue rather than claim success.
- Save output and extracted maps use atomic publication inside XDG data. Guard all map and save lengths before allocation. Unknown source blocks remain skippable only within validated bounds; unknown required versions fail closed.
- Replay validation is pre-commit: bounded fixed-width header/command decoding and compatibility checks precede mode change, game clear, RNG initialization and `MSG_NEW_GAME`. Debug/Release build strings may differ, but the characterized Linux compatibility rule must retain relevant data/INI/game-format checks and reject actual incompatible or corrupt files.
- Project-owned map/replay inputs supply default tests. Retail scenarios are separately gated, read-only, and produce only derived logical evidence. No Windows fixture is required.

## Scope

### Included

- Real original full save, autosave metadata, map embedding/extraction, Xfer traversal and postprocess with bounded I/O, transactional failure and XDG isolation.
- Original command recording/playback, validated startup `.rep` dispatch, RNG/CRC checkpoints and simulation-vs-client/audio random isolation.
- Failure injection after reset, during traversal and postprocess; invalid headers, versions, references, truncated/corrupt data and replay mismatch.
- Four-preset original-source tests and full suites, sanitizer, identity/provider-removal checks, read-only retail gate and source ledger refresh.

### Excluded and deferred

- M22 renderer backend and physical scene, M23 interactive media, M25 LAN, optional M18 Windows import; ARM64, toy `ZHSG` compatibility and private retail redistribution.

## Slice index

| Slice | Plan | Outcome | Dependencies | Status | Commit | Evidence |
|---|---|---|---|---|---|---|
| 01 | [slice 01](milestone_24_plan_01_slice_01.md) | A real scenario writes a bounded, atomically published original snapshot and autosave metadata | M21/M5 | complete | this slice commit | `evidence/qa/cnc-generals-zero-hour/M24-plan01-slice01-original-save.md` |
| 02 | [slice 02](milestone_24_plan_01_slice_02.md) | Original load restores source state; malformed/faulted loads leave exact pre-load state | 01 | complete | this slice commit | `evidence/qa/cnc-generals-zero-hour/M24-plan01-slice02-original-load.md` |
| 03 | [slice 03](milestone_24_plan_01_slice_03.md) | Original commands record/play back safely with compatible cross-preset CRC/RNG | 02 | complete | this slice commit | `evidence/qa/cnc-generals-zero-hour/M24-plan01-slice03-original-replay.md` |
| 04 | [slice 04](milestone_24_plan_01_slice_04.md) | Source identity, retail and four-preset cumulative M24 acceptance | 03 | pending | | |

Four slices are needed because source snapshot publication, destructive-load rollback, recorder protocol, and cumulative cross-preset/retail acceptance have distinct failure/review boundaries.

## Cross-slice concerns

- Compatibility: source-defined block layout, explicit fixed widths, UTF-16LE and bounded lengths; no implicit Windows import promise.
- Security/privacy: XDG-only writes, read-only retail, no committed private names/paths/hashes/bytes, no unbounded allocation from file fields.
- Lifecycle: failed load/recording/playback must close files, remove private transaction temporaries and restore mode/state; reset remains original-engine-owned.
- Observability: source state and linked object identity, command/RNG/CRC witnesses, not counters from a substitute store.
- Performance: large maps and snapshots stream or enforce characterized limits; test failure paths as well as success.
- Environment: default tests are asset-free/headless; retail gate has explicit local provisioned roots, no GPU/window required.

## Milestone completion gate

- Every slice plan, test, evidence and commit is complete; original source identity/owner-removal controls pass.
- Full original map round-trip, autosave, replay, corruption, rollback and deterministic cross-preset checks pass; client/audio random burns do not alter logic CRC.
- Applicable targets build and the full asset-free CTest suite passes in all four presets; relevant Clang ASan/UBSan passes with limitations disclosed.
- Read-only retail save/replay gate passes without writes to the retail root or private details in evidence; dependency ledger/source-drift checks pass.

## Rollback and recovery

Each slice is reviewable/revertible. No rollback command may alter user saves or retail assets. Runtime transaction files live under isolated XDG state and are removed or atomically published; failed load restores in-memory source-owned state from a validated original-Xfer backup.

## Execution notes

Planning completed before production edits in `64aeb22`. Parent orchestration owns milestone/workflow status. Slices 01–03 source save/load/replay passed; cumulative acceptance remains slice 04.

## Deferred follow-ups

M22 renderer backend, M23 interactive behavior, M25 network and M18 Windows import remain separate.
