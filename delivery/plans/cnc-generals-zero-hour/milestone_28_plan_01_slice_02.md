# M28 plan 01 slice 02: font, image, and WND resources

## Goal and observable outcome

Original image and window-layout semantics consume owned logical fixtures, bind registered callbacks and a retained native font face, and emit deterministic recorded UI commands. The reset `Menus/BlankWindow.wnd` fixture loads independently; malformed or missing dependencies fail before publication.

## Scope

- Extract source-owned `Image` coordinate/status behavior and bounded WND/layout parsing.
- Bind named WND callbacks through an explicit registry and reject unknown required callbacks.
- Use the accepted M8 font loader/layout and recording renderer rather than device font APIs.
- Track image, font, layout, window, and callback ownership across repeated and partial-failure paths.

## Non-scope

Complete GameWindowManager/GameClient registry, interactive navigation, localized retail font acceptance, hardware rasterization, movie/cinematic behavior, and M25 CRC error layouts.

## Dependencies and ordering

Requires slice 01 lifecycle/resource ownership plus accepted M8 native font/UI components.

## Entry point and end-to-end behavior

The fixture consumer loads a project-owned font through VFS, parses mapped image coordinates/status, registers callbacks, loads `Menus/BlankWindow.wnd` and a representative child-control layout, resolves font/image references, records the UI scene, invokes a callback, and releases all objects.

## Data and state transitions

Logical bytes are parsed into temporary image/layout objects; references and callbacks are validated; only then are resources published. Teardown clears windows/layouts before callback/font/image owners. Failed parse or resolution leaves prior state and live counts unchanged.

## Authorization and permissions

No authorization surface. All reads use caller-owned VFS fixtures; no host font installation or writes occur.

## Validation and error handling

Positive tests cover normalized UV/image dimensions, rotated status, retained font bytes, BlankWindow and child/control callbacks, command ordering, arbitrary CWD, repeated lifecycle, and callback replacement rejection. Negative tests cover malformed/oversized WND/image data, missing image/font/layout, unknown callback/control/unsupported operation, and failure injection.

## Expected implementation surfaces

Source-owned UI extraction unit; provider header/CMake additions; owned UI fixtures/tests; provenance and dependency ledger extensions.

## Required commands

- Build and run cumulative M28 focused tests in all four presets.
- Run provider identity, provenance, and negative input controls.

## Acceptance criteria

- Actual source-owned image/WND/font consumer behavior executes without display/GPU acquisition.
- `Menus/BlankWindow.wnd` is enumerated and loaded from an owned fixture.
- Callback/resource identities and recorded work are observable.
- Success and every failure path restore ownership counts exactly.

## Commit boundary

One commit containing UI provider behavior, fixtures/tests, plan result, and relevant checked-ledger updates. Audio definitions remain excluded.
