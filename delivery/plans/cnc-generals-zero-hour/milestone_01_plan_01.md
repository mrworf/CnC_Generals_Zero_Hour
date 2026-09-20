# M1: portable ABI and support libraries

## Outcome

The bootstrap graph gains a portable, asset-free foundation layer with fixed-width ABI types, explicit Unicode and binary codecs, deterministic numeric helpers, POSIX/XDG support, and bounded compression adapters. The same serialization fixture is byte-identical in all four canonical presets.

## Delivery-goal context

- Goal status: `workflow/delivery/state.yaml`
- Product ID: `cnc-generals-zero-hour`
- Governing contract: `delivery/milestones/cnc-generals-zero-hour/M1-portable-foundations.md`
- Product authority: `docs/zero-hour-linux-port-plan.md`, sections 5, 6, and 8/M1
- Transaction start: `f171c987f48d9e1ad9c5fbf8d99356e9cfd66e7c`
- This companion transaction implements M1 only; the outer orchestrator owns milestone acceptance and status.

## Current-state findings

- M0 exposes explicit `zh_wwsupport`, `zh_os_posix`, and `zh_compression` targets, but each still compiles only the bootstrap component.
- Deterministic compiler flags and four GCC/Clang Debug/Release presets already exist.
- Legacy RefPack and compression sources exist, but their readers trust input sizes, perform unaligned host reads, and use host-sized integers.
- No portable engine ABI, UTF codec, bounded byte reader/writer, XDG/path abstraction, or foundation test executable exists yet.

## Decisions

- Add project-owned C++17 foundation APIs under `include/zh/foundation` and `src/foundation`; retain bootstrap symbols while targets acquire real sources.
- Represent fallible parsing/conversion with status-bearing result objects or exceptions caught at the test/application boundary; never expose partially decoded state.
- Define binary layouts only through bounded readers/writers and explicit endian methods. The stable fixture is checked against a repository constant and emitted for cross-preset comparison.
- Adapt RefPack decoding into a bounded implementation and zlib into an explicit `ZL1`-`ZL9` envelope. Recognize `NOX` as a typed unsupported error.

## Slice index

| Slice | Plan | Outcome | Dependency | Status | Commit |
|---|---|---|---|---|---|
| 01 | [portable ABI and codecs](milestone_01_plan_01_slice_01.md) | Fixed widths, UTF, endian/bounded serialization, formatting, and numeric semantics | M0 | completed | `757aec8` |
| 02 | [POSIX support](milestone_01_plan_01_slice_02.md) | XDG/path, files, clock, threading, atomic, and socket shared types | 01 | completed | `94349be` |
| 03 | [compression and matrix](milestone_01_plan_01_slice_03.md) | Bounded RefPack/zlib adapters and complete four-preset/sanitizer validation | 01, 02 | completed | `bb9a388` |

## Cross-slice constraints

- x86-64 only; no `-fshort-wchar`, raw host-object serialization, unaligned typed reads, retail content, network fetch, or device requirement.
- Each behavior has positive, malformed-input, and relevant boundary tests under the `foundation` label.
- All source lists remain explicit. M0 smoke and inventory behavior must continue to pass.
- Retail roots are read-only by contract and are not accessed by this milestone.

## Milestone completion gate

- Configure, build, and run `ctest -L foundation` for all four canonical presets.
- Compare the emitted primitive fixture byte-for-byte across presets and against the checked hexadecimal contract.
- Run foundation tests in an ASan/UBSan build where the host toolchain supports them.
- Confirm malformed UTF-16/UTF-8, buffer exhaustion, numeric exceptional values, invalid logical paths/XDG inputs, clock wrap, thread join, truncated/oversized compression, and `NOX` rejection produce stable errors.
- Run `git diff --check` and confirm no private or absolute local path entered tracked artifacts.

## Rollback and recovery

Each slice is independently revertible. The POSIX and compression slices consume only the portable contracts established by slice 01. Reverting a consumer does not alter the serialized format contract.

## Execution notes

All slice plans were created before production edits. They were inspected together for dependency order, independently testable behavior, explicit negative cases, and commit boundaries.

All four canonical presets configure and build the full M0/M1 graph. Each preset passes nine `foundation` tests and independently checks the same fixed serialization hex, so byte identity is enforced against a shared format contract rather than inferred from host memory. Combined ASan/UBSan builds pass under GCC 16.2.1 and Clang 22.1.8. The restricted execution environment cannot initialize LeakSanitizer under ptrace, so sanitizer test execution uses `ASAN_OPTIONS=detect_leaks=0`; address and undefined-behavior instrumentation remain enabled.
