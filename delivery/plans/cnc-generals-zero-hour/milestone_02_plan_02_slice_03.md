# N2 plan02 slice 03 — Original H1 updates, script and deletion

## Goal and observable outcome

The loaded H1 world completes 120 steady `GameEngine::update` /
`GameLogic::update` frames. The original script sets `H1Started`, and the
original sleepy-module scheduler removes `H1Expiring` at its parsed 30-frame
deadline while `H1Persistent` survives.

## Scope and non-scope

Included: Source frame and pending-load distinction, original update order,
required logical radar/audio/client services, actual script action and
DeletionUpdate callback, observed terrain samples and world cleanup.
Excluded: No harness-driven module callback, skipped client/audio update,
fabricated flat terrain, physical output or cross-compiler checkpoint codec.
The canonical three-lifetime comparison follows in slice04.

## Dependencies and ordering

Plan02 slice02's exact H1 loaded map, actors, script and module registration
accepted and committed. Exact contract: HQ-01–04 and the active N2 milestone.
Preserve source-format and ownership findings in `docs/original-engine-formats.md`.

## Entry point and application behavior

Extend the documented native headless entry or its actual original-runtime
test invocation. Record map-load completion before counting steady frames;
do not rewrite the frame counter or use a replacement simulation loop.

## Data, state and permissions

Generated read-only H1 inputs and separate user storage. Fixed original seed
`0x5A170001`; no wall-clock advance, pause or camera freeze. Observe source
IDs, frame, script flag, scheduler/deletion and terrain without mutating them.

## Validation, errors and recovery

Positive 120 completed updates, one-frame source increments, surviving actor,
script flag and deadline/retirement witness. Negative unsupported provider or
stalled/early frame rejects. A failed tick retires its world and permits fresh
startup; no universal transactional tick rollback is invented. Run focused
affected tests under GCC debug and both GCC/Clang sanitizer builds, then
related tests. Target-specific commands and bounded evidence are recorded
when the executable paths exist. Full frozen matrix waits for N2 acceptance.

## Expected implementation surfaces

Original GameEngine/GameLogic native provider code, generated fixture,
`tools/zero_hour_headless.cpp`, `cmake/OriginalRuntime.cmake`, build docs and
public QA evidence. Stage only owned hunks of pre-existing dirty files.

## Acceptance criteria

- [ ] The original engine completes 120 steady H1 updates and source frame
  advances exactly once per update.
- [ ] Source script processing sets `H1Started`; original DeletionUpdate
  scheduling removes only the expiring actor at its observed deadline.
- [ ] Corrected-retry and ownership checks pass without private data or
  weakening accepted regressions.

## Commit boundary

One reviewable behavior commit after these checks pass; update the governing
index and bounded evidence. This is a separate slice from map admission.
