# M19 delivery plan: original support closure and source identity

Status: completed

## Contract

Deliver the original-source support boundary required by M19 without claiming GameEngine, rendering, UI, persistence, or gameplay integration. The transaction establishes an auditable classification for every legacy inventory entry, compiles a bounded set of actual original support implementations in production targets, and proves at runtime that the linked implementations—not bootstrap substitutes—execute.

Authority: `delivery/milestones/cnc-generals-zero-hour/M19-original-support.md`, `docs/zero-hour-source-engine-migration.md` SE-001/SE-002/SE-010, and the preservation rules in `docs/zero-hour-linux-port-plan.md`.

## Scope boundaries

- The repository-owned `GeneralsMD` source tree may receive narrow portability corrections.
- The retail `original_game_symlink` is read-only and is not read, copied, hashed, or committed by this transaction.
- Existing native components remain useful fixture/component evidence. Bootstrap registration and the Alpha/Bravo simulation are explicitly non-production.
- M20 factories/lifecycle and all later source-engine integration are excluded.

## Slice index

1. [Slice 01 — auditable source classification and identity gate](milestone_19_plan_01_slice_01.md): completed (`95a2c31`)
2. [Slice 02 — original compression runtime boundary](milestone_19_plan_01_slice_02.md): completed (`4679929`)
3. [Slice 03 — original WW support runtime boundary and milestone evidence](milestone_19_plan_01_slice_03.md): completed (commit pending)

## Validation strategy

Each slice runs focused positive and negative tests. At milestone completion, build and run the asset-free suite under all four native presets, run the original-support subset under ASan/UBSan, and record compiler/link/runtime identity evidence. The negative identity control must fail when a required original provider is disabled or replaced by the bootstrap implementation.

## Decisions

- Source identity is established by generated classification data, build-system source membership, final-link inspection, and runtime witnesses emitted from original translation units. No one signal is accepted alone.
- Classification describes the current migration boundary: production-compiled, platform-replaced, tool-only, or excluded/deferred with provider and rationale. Later milestones may promote deferred entries without rewriting history.
- Narrow portability shims may replace Win32 compiler/CRT surfaces while preserving original algorithms and public source interfaces.

## Completion record

All three slices established complete source classification and bounded original-source compile/link/runtime boundaries for EAC RefPack plus the dependency-closed WWLib/WWMath/WWSaveLoad support set. All four native preset suites pass 62/62, and the Clang ASan/UBSan original-support suite passes 7/7, including clean allocator teardown. GameEngine and device-provider integration remain explicitly deferred to M20–M25. The outer orchestrator owns milestone status and delivery-goal transitions.
