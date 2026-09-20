# M6 SDL platform and input acceptance evidence

Date: 2026-09-19 America/Los_Angeles (2026-09-20 UTC)

## Context

- Host: Arch Linux x86-64 developer system
- SDL: 3.4.14 from the distribution package
- Compilers/presets: GCC and Clang, Debug and Release
- Display accepted: SDL `wayland` driver in the existing graphical session
- Retail data, audio devices, network, Vulkan, and SDL_GPU: not used

The normal restricted command environment could not access a video device and reported `SDL video initialization failed: No available video device`. Running the same executable with approved access to the existing desktop created a real Wayland window and completed PRE-016.

## Automated event and error coverage

Each supported preset configured, built, and passed two `platform` tests:

```text
cmake --preset <linux-gcc-debug|linux-clang-debug|linux-gcc-release|linux-clang-release>
cmake --build --preset <same>
ctest --preset <same> -L platform --output-on-failure
```

Result: 8/8 selected test executions passed. The injectable suite covers scancode/keycode separation, repeat, UTF-8 text, IME editing (including unset ranges), motion, drag/button state, double-click, normal/flipped wheel, focus gain/loss, resize, fullscreen, close/quit, and ignored unrelated events. It rejects unknown scancodes, invalid buttons, null/malformed UTF-8, invalid composition bounds, and nonpositive sizes without partial state mutation.

The GCC Debug full suite also passed 26/26 tests after the platform implementation.

## No-display negative acceptance

With `DISPLAY`, `WAYLAND_DISPLAY`, and `XDG_RUNTIME_DIR` removed:

```text
platform-smoke: expected-no-display-error: SDL video initialization failed: No available video device
```

The error identifies the SDL video subsystem and preserves the SDL library detail.

## Interactive PRE-016 acceptance

`build/linux-gcc-debug/zh_platform_smoke` ran against the existing display and reported:

```text
platform-smoke: video-driver=wayland
platform-smoke: text-input=active
platform-smoke: clipboard=utf8-round-trip
platform-smoke: cursor=hide-show confinement=requested compositor-state=deferred-until-focus
platform-smoke: keyboard=physical mouse=motion-button-wheel text=utf8 composition=ime
platform-smoke: focus-loss=held-state-cleared
platform-smoke: resize=800x600 fullscreen=toggle
platform-smoke: shutdown=clean gpu-device=not-created
```

The smoke created and pumped a real resizable window, used SDL text-input and the real UTF-8 clipboard, exercised cursor visibility/confinement and relative capture, passed keyboard/mouse/text/IME events through the production translator, cleared held input and real relative capture on focus loss, resized, entered and left fullscreen, translated close, stopped text input, and destroyed the window/subsystem in reverse order. The Wayland compositor accepted confinement calls but did not report an engaged grab while the automated window lacked compositor focus; the requested behavior remains covered through SDL and the focus-independent injected state suite.

A diagnostic forced-X11 retry in the Wayland session exited with signal 11 before producing output. It is not acceptance evidence and no X11 claim is made; the source-defined gate requires one SDL3-capable display session, which the native Wayland run satisfies.

## GPU-free proof

The platform library, tests, and smoke contain no `SDL_GPU` reference. Source search and undefined-symbol inspection of `zh_platform_smoke` returned no SDL GPU symbol. The platform target depends on SDL3 and foundation Unicode support only; no renderer object is constructed. Real GPU behavior remains M14.
