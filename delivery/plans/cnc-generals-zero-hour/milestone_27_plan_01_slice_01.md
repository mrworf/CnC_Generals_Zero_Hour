# M27 plan 01 slice 01: logical files, INI and CSF

## Goal and observable outcome

An owned fixture mounted through the native loose/BIG VFS can be enumerated and read by a source-owned original-data facade. INI fields and CSF labels yield deterministic source values while malformed, missing, oversized, traversal, and encoding-invalid inputs fail with useful errors.

## Scope

- Add the M27 original-data provider target and a production-facing interface.
- Bridge original file/config consumers to `zh::data::VirtualFileSystem` without duplicating mount authority.
- Extract the reusable INI tokenizer/typed field core from original behavior while retaining `INI.cpp`'s complete production dispatch table for M20.
- Preserve `INI::loadDirectory` ordering: sorted root files first, then sorted descendants; apply layer precedence through the existing VFS.
- Parse the source CSF format as bounded little-endian records and preserve UTF-16 code units.
- Invoke M26's sole `setFPMode` from the shared INI load path.

## Non-scope

Full `INI.cpp` static table linkage, UI/audio/AI/object callbacks, `INIMapCache`, GameText singleton integration, retail data, and full engine startup.

## Dependencies and ordering

Requires accepted M4 VFS behavior and M26 allocator/string/FPU provider. This is the first M27 slice and establishes the data provider consumed by slices 02-03.

## Entry point and behavior

The test creates owned loose and BIG fixtures, mounts them through `VirtualFileSystem`, constructs the original-data reader, enumerates logical INI files, and parses selected real typed callbacks (Bool, Int, Real, ASCII, UTF-16, and name keys) into fixture-owned state. The same reader loads CSF labels. Read operations expose their logical source and never a host-private path.

## Data/state transitions

Input bytes -> bounded tokenizer/decoder -> temporary parsed document -> committed callback-owned values only after complete field validation. Missing/malformed loads do not partially mutate destination state. Name keys are deterministic within a reader lifetime.

## Authorization and permissions

No authorization surface. Inputs are caller-owned read roots. The slice opens no output file and never requests writable access to them.

## Validation and error handling

Positive tests cover loose/BIG override precedence, case/separator normalization, duplicate key override, root-before-subdirectory two-pass ordering, all selected field types, CSF strings including a surrogate pair, and arbitrary CWD. Negative tests cover missing files, traversal, malformed/unknown INI fields, invalid Bool/Int/WideChar, truncated/oversized CSF records, and unchanged read roots.

## Expected implementation surfaces

`CMakeLists.txt`; `include/zh/original_data.h`; original-tree extracted file/INI/CSF provider sources; M26 compatibility support as narrowly required; `tests/original_data/`; source classification and dependency ledger rows owned by this slice.

## Required commands

- Configure/build the focused target in all four presets.
- `ctest --preset <preset> -L original-data --output-on-failure` for each preset after the slice tests are registered.
- Run source-classification and dependency-ledger checks.

## Acceptance criteria

- Runtime witnesses come from the source-owned provider linked to the real M4 VFS and sole M26 `setFPMode`.
- Ordering, precedence, typed values, and UTF-16 output match owned golden fixtures in all presets.
- Every negative input fails before destination mutation; read-root snapshots remain unchanged.
- No production INI registry callback is removed, stubbed, or faked.

## Commit boundary

One commit containing this plan, provider/interface, CMake wiring, tests, ledger/classification updates, and focused validation result. Slice 02 work is excluded.

## Result

Complete. `LogicalFiles` consumes the accepted native VFS, preserves sorted root-files-first then descendants, and bounds every logical read. The extracted INI core invokes the sole M26 `setFPMode`, parses selected real field semantics into temporary state, and fails before publishing malformed blocks. The extracted CSF consumer preserves the original inverted UTF-16LE representation, including a surrogate pair and optional speech name. Missing/traversing files, invalid Bool, truncated/invalid/oversized CSF, arbitrary CWD, and unchanged read roots are covered.

Focused runtime plus compile/link/live-symbol identity tests passed 2/2 in `linux-gcc-debug`, `linux-clang-debug`, `linux-gcc-release`, and `linux-clang-release`. The checked dependency ledger and source classification pass. Commit: `af04a8f8beffc620274f3f8524d7057fa3352363`.
