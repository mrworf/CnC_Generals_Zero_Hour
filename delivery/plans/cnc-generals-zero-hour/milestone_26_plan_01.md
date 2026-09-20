# M26: Original process allocation and ABI foundation

This plan governs exactly one milestone transaction.

## Outcome

Asset-free Linux executables use the original allocator, string, version, synchronization, and floating-point support before full engine initialization. They preserve x86-64 and UTF-16 behavior, accept a bounded optional pre-VFS pool override, and provide source-to-runtime evidence that fails when an original provider becomes unowned or is removed.

## Delivery-goal context

- Goal status record: `workflow/delivery/state.yaml`
- Product ID: `cnc-generals-zero-hour`
- Active packet and revision: `delivery/milestones/cnc-generals-zero-hour/status.yaml`, immutable Markdown fingerprint `sha256:0d5f1fef5dcec3bbbbb26656c5aaf591d653e2454501b51cfb09059fbb6acd7a`
- Source transaction: `8cf8f6ce3842cffe37505d02e610ca825ecdb532`
- Planning transaction: `1370387ffb3c61c05a9044338a1811c185218289` (closeout `b53700d92276a6c56beab936bc3d60b20184b282cd`)
- Fixed goal scope: `M26,M27,M28,M20,M21,M22,M23,M24,M25,M15,M16,M17,M18` (context only)
- Current milestone: `M26`
- Resume mode: new

## Governing contracts

- Milestone: `delivery/milestones/cnc-generals-zero-hour/M26-original-process-foundation.md`
- Product authority: `docs/zero-hour-linux-port-plan.md`
- Migration authority: `docs/zero-hour-source-engine-migration.md` (`SE-001`, `SE-002`, `SE-010`)
- Reconciliation authority: `docs/zero-hour-runtime-closure-reconciliation.md` (`RC-001`, `RC-002`, `RC-004`, `RC-006`, `RC-007`, `RC-008`, `RC-011`)
- Repository instructions: no repository-local `AGENTS.md` exists; the active milestone, readiness packet, CMake presets, and established CTest conventions govern.

## Current-state findings

M19 compiles ten bounded original WWLib/WWMath/WWSaveLoad and RefPack units, but `zh_game_engine` is still a synthetic component. The original `GameMemory.cpp`, `MemoryInit.cpp`, `AsciiString.cpp`, `UnicodeString.cpp`, `CriticalSection.cpp`, and `version.cpp` are not built. Their PCH and headers assume MSVC/Win32 (`atlbase.h`, `windows.h`, `new.h`, `GlobalAlloc`, `CRITICAL_SECTION`, `__int64`, x86 inline assembly, MS formatting names, and 16-bit Windows `wchar_t`). The WWLib mutex already has a native atomic implementation. The original allocator contains the pre-main fallback and linkage counter, while `MemoryInit.cpp` has compiled pool defaults and an unsafe Windows/CWD-dependent optional override. The sole `setFPMode` body is embedded in the coupled `GameLogic.cpp`.

## Decisions

- Compile the actual original translation units with a narrowly scoped Linux compatibility PCH and conditional portability edits in their authoritative headers/sources. Do not create reduced replacement classes.
- Preserve the allocator algorithm, object layouts, globals, and original method bodies. Replace Win32 raw allocation and synchronization primitives with characterized libc/C++ equivalents and add sized/aligned C++17 overload pairing needed by GCC/Clang.
- Preserve 16-bit `WideChar` on Linux and use bounded UTF-16 helpers rather than relying on the host's 32-bit `wchar_t` ABI.
- Resolve the optional `MemoryPools.ini` path from an explicit pre-main-safe environment variable or the executable directory; never from CWD. Parse into a temporary fixed-size table, validate the whole file, then apply once before live allocations.
- Move `setFPMode` to a small shared original source unit and leave all existing callers using the same declaration. M27 will prove the INI/map callers.
- Keep new original-runtime targets active in all build types without the M19 `NDEBUG` workaround.

## Scope

### Included

- Actual original allocation, pool-default, strings, version, synchronization, and FPU control providers.
- Linux CRT/PCH/ABI compatibility required by those providers.
- Asset-free original-consumer and failure-path tests in all four presets.
- Checked dependency ledger, identity evidence, drift detection, and provider-removal controls.
- Focused sanitizer execution with exact options and explicit live-allocation/service counts.

### Excluded and deferred

- M27 VFS/INI/map caller linkage and general data parsers.
- M28 UI/resource/media consumers.
- M20 integrated engine registry/startup/shutdown.
- Retail data, devices, networking, rendering, gameplay rules, and save formats.

## Slice index

| Slice | Plan | Outcome | Dependencies | Status | Commit | Evidence |
|---|---|---|---|---|---|---|
| 01 | [Allocator bootstrap and ABI](milestone_26_plan_01_slice_01.md) | Actual original allocator and bounded pre-VFS pool configuration execute safely across process/target boundaries. | M19 | complete | this slice commit | 2 focused tests pass in all four presets |
| 02 | [Strings and process services](milestone_26_plan_01_slice_02.md) | Actual original ASCII/UTF-16 strings, synchronization, logging seam, and Version lifetime execute on the allocator. | slice 01 | pending | | focused string/lifecycle tests |
| 03 | [FPU and dependency evidence](milestone_26_plan_01_slice_03.md) | The original FPU body is shared and characterized; checked ledger and target-specific identity gates cover M26. | slices 01-02 | pending | | FPU, ledger, identity, negative controls, four presets, full CTest, sanitizers |

## Cross-slice concerns

- Compatibility and migration: x86-64 only, 16-bit UTF-16 code units, original public interfaces and allocator ownership stay stable.
- Authorization and security: no authorization surface; override paths are bounded and explicit, and ordinary tests are asset-free.
- Invalidation and lifecycle effects: pre-main initialization is single-shot; invalid configuration cannot partially retune pools; static destruction retains allocator availability.
- Audit and observability: tests expose provider identity, configuration result, allocation counts, and source/ledger grades without private paths.
- Performance and scale: pool defaults remain authoritative; test fixtures may select bounded test counts without changing production defaults.
- Environment or external services: none; no retail tree, device, GPU, network, or writable global path.

## Milestone completion gate

- Configure, build, and run M26 tests in `linux-gcc-debug`, `linux-clang-debug`, `linux-gcc-release`, and `linux-clang-release`.
- Run every focused positive/negative M26 test and the full asset-free CTest suite once after stabilization.
- Run relevant ASan/UBSan tests and record exact environment/options plus live allocation/service counts.
- Verify compile commands, live linked symbols, runtime witnesses, provider-removal failure, ledger freshness, and ownerless-edge failure.
- Reconcile every M26 acceptance criterion with committed evidence and inspect the cumulative diff for later-milestone leakage.

## Rollback and recovery

Each slice is independently revertible. Reverting slice 03 removes evidence/FPU support while preserving allocator and strings; reverting slice 02 removes string/version consumers while preserving allocator; reverting slice 01 removes the M26 target entirely. Generated build directories remain disposable.

## Execution notes

Planning completed before production edits at transaction start `f84161fb666ec3a5092b46002094aca4f91c41ca`; the worktree was clean.

## Deferred follow-ups

- M27 must link real INI and MapCache callers to the shared `setFPMode` definition.
- M27/M28/M20 owners are retained in the dependency ledger until their runtime evidence upgrades the corresponding edges.
