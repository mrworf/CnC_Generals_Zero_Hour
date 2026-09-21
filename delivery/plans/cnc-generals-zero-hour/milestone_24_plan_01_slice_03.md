# M24 slice 03: original recorder and deterministic replay

## Goal and observable outcome

The source `RecorderClass` records real game commands and playback/startup `.rep` replays them with matching simulation RNG and CRC checkpoints across GCC/Clang Debug/Release.

## Scope / non-scope

Own original recorder header/command codec portability, validation-before-mode-change, original startup dispatch and CRC witnesses. Do not invent a replay model, remove version/data validation wholesale, or implement network/M22.

## Dependencies, entry and state

Slice 02 stable state. Production recording accepts original `GameMessage`s during update, writes bounded fixed-width Linux replay fields in XDG, and stops/closes. Playback validates complete header/game info/command frames and explicit cross-preset compatibility, then commits mode, map, `MSG_NEW_GAME` and `InitRandom`; command dispatcher advances source state to the same checkpoints.

## Permissions, validation and errors

Local replay basename only. Invalid/truncated header, bad version/data CRC, unsupported command/type/length, frame order mismatch and command CRC mismatch fail closed, restore prior recorder mode/game and close handles. Debug-vs-Release version/build-time differences are characterized explicitly; same data/INI and source protocol remain required. Diagnostics are bounded/noninteractive.

## Expected surfaces

`Recorder.cpp/.h`, original `MessageStream`/CRC paths only as evidenced, startup `GameEngine`/Linux entry narrow hook, test fixtures and identity checks, ledger.

## Tests and commands

- Positive: source command recording and startup playback, map/objects/scripts/AI command transitions, equal frame/RNG/CRC sequences in all four presets; varied client/audio draws leave logic checkpoint equal.
- Negative: invalid/truncated header, wrong version or INI CRC, corrupt argument/frame, replay CRC mismatch and missing file; assert mode/live state unchanged on failed start and no leaked file.
- Focused original-persistence/replay tests, related simulation, Clang ASan/UBSan; four-preset/full suite at gate.

## Acceptance / commit

Actual original command and CRC playback is deterministic; invalid playback never enters mode/changes live state. Commit as `delivery: M24 slice 03 replay original commands deterministically`.
