# M3 slice 01: headless entry and null devices

## Goal and observable outcome

`zh_main --headless --ticks N --state-dir /absolute/path` initializes only null devices, advances exactly `N` deterministic ticks, emits a capability report and lifecycle log, writes a completion record below the selected state root, and shuts down cleanly.

## Scope

- Strict headless argument parsing and stable exit-code categories.
- Concrete null platform, renderer, audio, and video implementations on their existing targets.
- Staged paths/logging/device/engine construction with reverse teardown.
- Deterministic fixed-tick runner, capability report, run log, and completion record.
- Preserve the M0 `--bootstrap-smoke` entry.

## Non-scope

Failure injection coverage beyond the basic seam, retail data, simulation/gameplay integration, SDL initialization, actual devices, networking, launcher/DRM compatibility, or legacy source cleanup.

## Dependencies and ordering

M1 and M0. This slice establishes the lifecycle consumed by slices 02 and 03.

## Entry point and end-to-end behavior

`main(int, char **)` dispatches the explicit bootstrap or headless command. Headless parses all options before mutation, resolves an absolute explicit state root or the M1 XDG state path, creates only its `logs` directory, initializes null devices, advances a counter, writes `last-headless-run.txt`, and tears down.

## Data and state transitions

Configuration is immutable after parsing. Each lifecycle stage transitions from absent to initialized once and back to stopped once. Tick count advances monotonically from zero to the requested count. Files are constrained to the state root.

## Authorization and permission behavior

No authorization model applies. Filesystem permission failures are reported as path/log/runtime errors; no fallback writes outside the selected root occur.

## Validation and error handling

- Positive: explicit state root with one and several ticks; default XDG state; bootstrap compatibility.
- Negative: unknown/duplicate/missing options, relative state root, malformed or excessive tick count, unwritable state location.
- Boundary: zero ticks and the one-million-tick maximum.

## Implementation surfaces

`include/zh/headless/*`, `src/headless/*`, `src/bootstrap/main.cpp`, the four null targets and headless runtime wiring in `CMakeLists.txt`, focused C++ tests, and build documentation.

## Required validation

- Build and run focused argument/runtime tests and process smoke tests.
- `ctest --preset linux-gcc-debug -L 'foundation|headless' --output-on-failure`.
- `git diff --check`.

## Acceptance criteria

Output reports exact ticks and intentional skips; log and completion files are below the chosen root; no SDL/device/retail/network setup occurs; invalid input fails before lifecycle mutation.

## Commit boundary

Commit this plan set, parsed entry, null devices, successful lifecycle, tests, and documentation as `delivery: M3 slice 01 add headless runtime`.

## Completion evidence

- GCC Debug builds the updated graph and passes all eleven `foundation|headless` tests.
- Focused tests cover zero/default/maximum ticks, malformed/duplicate/unknown options, explicit and XDG state roots, unwritable path diagnostics, lifecycle logs, completion records, capability reporting, and M0 bootstrap compatibility.
- The process smoke reaches two ticks through the installed executable path with no retail or interactive-device input.
