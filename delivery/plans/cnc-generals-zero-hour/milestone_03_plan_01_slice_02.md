# M3 slice 02: failure-safe lifecycle

## Goal and observable outcome

Developers can inject a failure at every named initialization stage and observe an actionable stage-specific error, stable exit code, and exact reverse teardown of all previously initialized stages.

## Scope

- `--fail-init` validation and deterministic injection for paths, logging, platform, renderer, audio, video, and engine.
- Lifecycle event recording in console/log output.
- Exhaustive partial-initialization teardown tests and repeatable successful reruns after failure.

## Non-scope

Production fault recovery beyond orderly process exit, arbitrary exception injection, crashes/signals, or legacy engine subsystem construction.

## Dependencies and ordering

Slice 01. It extends the same staged runner without changing its successful behavior.

## Entry point and end-to-end behavior

The headless command accepts one test-only stage name. When reached, startup stops, reports `initialization failed at <stage>`, tears down each earlier stage in reverse order, and returns the initialization-failure exit code.

## Data and state transitions

Only completed stages enter the active stack. Failure prevents the named stage from becoming active. Unwinding pops active stages last-in-first-out and leaves no worker or device state.

## Authorization and permission behavior

Not applicable; injection is a local developer option with no elevated operation.

## Validation and error handling

- Positive: each valid stage injects exactly once and a subsequent normal run succeeds.
- Negative: unknown/duplicate/missing stage values are usage errors before initialization.
- Boundary: failure before any active stage and failure after all null devices but before ticking.

## Implementation surfaces

Headless argument/runtime sources, focused lifecycle tests, and process-level CTest registration.

## Required validation

- Run all headless tests including every stage table case.
- `ctest --preset linux-gcc-debug -L 'foundation|headless' --output-on-failure`.
- `git diff --check`.

## Acceptance criteria

All seven stages report by name; teardown is exactly the reverse of successful earlier initialization; engine failure starts no ticks; failures leave no workers and do not prevent a clean retry.

## Commit boundary

Commit injection, teardown evidence, and tests as `delivery: M3 slice 02 prove staged teardown`.

## Completion evidence

- The focused runtime test injects all seven stages, checks the named initialization exit, verifies no tick completion, checks reverse device shutdown, and performs a successful retry in the same state root.
- Invalid, missing, and duplicate injection arguments fail as usage errors before initialization.
- GCC Debug passes the complete `foundation|headless` selection after the lifecycle changes.
