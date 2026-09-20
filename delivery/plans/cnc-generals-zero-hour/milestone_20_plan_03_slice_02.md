# M20 plan 03 slice 02: bounded BIG archive provider

## Goal and observable outcome

The retained Linux BIGF/BIG4 adapter safely opens valid archives through original ArchiveFile/RAMFile/StreamingArchiveFile consumers and rejects malformed, truncated, out-of-range, and overflowing inputs without leaks, stale directory entries, partial publication, or host-dependent behavior.

## Scope and explicit non-scope

Characterize and harden the production adapter retained at the plan-02 blocker. Preserve canonical case-folded lookup, archive ordering/override semantics, loose-file precedence, streaming bounds, and exactly-once cleanup. Add owned binary fixtures built by tests without importing retail content. Do not change retail roots, invent new archive formats, expose private metadata, or claim W3D/lifecycle acceptance from archive tests.

## Dependencies and ordering constraints

Depends on slice 01 only because final retail traversal consumes both boundaries; implementation itself must remain independently testable and revertible.

## Entry point and end-to-end behavior

The original `ArchiveFileSystem::loadArchivesFromDirectory` discovers a valid BIGF/BIG4 file, validates its header and directory using checked offsets/sizes, publishes entries only after full validation, and returns original file abstractions whose seek/read operations remain within the entry. Invalid inputs leave no readable entries and preserve the initiating diagnostic. Destruction closes files and removes owned directory state exactly once.

## State, authorization, and permissions

Archive files are read-only. Test fixtures live in temporary project-test roots. No user authorization is applicable and no retail bytes, paths, hashes, or writes are permitted.

## Validation and error recovery

Positive cases cover BIGF/BIG4, mixed-case canonical lookup, directory ordering, duplicate/override behavior, RAM and streaming reads, seek boundaries, and repeated cleanup. Negative/boundary cases cover short headers, invalid identifiers, truncated tables/names/data, missing name terminators, oversized counts, arithmetic overflow, offsets before/after the file, entry ranges crossing EOF, failed opens, and partial multi-archive load. ASan/UBSan and explicit live-owner checks cover error cleanup.

## Expected implementation surfaces

`src/original_runtime/linux_game_engine.cpp`, focused archive tests/fixture builders, CMake test registration, and archive dependency/classification evidence.

## Acceptance criteria and commands

- All owned valid lookup/read/override cases pass through the production adapter.
- Every malformed/range/overflow case fails deterministically without publication or leaks.
- Focused configure/build/CTest passes under all four native presets and focused sanitizers.
- Commit boundary: `delivery: M20 plan03 slice 02 harden BIG archives`.
