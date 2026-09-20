# M20 plan 02 slice 05: entry, side effects, and failure semantics

## Goal and observable outcome

The production Linux executable enters through original `GameMain` and preserves source success/failure semantics for initialization, early exits, LOD/capability/benchmark setup, map-cache side effects, execute/reset, and process teardown.

## Scope and non-scope

Make original entry authoritative; remove protection/fingerprint/deletion/online-service behavior while retaining real resource checks; bound GameLOD/benchmark adaptation without device acquisition; route generated state to XDG; preserve verify/cache-build controlled-success exits. Cover non-D3D ErrorCode, malformed data, missing music/resources, constructor/init/INI/post-load/update/reset failures, and teardown diagnostics. This slice does not claim later domain behavior.

## Dependencies and ordering

Requires slices 01-04. Supplies production entry and complete failure behavior for slice 06 assurance.

## Entry point, state, and recovery

Process services initialize before original `GameMain` and shut down after engine/workers. Valid bounded profile exits zero. Each injected error exits nonzero without continuing to execute/reset, preserves its first diagnostic, unwinds pending and registered owners exactly once, stops workers, and leaves retail/CWD unchanged. Verify/cache-build success follows source semantics and never initializes unnecessary hardware.

## Permissions and tests

Owned fixtures and isolated XDG roots are default. Read-only retail is deferred to slice 06's explicit gate. Test arbitrary CWD, read-only input, denied cache/config output, absent/stale/cold/warm cache, bounded benchmark in Debug/Release, no-device interception, and primary-error preservation.

## Expected surfaces

Linux main/process bootstrap, original GameMain/factory/error bridge, LOD/cache/benchmark portability, diagnostics, CLI tests, ledger/classification and identity controls.

## Required validation and acceptance

Focused process tests pass in four presets; success and every failure exit code/diagnostic are correct; no failure continues into execute/reset; all output is XDG; no physical device or network is acquired; exact ownership counts return to baseline. Commit as `delivery: M20 slice 05 finalize original Linux entry`.

The production fixture must observe `GameLogic` and `GameClient` source state before/after each update and reset, rather than accepting trace-only calls. It runs cold and warm from an arbitrary CWD with read-only input, exercises preset-independent `-benchmark 1`, and proves `-buildmapcache` exits during original initialization without update/reset. A denied XDG output root and missing, malformed, and unknown INI inputs fail nonzero with their primary diagnostic and no lifecycle-success marker. Link-map identity and removal of the extracted `GameLogic.cpp.o` provider apply to the production executable, not only the slice-04 harness. The FunctionLexicon gate inventories all 242 names, requires all 96 online-only callbacks to resolve to unique exact-name failure functions, proves representative system/init/update/shutdown runtime rejection, and requires all 147 offline names to be live outside that boundary. The authoritative absent-SDK evidence is the checked legacy source inventory; no retail path or hash is evidence.

Optimized execution is part of the slice boundary, not deferred assurance. GCC Release exposed legacy null-`this` deletion idioms after an owner had been released or was initially absent (`MemoryPoolObjectHolder`, `SidesInfo`, and the optional `ScriptConditions` cache). The accepted corrections guard the pointer before calling `deleteInstance`; populated ownership and destruction remain unchanged. Clang then reached five additional opaque enum families and one C++17-obsolete `register` header through the complete provider closure. Their declaration/definition pairs are recorded in the enum ABI manifest, checked as `sizeof(int)`, and compared against unchanged baseline enumerator bodies. These are mechanical source-portability prerequisites, not later-domain behavior.
