# M6 slice 01: injected SDL event and input state contract

## Goal and observable outcome

Provide a platform event translator that accepts synthetic `SDL_Event` fixtures without initializing SDL. Headless tests prove physical scancode mapping, distinct layout keycodes, mouse motion/buttons/double-click/wheel, UTF-8 text and IME editing, resize/fullscreen/focus/quit events, and deterministic state clearing on focus loss.

## Scope

- Define engine-owned platform event, input snapshot, and translator interfaces.
- Translate the M6 SDL keyboard, text/editing, mouse, focus, resize, fullscreen, and close event families.
- Track held physical keys, held mouse buttons, pointer position, focus, relative capture, and window dimensions.
- Validate text as UTF-8 and reject null/malformed text and invalid composition ranges with actionable errors.
- Normalize flipped mouse wheel direction and retain click count for double-click behavior.

## Non-scope

SDL subsystem/window initialization, system clipboard calls, real cursor/grab operations, GPU creation, rendering, gameplay wiring, and retail content.

## Dependencies and ordering

M1 supplies UTF codecs and fixed-width types; M3 supplies the GPU-free headless boundary. This slice precedes slice 02 because the real window delegates all event semantics to this translator.

## Entry point and end-to-end behavior

A caller constructs `SdlEventTranslator`, supplies an `SDL_Event`, and receives zero or one engine-owned event. Recognized inputs update an inspectable snapshot. Unrecognized SDL events return no event and do not mutate state. Focus loss emits a focus event after clearing held keys, buttons, and relative capture.

## State transitions

- Key down/up inserts/removes a mapped physical scancode; repeat is retained without duplicating state.
- Mouse down/up updates a bounded button bitset; motion preserves absolute and relative coordinates.
- Text input/editing validates UTF-8; editing validates nonnegative cursor/selection bounds or SDL's paired `-1/-1` unset sentinel.
- Focus loss atomically clears held state and capture; focus gain restores only focus.
- Resize requires positive dimensions; fullscreen enter/leave and close become explicit events.

## Authorization and permissions

Not applicable. Translation is process-local, asset-free, performs no I/O, and initializes no SDL subsystem.

## Validation and error handling

Positive tests cover keyboard scancode versus keycode, repeat, text, composition, mouse drag/double-click/wheel, focus transitions, resize, fullscreen, and quit. Negative tests cover unknown scancodes, invalid mouse buttons, malformed/null UTF-8, invalid composition ranges, and nonpositive resize. Errors name the invalid SDL event field. Unknown unrelated SDL event types are safely ignored.

Required commands:

```text
cmake --preset linux-gcc-debug
cmake --build --preset linux-gcc-debug
ctest --preset linux-gcc-debug -L platform --output-on-failure
```

## Expected implementation surfaces

- `include/zh/platform/sdl_platform.h`
- `src/platform/sdl_events.cpp`
- `tests/platform/test_sdl_events.cpp`
- `CMakeLists.txt`
- the governing and both slice plan files

## Acceptance criteria

- Injectable tests cover every scoped event family without an SDL display or initialization.
- Physical identity comes from scancode while the layout-dependent keycode remains separately observable.
- Text is never synthesized from key events.
- Focus loss demonstrably clears held key/button/capture state.
- Malformed boundary inputs fail without partial mutation.

## Commit boundary

One commit contains the complete injected translator, tests, build wiring, governing plan, and both predeclared slice plans. It does not include real window lifecycle behavior.
