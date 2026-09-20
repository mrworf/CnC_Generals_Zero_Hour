# Milestone M1: portable ABI and support libraries

## Objective

Deliver portable ABI and support libraries as the source-defined M1 outcome. This contract compiles the accepted native Linux migration; it does not mark implementation complete.

## User/System Outcome

PRE-005 is complete. `foundation` tests pass with GCC and Clang in x86-64 Debug and Release builds and under ASan/UBSan where available. Serialized primitive fixtures are byte-identical across compilers and build types.

## Scope

- introduce fixed-width engine types, `char16_t` `WideChar`, UTF-8/UTF-16 conversion, endian codecs, bounded readers/writers, and portable formatting helpers;
- port support libraries needed below the game loop: files, paths, XDG locations, clocks, threads, atomics, sockets' shared types, RefPack, and the zlib adapter;
- replace compiler extensions, inline x87 conversion helpers, `long`/pointer-size assumptions, and unaligned host reads in this closure;
- characterize numeric conversions and establish deterministic compiler/floating-environment flags; and
- add synthetic BIG, Unicode, compression, serialization, numeric, path, and timer fixtures.

## Explicit Exclusions

Subsystem save/replay/packet integration is M5/M11.

Only Zero Hour in GeneralsMD is ported. Base Generals executable, authoring tools, DRM, obsolete online services, unrelated engine rewrites, non-x86-64 support and retail redistribution remain excluded.

## Source Requirements

- [Authoritative port plan](../../../docs/zero-hour-linux-port-plan.md), §8 “M1 — portable ABI and support libraries”.
- Governing sections: §5 Fixed-width types / Unicode / Serialization and packets / Floating-point behavior; §6 Time, threads, diagnostics, and compression; PRE-005.
- Shared §1 scope, §2 preservation rules, §3 selected stack, §7 dependency/check contract and §10 validation matrix apply. Source revision and commit are recorded in status.yaml; this contract cannot supersede that authority.

## Preconditions

M0.

Direct implementation dependencies: [M0](M0-build-graph.md).

If a precondition fails, retain completed evidence and stop only this milestone and dependants; do not claim acceptance from a partial demonstration.

- `PRE-003` — audited source/dependency manifests; provider M0; require completed acceptance evidence before entry (currently pending implementation).
- `PRE-004` — offline CMake presets, probes, smoke tests and shared CI instructions; provider M0; require completed acceptance evidence before entry (currently pending implementation).

## Readiness checks

- Inspect status.yaml and linked acceptance evidence for M0 — each direct provider must have completed its contract; pending implementation is not completed evidence.
- After M0 exists, run `cmake --list-presets` — all four source-defined presets are listed. Inspect provider test/evidence records and use their documented checks if freshness is in doubt; do not require this milestone's unfinished features at entry.

Produces `PRE-005` (portable ABI, codecs and support libraries) as part of this milestone's acceptance, not as entry requirements.

Readiness audit: [prerequisite manifest](../../readiness/cnc-generals-zero-hour/prerequisite-manifest.md). Unmet providers stop this milestone and dependants only; all implementation statuses remain pending.

## Functional Requirements

Implement the scoped work above using the concrete interface contract below.

Complete every scoped work item and its source exit together; preserve existing simulation, formats and engine-facing behavior except the explicitly replaced platform edges.

## Architecture / Security Constraints

Do not use -fshort-wchar or host-sized raw serialization. Preserve engine libraries, compression tags/levels and allocator architecture. Reject NOX/LZH with typed archive/logical-path error. FE_TONEAREST assertions and checked narrowing replace x87 assumptions.

Treat retail files as read-only, including the ignored original-game symlink. Commit only project-owned source, synthetic fixtures and permitted logical/derived evidence. Ordinary tests must run without private media, interactive devices or a network fetch.

## Interfaces and Compatibility

Apply the concrete [shared writable-path contract](index.md#shared-writable-path-contract), including every XDG fallback and read-only retail boundary.

Use fixed-width integer aliases, char16_t WideChar, UTF-16LE file codecs and UTF-8 boundary conversion. Bool/enums/coordinates are encoded as named fixed-width fields; retain versioned source-established Win32 layouts.

Use the canonical x86-64 presets `linux-gcc-debug`, `linux-clang-debug`, `linux-gcc-release` and `linux-clang-release`. Shared primitive codecs and public engine interfaces remain compatibility boundaries. Changes outside this contract require source-authorized evidence, not incidental cleanup.

## Acceptance Criteria

- [ ] PRE-005 is complete. `foundation` tests pass with GCC and Clang in x86-64 Debug and Release builds and under ASan/UBSan where available. Serialized primitive fixtures are byte-identical across compilers and build types.
- [ ] Every scoped behavior and interface above has positive evidence; the listed negative/boundary cases fail with actionable diagnostics.
- [ ] The required validation below passes; deferred work is not used to mask an unfinished path.
- [ ] Evidence records compiler/build or device/corpus context as applicable without private media or absolute private paths.

## Required Validation

Run `cmake --build --preset <preset>` and `ctest --preset <preset> -L 'foundation' --output-on-failure` for applicable supported presets.

Test byte equality across four presets, malformed UTF-16/surrogates, overflow/alignment/endian boundaries, ASCII-loss restrictions, XDG fallback/unwritable paths, compression truncation/bounds, wrap-safe clocks, thread join and numeric signed-zero/halves/NaN/infinity behavior.

Retail checks use a separate build directory with `ZH_ENABLE_RETAIL_TESTS=ON` and local `ZH_RETAIL_ZH_DATA`, `ZH_RETAIL_GENERALS_DATA`, `ZH_RETAIL_LANGUAGE`. Hardware jobs additionally enable `ZH_ENABLE_GPU_TESTS=ON`; defaults stay OFF. Do not claim future commands have run when compiling this packet.

## Known Risks / Deferred Work

Game save/replay integration is M5; packet integration M11; no claim of Windows fixture compatibility.

Source §9 conditional gates remain evidence-driven; newly demonstrated blockers are recorded at the consuming milestone.
