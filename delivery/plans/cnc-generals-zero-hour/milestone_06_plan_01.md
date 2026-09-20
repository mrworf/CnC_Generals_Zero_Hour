# M6 delivery plan: SDL platform and input without GPU

## Authority and outcome

This plan delivers [M6](../../milestones/cnc-generals-zero-hour/M6-sdl-platform.md) under the accepted `docs/zero-hour-linux-port-plan.md`. The outcome is a GPU-free SDL3 platform boundary whose event translation is exhaustively injectable in headless tests and whose real window lifecycle passes one interactive display smoke.

Transaction parent: `71021b66ec9f2951d2aaa4a7127aaf1753e979ae`.

Dependencies M1 and M3 are accepted in the milestone packet. SDL3 3.4.14 is available. PRE-016 is closed only by the interactive evidence in slice 02; display environment variables alone are not acceptance.

## Constraints

- Do not initialize or create an SDL GPU device.
- Headless translation tests initialize no SDL subsystem and require no display, retail data, audio device, or network.
- Physical controls use SDL scancodes; keycodes are retained only as layout-dependent shortcut values. Text is accepted solely through UTF-8 text/editing events.
- Focus loss clears held keys/buttons and relative capture. Window, fullscreen, resize, mouse, cursor, confinement, clipboard, text/IME, and clean shutdown behavior remain explicit and testable.
- A missing display fails with the SDL subsystem and underlying error named.

## Slice index

1. [Slice 01 — injected SDL event and input state contract](milestone_06_plan_01_slice_01.md) — completed in `1746732`.
2. [Slice 02 — SDL window lifecycle and interactive PRE-016 smoke](milestone_06_plan_01_slice_02.md) — completed in `fe01847`.

The slices are dependency ordered and all are defined before production edits. Slice 01 provides the translator/state model consumed by the real window in slice 02.

## Milestone validation

For all four supported presets:

```text
cmake --preset <preset>
cmake --build --preset <preset>
ctest --preset <preset> -L platform --output-on-failure
```

Run the interactive smoke in the accessible developer graphical session and record its display driver, exercised behaviors, clean shutdown, and proof that the GPU API was never invoked. Also run the no-display negative path and record its actionable diagnostic.

## Completion boundary

M6 is ready for orchestrator acceptance only when both slice commits exist, the four-preset `platform` suite passes, and `evidence/platform/cnc-generals-zero-hour/M6-sdl-platform.md` records successful PRE-016 interactive acceptance. GPU creation and pixel acceptance remain M14 work.
