# M3 slice 03: isolated concurrent processes

## Goal and observable outcome

Two simultaneously launched headless processes complete independently when given different state roots; neither process writes into the other's root or depends on display, audio, GPU, retail data, networking, or its working directory.

## Scope

- Process-level concurrency/isolation test with distinct absolute state roots and unrelated working directories.
- Poisoned display/audio environment values to verify no SDL device initialization.
- Final developer documentation, four-preset build/test matrix, and governing-plan completion evidence.

## Non-scope

Same-root coordination, named mutex replacement, multiplayer/network concurrency, load/performance claims, or interactive sessions.

## Dependencies and ordering

Slices 01 and 02.

## Entry point and end-to-end behavior

The test starts two `zh_main --headless` processes before waiting for either, assigns separate roots and tick counts, and verifies each process' output, log, completion record, and root identity.

## Data and state transitions

Each process owns only its configured state tree. Completion records atomically replace only their corresponding root's record and contain that process' tick result.

## Authorization and permission behavior

No elevated permission is used. Temporary directories are test-owned and state is private to each process path.

## Validation and error handling

- Positive: both concurrent processes exit zero and produce their distinct expected records.
- Negative: process A output/files never mention process B's root and vice versa; no file appears outside the two roots.
- Boundary: launch from working directories unrelated to source/build and run with invalid SDL driver names.

## Implementation surfaces

Process integration test, CMake/CTest registration, `docs/building-linux.md`, and completion evidence in this plan set.

## Required validation

- Build all four canonical presets.
- For each preset run `ctest --preset <preset> -L 'foundation|headless' --output-on-failure`.
- Run `git diff --check` and inspect the final tracked-file diff for retail/private paths.

## Acceptance criteria

Both processes overlap, succeed, and remain isolated; invalid SDL environment settings have no effect; the complete M0-M3 asset-free suite passes under GCC and Clang Debug/Release.

## Commit boundary

Commit concurrency validation, documentation, and final evidence as `delivery: M3 slice 03 validate process isolation`.
