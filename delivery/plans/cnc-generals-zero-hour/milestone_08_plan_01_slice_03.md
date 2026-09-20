# M8 slice 03: SDL-driven UI controls and representative flows

## Goal and observable outcome

Injected SDL-derived platform events navigate and edit existing-style UI controls while normalized streams demonstrate all M8 menu/text/focus/resize cases through the recorder.

## Scope and non-scope

Implement a bounded UI control model for physical navigation, pointer activation, UTF-8 text input, IME pre-edit/commit, selection/cursor movement, focus loss, viewport resize, and representative menu/loading/tooltip/subtitle/caption/save/player/game/chat flows. Connect it to slice 01 scene recording and slice 02 layout. Controller UI, online service replacement, redesign, GPU pixels, and interactive device acceptance are excluded.

## Dependencies and ordering

Requires slices 01 and 02 plus the M6 `PlatformEvent` contract. This final slice completes the end-to-end M8 paths and milestone validation.

## Entry point and end-to-end behavior

`UiSession::handle` receives translated platform events. Physical scancodes navigate controls; layout-dependent shortcut keys are limited to explicit shortcuts; only text-input events commit text and text-editing events update pre-edit state. Focus loss clears pre-edit/held UI state. Resize updates viewport and triggers recorder attachment recreation. `record_flow` emits named menu, loading, tooltip, subtitle/caption, save/player/game name, and chat streams; Internet actions resolve locally immediately.

## Data/state transitions

Session state transitions among named screens and loading states, focused controls, committed UTF-8 text, optional composition range/text, focus, and viewport. Invalid events leave prior state unchanged. Focus loss clears composition; focus gain does not synthesize input. Resize is applied only for positive bounded extents.

Authorization is not applicable. The session performs no DNS, socket, HTTP, filesystem write, or privileged action.

## Validation, errors, and recovery

Positive tests cover keyboard/mouse menu navigation, text fields for save/player/game/chat, IME pre-edit and commit, tooltip/subtitle/caption layout, wrap/truncate/fallback, loading transitions, focus loss/gain, resize, and deterministic snapshot normalization. Negative tests cover text capacity, malformed/out-of-range composition, input while unfocused, resize overflow, invalid flow, and unavailable Internet actions. A later valid event proves recovery.

## Expected surfaces

`include/zh/ui/session.h`, `src/ui/session.cpp`, `tests/ui/test_ui_session.cpp`, representative snapshots under `tests/ui/snapshots/`, `CMakeLists.txt`, and governing plan delivery record.

## Validation commands

Build/run `ui_session_tests`; run `ctest --preset linux-gcc-debug -L 'platform|renderer-contract|ui' --output-on-failure`; then build and run that label in all four supported presets. Verify every UI shader artifact exists.

## Acceptance and commit boundary

All named UI/font/input paths and negative boundaries have deterministic automated evidence with no display, GPU, retail asset, or network dependency. Commit as `delivery: M8 slice 03 integrate UI input flows`.
