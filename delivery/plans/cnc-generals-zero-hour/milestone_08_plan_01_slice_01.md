# M8 slice 01: recorded 2D/UI scene generation

## Goal and observable outcome

Callers can submit bounded UI scenes to `RecordingGpuDevice` and compare normalized command streams containing clip, blend, half-pixel, layer, menu/loading transition, and cursor ordering evidence.

## Scope and non-scope

Implement an engine-neutral UI scene recorder, quad validation/clipping, premultiplied-alpha pipeline selection, stable layer ordering, cursor-last behavior, viewport resize recreation, local unavailable-Internet responses, and all required UI shader variants. Fonts, glyph layout, SDL control integration, actual pixels, and redesign are outside this slice.

## Dependencies and ordering

Requires M7 recording resources and renderer conventions. This first slice creates the UI command surface used by later text and input flows.

## Entry point and end-to-end behavior

`UiRecorder::record` receives a viewport plus ordered `UiElement`s. It rejects invalid dimensions, non-finite coordinates, empty labels, and rectangles that exceed bounded geometry. Valid elements are clipped, offset by the legacy half pixel, sorted by layer with the cursor last, and emitted through the normal recording device. Menu/loading transitions append stable semantic markers. Internet selection returns one local explanation with `network_attempted=false` and never enters a waiting state.

## Data/state transitions

Recorder state transitions from an empty viewport to initialized render targets and buffers, then `idle -> pass -> idle`. Resize destroys and recreates attachments before the next pass. Menu/loading transition state is copied into each snapshot; invalid scenes do not begin a pass.

Authorization is not applicable. No files, display, GPU, or network are opened.

## Validation, errors, and recovery

Positive tests cover clip intersection, blend variants, half-pixel vertices, stable draw order, cursor overlay, menu/loading transitions, resize, and local Internet unavailability. Negative tests cover zero viewport, NaN/oversized geometry, invalid clips, empty labels, and over-capacity scenes; diagnostics name the offending field/element and a later valid scene succeeds.

## Expected surfaces

`include/zh/ui/renderer.h`, `src/ui/renderer.cpp`, `shaders/ui/*`, `tests/ui/test_ui_renderer.cpp`, `CMakeLists.txt`, and the M8 plans.

## Validation commands

Build `ui_renderer_tests`, run it directly, then run the `renderer-contract|ui` label in `linux-gcc-debug`. Milestone-wide four-preset validation follows slice 03.

## Acceptance and commit boundary

Representative menu/loading/cursor snapshots are deterministic and all invalid inputs fail safely. Commit as `delivery: M8 slice 01 record UI scenes`.
