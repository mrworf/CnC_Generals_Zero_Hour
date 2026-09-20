# M1 slice 02: POSIX support primitives

## Goal and observable outcome

Foundation consumers can resolve XDG-owned paths independently of the working directory, normalize safe logical retail paths, perform bounded local file I/O, use wrap-safe clocks, and rely on joined threads plus fixed socket address types.

## Scope

- XDG config/data/state/cache resolution with specification fallbacks and injectable environment.
- Logical path separator normalization, ASCII case folding, and rejection of absolute/parent traversal.
- Bounded binary file read and atomic project-owned write helper.
- Steady/wrapping millisecond clocks and wrap-safe deadline comparison.
- RAII joining thread, atomic aliases, and fixed-width IPv4/port shared types.

## Non-scope

Retail VFS mounting, archive enumeration, live sockets, logging sink integration, engine worker migration, and changes to user data.

## Dependencies and ordering

Depends on slice 01 types and errors.

## Entry point and end-to-end behavior

`foundation_platform_tests` supplies synthetic environment maps and temporary project-owned directories, checks all four XDG paths and fallbacks, normalizes logical names, writes/reads a bounded file, joins a worker, and exercises clock wrap semantics.

## Data and state transitions

Path resolution is pure. Atomic write creates a sibling temporary file then renames it to the destination; failures remove the temporary artifact. Joining-thread destruction transitions joinable workers to joined. Retail roots are never inputs to writable helpers.

## Authorization and permission behavior

Tests write only to a unique temporary directory. Writable APIs require a caller-selected project path; they do not discover or mutate retail roots.

## Validation and error handling

- Positive: explicit XDG variables, HOME fallbacks, mixed separators, case fold, exact bounded read/write, joined worker, monotonic/wrapping clocks.
- Negative: unset/relative XDG variables, missing HOME, absolute/traversal/empty logical path, read size overflow, unwritable/missing parent, invalid ports.
- Boundary: wrap at `UINT32_MAX`, exact maximum file size, hidden dotted logical names, and repeated separators.

## Implementation surfaces

`include/zh/foundation/platform.h`, `src/foundation/platform.cpp`, target wiring, and `tests/foundation/test_platform.cpp`.

## Required validation

- Build/run focused `foundation_platform_tests`.
- Run `ctest --preset linux-gcc-debug -L foundation --output-on-failure`.
- Run `git diff --check`.

## Acceptance criteria

All XDG fallbacks are exact and working-directory independent; unsafe logical paths are rejected; file operations are bounded; workers join; clock comparisons remain correct across wrap; APIs use fixed-width socket data.

## Commit boundary

Commit this plan, POSIX support implementation, wiring, and tests as `delivery: M1 slice 02 add POSIX support`.

## Completion evidence

- `foundation_platform_tests` passes with explicit and HOME-fallback XDG locations, including the all-explicit/no-HOME boundary.
- Mixed path separators/case normalize deterministically; empty, absolute, drive-qualified, and parent-traversal inputs are rejected.
- Exact-limit file round trip, oversized/missing/unwritable-path failures, 32-bit clock wrap, RAII thread join, atomics, and checked IPv4 port construction pass.
- GCC Debug `ctest -L foundation` passes all eight M0/M1 tests; broader matrix evidence is recorded by slice 03.
