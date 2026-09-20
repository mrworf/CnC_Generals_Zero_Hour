# M19 slice 03: original WW support runtime boundary and milestone evidence

Status: completed

## Goal and observable outcome

Required original WWMath/WWLib/WWSaveLoad support implementations compile and execute in a project-owned harness, including numeric rounding, fixed-width chunk handling, string/UTF-16 boundaries, allocator teardown, and pointer-remap behavior.

## Scope

Promote the smallest dependency-closed original source set that supplies the M19 support boundary. Apply narrow x86-64 compiler/CRT/typedef fixes at original call sites or explicit platform adapters. Replace the `zh_wwsupport` bootstrap provider, connect the identity gate, add positive/negative/boundary tests, and produce milestone evidence with an explicit incomplete production-closure report for M20+ providers.

## Non-scope

No GameEngine factory, asset, UI, renderer, snapshot-format compatibility, or gameplay claim. WWSaveLoad support here is its reusable pointer/chunk foundation; complete game snapshot traversal belongs to M24.

## Dependencies and ordering

Depends on slices 01 and 02. This is the milestone-level validation slice.

## End-to-end behavior and state

The harness initializes original support code, exercises source-owned numeric/chunk/string/remap/allocation transitions, validates malformed/endian boundaries, and tears down without leaked live allocations. Failures are explicit and do not fall back to bootstrap/native toy implementations.

## Authorization and permissions

No privileged operation, network, retail data, display, or GPU access applies.

## Validation and recovery

Focused positive and negative support tests, all four preset builds and asset-free CTest suites, then ASan/UBSan focused allocator teardown. The evidence records exact commands, source/object/link/runtime witnesses, evidence grade, deferred providers, and limitations.

## Expected implementation surfaces

`GeneralsMD/Code/Libraries/Source/WWVegas/`, portability adapter headers/sources, `CMakeLists.txt`, `tests/original_support/`, classification policy/report, and `evidence/qa/cnc-generals-zero-hour/M19-original-support.md`.

## Commands

- Configure/build `linux-gcc-debug`, `linux-clang-debug`, `linux-gcc-release`, and `linux-clang-release`.
- Run each preset's complete asset-free CTest suite.
- Configure/build/run the sanitizer preset's original-support tests.
- Run source classification, compile-command, final-link, runtime identity, and negative-control checks.

## Acceptance criteria

- Required original support implementations compile in all four presets and execute from the final harness.
- Numeric, UTF-16, malformed/endian chunk, pointer-remap, and allocator teardown cases pass.
- Bootstrap/toy fallback cannot satisfy acceptance.
- Evidence distinguishes component/fixture from original-source compile/link and runtime grades, and states M20+ production closure remains incomplete.

## Commit boundary

One commit containing the WW support closure, tests, final classification/evidence, and completion records for this slice and governing plan.

## Completion record

`zh_wwsupport` now compiles seven actual repository-owned WWLib/WWMath/WWSaveLoad providers and no longer compiles the generic bootstrap component. Its asset-free probe exercises GCD/LCM, string duplication, allocator allocate/free teardown, triangle containment, fixed 32-bit little-endian chunk round-trip and truncated-input rejection, pointer remap success/failure, nearest-even rounding, and fixed-width `char16_t` UTF-16. Compiler-command, link-map, and runtime-witness identity gates prove the required objects execute. All four native preset suites passed 62/62, and the Clang ASan/UBSan original-support suite passed 7/7 with leak detection disabled for the established restricted environment. A pre-existing two-process LAN output race found during regression was repaired by preserving the accepted-peer witness when command and disconnect arrive in one pump. Commit: `b868418`.
