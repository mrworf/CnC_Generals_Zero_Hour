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

