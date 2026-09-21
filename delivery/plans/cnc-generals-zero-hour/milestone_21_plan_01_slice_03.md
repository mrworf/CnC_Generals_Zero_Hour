# M21 plan 01 slice 03: original simulation behavior and checkpoints

## Goal and observable outcome

Bounded mission and skirmish scenarios accept valid original commands, advance original object/module, script, and AI updates, produce movement/attack effects, and reach victory or defeat with repeatable source-owned checkpoints.

## Scope

Included: original message/command routing, update order, object movement/attack effects, script and AI state, victory/defeat, derived checkpoints, independent client/audio randomness variation, invalid-command controls. Excluded: UI interaction, pixels/audio playback, full replay/save and network synchronization.

## Dependencies and ordering

Requires slice 02's ready scenario state and precedes lifecycle/corpus acceptance.

## Entry point and end-to-end behavior

Tests submit fixture-defined valid commands through the original message stream, advance bounded production updates, and read checkpoints derived from original frames, objects, players, scripts/AI, and victory state.

## Data and state transitions

Commands transition real object/player/script/AI state and produce terminal mission/skirmish outcomes. Invalid commands leave authoritative state unchanged. Client/audio random consumption is varied independently and must not change simulation checkpoints.

## Authorization and permissions

No authorization layer applies. Unsupported network commands fail closed and no socket/session is acquired.

## Validation and error handling

Positive: movement and attack alter original state, scripts and AI advance, and both victory and defeat are observed. Negative: invalid command type/player/object/target and provider removal fail or are rejected without corrupting state. Repeated and random-variation runs compare exact logical checkpoints.

## Implementation surfaces

Expected: original command/game-logic paths only where portability or test hooks are required, production checkpoint interface, owned fixture definitions, focused tests, provider-removal/identity tools.

## Required commands

- GCC Debug focused original-simulation behavior and determinism tests.
- Repeated checkpoint comparison with at least two independent client/audio random activity patterns.
- Focused Clang ASan/UBSan behavior and invalid-command cases.

## Acceptance criteria

- Evidence records actual original object movement/attack, script/AI advancement, and victory/defeat state.
- Invalid commands cannot produce partial success or mutate the accepted checkpoint.
- Simulation checkpoints are identical across repeats and independent client/audio random variation.
- Removing command, script/AI, or victory providers breaks acceptance.

## Commit boundary

Commit the complete original simulation behavior, tests, checkpoint contract, and plan/evidence updates as one slice.
