# M21 plan 01 slice 02: original mission and skirmish setup

## Goal and observable outcome

The production original engine enters bounded mission and skirmish scenarios from slice-01 maps and creates source-owned players, teams, objects/modules, script/AI services, and reached client CPU resources.

## Scope

Included: `GameLogic::startNewGame`, original side/player/team/object setup, initial object modules, GameClient preload, TerrainVisual props, Recorder control initialization, mode-specific state, and setup failure unwind. Excluded: multi-tick commands/victory, hardware presentation, full replay behavior.

## Dependencies and ordering

Requires slice 01's transactional map state. Must finish before simulation advancement in slice 03.

## Entry point and end-to-end behavior

The M21 request selects mission or skirmish, runs the production new-game path, and reaches a ready-to-update state with nonempty original objects and mode-correct player/script/AI ownership. Initial `.map` dispatch is not bypassed.

## Data and state transitions

Parsed map state becomes active original game state. Prop/preload/recorder operations record CPU-owned logical effects while physical device attempts remain zero. Setup failures revert all map/game/client state.

## Authorization and permissions

No authorization semantics apply. No physical device or network session is opened; test roots remain isolated.

## Validation and error handling

Positive: mission and skirmish start with expected original player/team/object/module and CPU-consumer state. Negative: missing object template/module, invalid side/team ownership, unavailable reached provider, and injected setup-stage failure reject partial success and unwind exactly once.

## Implementation surfaces

Expected: production Linux factory adapters, original GameLogic/GameClient/TerrainVisual/Recorder CPU methods and hooks, original-simulation tests/fixtures, CMake target wiring, identity/ledger controls.

## Required commands

- GCC Debug focused original-simulation setup tests and all related original-lifecycle tests.
- Focused Clang ASan/UBSan setup/failure tests.
- Provider-removal and source-identity checks.

## Acceptance criteria

- Mission and skirmish reach real original state with nonempty objects/modules, players/teams, scripts and AI services.
- `preloadAssets`, `addProp`, and `initControls` execute meaningful CPU ownership/state operations when reached.
- No label-only witness, toy entity, null success provider, GPU/window/audio-device/network acquisition, or partial-state success exists.

## Commit boundary

Commit production setup behavior, adapters, tests, plan status/evidence, and directly required source-ledger changes together.
