# M6 slice 02: SDL window lifecycle and interactive PRE-016 smoke

## Goal and observable outcome

Provide an RAII SDL video/window platform that polls through the slice-01 translator and controls text input, clipboard, cursor, confinement, relative capture, window size, and fullscreen without touching SDL_GPU. A purpose-built smoke executable exercises the interactive boundary and emits a deterministic success report.

## Scope

- Initialize only SDL video/events, create a resizable window, and unwind window/subsystem state safely.
- Expose polling, text input, clipboard, cursor visibility, confinement, relative capture, resize, and fullscreen operations.
- Synchronize requested relative capture with the translator and ensure focus loss disables the actual SDL mode.
- Add `zh_platform_smoke` for automated interactive operations plus injected keyboard/mouse/text/composition/focus/resize/fullscreen/close coverage through the same translator.
- Provide an explicit no-display failure mode naming SDL video/window and `SDL_GetError()`.
- Record PRE-016 evidence including the active video driver and GPU-free proof.

## Non-scope

SDL_GPU or any renderer/device creation, pixels or screenshots, gameplay UI behavior, custom cursor artwork, retail data, audio, and M14 visual acceptance.

## Dependencies and ordering

Requires slice 01's translator. The smoke may inject deterministic events where automation cannot type into the developer's compositor, but it must create and operate a real window, use the real clipboard, pump real lifecycle events, and verify requested window state in the accessible display session.

## Entry point and end-to-end behavior

`zh_platform_smoke` initializes the platform, creates a window, begins text input, round-trips Unicode clipboard text, changes cursor/grab/relative state, resizes and toggles fullscreen, pumps events, injects representative input events through the production translator, verifies focus-loss clearing, then exits and destroys the window/SDL video state. `--expect-no-display` succeeds only when construction fails with an actionable diagnostic.

## State transitions

Construction moves SDL video and window ownership into the platform object. Each operation updates SDL and translator state only after SDL reports success. Focus loss disables real relative mode before publishing the translated event. Shutdown stops text input, releases grab/capture, destroys the window, and releases only the subsystems initialized by this object.

## Authorization and permissions

The interactive command needs access to the user's existing local display and clipboard. It does not access retail data, the network, sound, or GPU APIs. If sandbox display isolation blocks it, rerun the same command with the available approval mechanism and record the exact environmental failure if it persists.

## Validation and error handling

Positive validation covers real window creation, video driver, clipboard UTF-8 round trip, text mode, cursor, confinement, relative mode, resize, fullscreen toggle, event pump, injected input semantics, focus-loss release, close, and reverse-order shutdown. Negative validation removes `DISPLAY`, `WAYLAND_DISPLAY`, and `XDG_RUNTIME_DIR` and requires a diagnostic naming SDL video/window plus the library error. Invalid window dimensions and operations after shutdown are rejected.

Required commands:

```text
cmake --preset <each-supported-preset>
cmake --build --preset <each-supported-preset>
ctest --preset <each-supported-preset> -L platform --output-on-failure
env -u DISPLAY -u WAYLAND_DISPLAY -u XDG_RUNTIME_DIR <smoke> --expect-no-display
<smoke>
```

## Expected implementation surfaces

- `include/zh/platform/sdl_platform.h`
- `src/platform/sdl_window.cpp`
- `tests/platform/test_sdl_window.cpp`
- `tests/platform/platform_smoke.cpp`
- `CMakeLists.txt`
- `evidence/platform/cnc-generals-zero-hour/M6-sdl-platform.md`
- this slice plan and governing plan completion records

## Acceptance criteria

- All four presets pass the `platform` label.
- A real SDL window succeeds in the provided display session and every M6 interaction category is reported.
- The no-display path produces an actionable subsystem/library diagnostic.
- Source/build inspection proves no SDL GPU symbol is referenced by the platform library or smoke executable.
- Evidence contains no private retail content or absolute private paths.

## Commit boundary

One commit contains the completed SDL window boundary, real/no-display tests, interactive evidence, and plan reconciliation. It preserves slice 01 and all M0-M5 behavior.
