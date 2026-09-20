# M20 plan 02 slice 04: integrated headless update and reset

## Goal and observable outcome

The same explicit offline headless profile initializes complete original providers, performs genuine original GameClient and GameLogic updates, resets through original layout/parser resources, and shuts down without acquiring a window, GPU, or audio device.

## Scope and non-scope

Implement bounded test-profile configuration using existing GlobalData semantics: offline, no active match, `-noshellmap`, intro/sizzle disabled, initial map/replay/cache-build/forced-benchmark dispatch cleared, controlled quit only after source updates, then original reset. Preserve normal defaults. Integrate message/script/view, radar, audio, recorder, CD, optional-null network, and process/worker enclosure. Full match/gameplay and physical/interactive acceptance remain later milestones. A test-only headless text object may satisfy update messages, and test-only draw overrides plus a null lexicon may exercise a callback-free owned `BlankWindow`; those bounded harness seams must not enter the slice-05 production executable, which requires actual GameText/localization, FunctionLexicon callbacks, and production factory selection.

## Dependencies and ordering

Requires slices 01-03. Supplies the integrated behavior consumed by final entry/failure and assurance slices.

## Entry point and state transitions

Original init/post-load produces ready state; capture original GameLogic frame and a source-owned client/message transition; `GameEngine::update` advances both; controlled quit exits execute; `GameEngine::reset` loads and destroys `Menus/BlankWindow.wnd`; destruction unwinds providers and process services. Unsupported network entry fails before traffic.

## Permissions, validation, and recovery

No display/audio/network permissions are needed. Interceptors assert zero physical acquisitions. Test normal and repeated runs; update/reset failures; missing BlankWindow/music/resources; UnicodeString/pool use in actual consumers; worker stop-before-owner release; diagnostic preservation. The callback-free reset harness accepts only layouts without callbacks: a callback-bearing owned WND must fail while the null lexicon seam is active. No trace string or test-only counter may substitute for original state.

## Expected surfaces

Actual GameEngine/GameClient/GameLogic update/reset paths, test-profile configuration, null/recording device adapters, owned runtime fixtures, integration tests, ledger/identity controls.

## Required validation and acceptance

Focused integrated runtime passes in all four presets with active assertions, genuine before/after state, original reset, zero device acquisition, null offline network, bounded completion, zero live workers and services, empty owner lists, stable warmed legacy-pool allocation count across reset, and real provider-removal failure. The legacy memory manager is initialized before `main` and retains its pools through process teardown, so a fabricated final-zero raw allocation assertion is not accepted as evidence. Commit as `delivery: M20 slice 04 run original headless lifecycle`.
