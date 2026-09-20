# M26 plan 01 slice 02: Strings and process services

## Goal and outcome

Independent asset-free consumers execute actual original `AsciiString`, `UnicodeString`, and `Version` behavior on the original allocator with Linux synchronization and process-lifetime bootstrap/teardown support.

## Scope

Included: actual original string/CriticalSection/Version methods, 16-bit UTF-16 helpers, copy-on-write and boundary operations, minimal process logging diagnostics, service ownership, construction failure unwind, and tests. Excluded: localized GameText formatting that belongs to later resources, game registries, workers beyond the bounded process-service harness, and retail assets.

## Dependencies and ordering

Depends on slice 01. Uses its original allocator and bootstrap result. Slice 03 consumes these providers for identity/ledger coverage.

## Entry point and behavior

A consumer constructs, copies, mutates, formats/translates, and destroys original strings across target boundaries, then creates and destroys the original Version singleton through an owning process-services guard. Synchronization protects shared string storage. A forced service-construction failure unwinds already-created services exactly once and keeps diagnostics available.

## Data and state transitions

String buffers transition through empty, unique, shared, copy-on-write, and released states. Process services transition in strict allocator/synchronization/log/version order and reverse on normal/failure cleanup; globals are cleared before release and live counters return to baseline.

## Authorization and permissions

Not applicable. Tests use in-memory and owned asset-free values only.

## Validation and error handling

Positive: ASCII and UTF-16 boundary strings, non-BMP surrogate pair preservation, embedded copy-on-write, token/trim/format behavior, cross-target allocation/free, Version numeric/ASCII fields, and static destruction. Negative: maximum-length rejection, construction failure at every service stage, double-shutdown idempotence, and provider-removal link failure.

Focused commands: build and run string/process-service targets with the `original-process` label in all four presets; run the focused sanitizer target after ordinary tests stabilize.

## Implementation surfaces

- Original string, CriticalSection, and Version translation units/headers.
- Linux UTF-16 and process-service adapters under `src/original_runtime/` and public test-facing declarations under `include/zh/`.
- Tests under `tests/original_runtime/` and CMake registration.

## Acceptance criteria

- Original string and Version symbols are live in the test executable in all four presets.
- `sizeof(WideChar) == 2`, boundary operations preserve UTF-16 code units, and copy-on-write/refcount behavior is exercised.
- Process services release exactly once on success and each injected failure, with allocator/resource/worker counts reported.
- No private asset, device, network, VFS, or full-engine initialization is used.

## Commit boundary

Commit actual string/process provider portability, tests, evidence, and the updated plan index as one coherent slice.
