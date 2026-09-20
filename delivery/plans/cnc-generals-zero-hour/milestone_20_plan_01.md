# M20 delivery plan: original engine initialization and data lifecycle

Status: blocked — upstream delivery replanning required

## Contract

Deliver the M20 original-source lifecycle boundary without replacing original lifecycle semantics. Bounded source investigation found that the current contract cannot be executed in its declared dependency order: the original `GameEngine::init` reaches original module/object factories only through concrete GameLogic, GameClient/W3D and media providers assigned to M21–M23. A reduced parallel `GameEngine` would violate SE-003, so this transaction stops at an upstream planning blocker with no production edits.

Authority: `delivery/milestones/cnc-generals-zero-hour/M20-original-lifecycle.md`, `docs/zero-hour-source-engine-migration.md` SE-003/SE-010, and the preservation rules in `docs/zero-hour-linux-port-plan.md`.

## Scope boundaries

- Repository-owned source under `GeneralsMD` may receive narrow Linux lifecycle branches while the untouched Windows path remains available.
- Linux adapters may use the existing `zh_data` VFS and foundation XDG contracts. Retail roots remain read-only; tests default to project-owned fixtures.
- Headless execution has explicit null presentation and unavailable-simulation stages. It never substitutes the Alpha/Bravo loop or reports gameplay success.
- Actual original simulation, renderer/UI/media producers, persistence, LAN integration, and release acceptance remain M21 and later.

## Slice index

1. [Slice 01 — source dependency-boundary investigation](milestone_20_plan_01_slice_01.md): blocked (`UPSTREAM_REQUIRED`)
2. [Slice 02 — VFS/XDG data initialization and factory-stage validation](milestone_20_plan_01_slice_02.md): cancelled pending replanning
3. [Slice 03 — runtime identity, process isolation and milestone evidence](milestone_20_plan_01_slice_03.md): cancelled pending replanning

## Validation strategy

Implementation validation is intentionally not run against a substitute lifecycle. The source-order evidence and dependency ownership mismatch are recorded in `evidence/qa/cnc-generals-zero-hour/M20-original-lifecycle-blocker.md`. After replanning, the replacement M20 transaction must still use compile/link/runtime identity, process isolation, negative initialization controls and all-preset/sanitizer acceptance.

## Decisions

- Compiling original filenames under a wholesale alternate implementation is not original-source integration and cannot satisfy M20.
- Original module/object factory construction is not separable from M21 GameLogic and M22 GameClient/W3D registrations without changing behavior or providing fake success.
- Replanning must either move those factory obligations to their actual provider milestones and narrow M20's executable boundary, or reorder/combine provider work so preserved `GameEngine::init` can run in source order.
- No retail content or private path was accessed, and no production code from the rejected approach remains changed.

## Blocker and resume point

Classification: `UPSTREAM_REQUIRED` (`planning_stale`). Required workflow: delivery architecture reconciliation followed by delivery replanning of M20–M23 dependencies/contracts. Resume by replacing or superseding this governing plan only after the packet establishes an executable original-source initialization boundary. Do not resume by adding a reduced `GameEngine` class, success-returning null factories, or substitute registries.
