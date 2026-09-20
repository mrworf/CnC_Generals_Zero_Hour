# M20 plan 02 slice 04: integrated headless update and reset

## Goal and observable outcome

The same explicit offline headless profile initializes complete original providers, performs genuine original GameClient and GameLogic updates, resets through original layout/parser resources, and shuts down without acquiring a window, GPU, or audio device.

## Scope and non-scope

Implement bounded test-profile configuration using existing GlobalData semantics: offline, no active match, `-noshellmap`, intro/sizzle disabled, initial map/replay/cache-build/forced-benchmark dispatch cleared, controlled quit only after source updates, then original reset. Preserve normal defaults. Integrate message/script/view, radar, audio, recorder, CD, optional-null network, and process/worker enclosure. Full match/gameplay and physical/interactive acceptance remain later milestones.

## Dependencies and ordering

Requires slices 01-03. Supplies the integrated behavior consumed by final entry/failure and assurance slices.

## Entry point and state transitions

Original init/post-load produces ready state; capture original GameLogic frame and a source-owned client/message transition; `GameEngine::update` advances both; controlled quit exits execute; `GameEngine::reset` loads and destroys `Menus/BlankWindow.wnd`; destruction unwinds providers and process services. Unsupported network entry fails before traffic.

## Permissions, validation, and recovery

No display/audio/network permissions are needed. Interceptors assert zero physical acquisitions. Test normal and repeated runs; update/reset failures; missing BlankWindow/music/resources; UnicodeString/pool use in actual consumers; worker stop-before-owner release; diagnostic preservation. No trace string or test-only counter may substitute for original state.

## Expected surfaces

Actual GameEngine/GameClient/GameLogic update/reset paths, test-profile configuration, null/recording device adapters, owned runtime fixtures, integration tests, ledger/identity controls.

## Required validation and acceptance

Focused integrated runtime passes in all four presets with active assertions, genuine before/after state, original reset, zero device acquisition, null offline network, bounded completion, zero live resources/workers/allocations, and real provider-removal failure. Commit as `delivery: M20 slice 04 run original headless lifecycle`.

