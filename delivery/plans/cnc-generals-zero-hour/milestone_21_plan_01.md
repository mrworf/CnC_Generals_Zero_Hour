# M21: original map and simulation execution

## Outcome

The production Linux original-engine entry loads project-owned and read-only retail `.map` files through the original chunk/map parsers, establishes real mission and skirmish state, advances original object/command/script/AI/victory behavior, and can return, reset, and enter another scenario without stale state. Deterministic simulation checkpoints remain unchanged when unrelated client/audio randomness is varied.

## Delivery-goal context

- Goal status record: `workflow/delivery/state.yaml`
- Product ID: `cnc-generals-zero-hour`
- Active packet and revision: `delivery/milestones/cnc-generals-zero-hour/status.yaml`, `sha256:21ce030a34f700bbcf7773983454f0b741f384956f1ff6568d7fc6782bc5bfe9`
- Source transaction: `cb567c223beab5d63fa2f13d66718e670271d3d0`
- Planning transaction: `cd2389e40aa89b1c2d1e02be638d084a95438f2d`
- Fixed goal scope: M26, M27, M28, M20, M21, M22, M23, M24, M25, M15, M16, M17, M18; context only
- Current milestone: M21
- Resume mode: new, from transaction-start `97abf48ad0d7b73ba79df3f4ce467d973928b4ea`

## Governing contracts

- Milestone: `delivery/milestones/cnc-generals-zero-hour/M21-original-simulation.md`
- Architecture: `docs/zero-hour-runtime-closure-reconciliation.md` RC-002, RC-003, RC-004, RC-007 through RC-012
- Migration: `docs/zero-hour-source-engine-migration.md` SE-004 and SE-010
- Direct accepted provider: M20 commit `b21d43076f01bfd4fc80091e8c70f67769035670`, with `evidence/qa/cnc-generals-zero-hour/M20-plan03-original-lifecycle-acceptance.md`
- Base behavior and validation: `docs/zero-hour-linux-port-plan.md` sections 1 through 6 and 10
- Repository instructions: no applicable `AGENTS.md` was found at transaction start.

## Current-state findings

- M20 compiles the original `GameLogic`, parser, registry, player, AI, script, object, and client translation units and accepts bounded no-match lifecycle behavior. It deliberately does not dispatch an initial map or prove scenario behavior.
- `GameLogic::startNewGame`, `TerrainLogic::loadMap`, `SidesList`, `ScriptList`, `DataChunkInput`, command dispatch, and victory conditions are present original implementations. The Linux production factories still expose a CPU-only `TerrainVisual` adapter whose `addProp` operation is empty and have no M21 scenario witness.
- Original `MapObject` construction and map-object chunk parsing currently live with `WorldHeightMap.cpp`, whose physical W3D dependencies cannot be pulled wholesale into a headless target. RC-004 permits a narrow shared-method refactor while requiring the original bodies, state, formats, and ownership.
- Existing `zh_singleplayer` tests are modern component fixtures and cannot establish M21 original-source acceptance. M21 needs a separate production-linked original-simulation target, identity gates, and provider-removal controls.
- The canonical validation baseline is four CMake presets, the complete asset-free CTest suite once at milestone completion, and focused Clang ASan/UBSan. Retail maps are a separately gated read-only corpus; committed output may contain logical counts/outcomes only.

## Decisions

- Keep the production `GameLogic::startNewGame` path authoritative. Test hooks may request a bounded scenario and expose derived checkpoints, but may not recreate map, command, script, AI, or victory state.
- Shared-compile the original CPU map-object/chunk logic away from W3D device code where required. The W3D build and Linux headless build consume one canonical implementation; no second parser or substitute entity model is permitted.
- Device behavior remains behind Linux adapters. M21 implements every CPU constructor/state/method/destructor reached by map start, including observable prop, preload, and recorder-control state; it does not acquire a window, GPU, audio device, or render pixels.
- Use compact owned binary `.map` fixtures written from the documented original chunk schema and consumed by the production parser. Fixture generators are test-only; successful assertions must come from original source-owned state.
- Mission and skirmish use the same production entry with explicit mode/fixture selection. Missing providers, malformed chunks, invalid commands, and partial initialization fail closed and unwind before a success checkpoint.

## Scope

### Included

- Original initial `.map` dispatch, map/world/side/script/object/terrain parsing, and source-owned state witnesses.
- Original player/team setup, map object creation, message/command dispatch, object update effects, script and AI advancement, and victory/defeat transitions for bounded mission and skirmish fixtures.
- GameClient preload, TerrainVisual prop, and Recorder control operations reached by those scenarios.
- Deterministic checkpoints independent of unrelated client/audio random activity.
- Missing/malformed/provider-removal controls, reset/return/re-entry, four presets, full asset-free CTest, sanitizers, and gated read-only retail-map evidence.

### Excluded and deferred

- ARM64, base Generals, tools, gameplay redesign, retail redistribution or writes.
- Hardware/session rendering and pixel acceptance (M22), interactive UI/input/audio/video acceptance (M23), full save/replay compatibility (M24), and LAN acceptance (M25/M16).
- Replacing original parsers/state with labels, toy entities, proxy success, or the existing modern `zh_singleplayer` model.

## Slice index

| Slice | Plan | Outcome | Dependencies | Status | Commit | Evidence |
|---|---|---|---|---|---|---|
| 01 | [slice 01](milestone_21_plan_01_slice_01.md) | Original CPU map/chunk dispatch publishes map objects, world, terrain, sides, and scripts transactionally | M20 | complete | `8e43e63` | `evidence/qa/cnc-generals-zero-hour/M21-plan01-slice01-original-map.md` |
| 02 | [slice 02](milestone_21_plan_01_slice_02.md) | Production mission/skirmish entry creates original players, objects, and reached client CPU state | 01 | complete | `d3cff5f` | `evidence/qa/cnc-generals-zero-hour/M21-plan01-slice02-scenario-setup.md` |
| 03 | [slice 03](milestone_21_plan_01_slice_03.md) | Original commands, updates, scripts, AI, and victory produce deterministic checkpoints | 02 | complete | this slice commit | `evidence/qa/cnc-generals-zero-hour/M21-plan01-slice03-original-simulation.md` |
| 04 | [slice 04](milestone_21_plan_01_slice_04.md) | Return/reset/re-entry, identity, retail gate, and cumulative milestone acceptance pass | 03 | pending | | |

## Cross-slice concerns

- Compatibility and migration: preserve original chunk versions, field widths, UTF-16 rules, map lookup, update order, and source-defined state ownership.
- Authorization and security: retail trees are read-only; tests isolate XDG state and never commit private paths, hashes, or bytes.
- Invalidation and lifecycle effects: parsers publish state only after success; reset and failure unwind release map objects, callbacks, player/team/script/AI state, and client-side CPU resources exactly once.
- Audit and observability: identity and provider-removal tests bind witnesses to production original objects and translation units; checkpoints contain derived logical state only.
- Performance and scale: fixture and retail parsing is bounded; checkpoint collection does not alter update scheduling.
- Environment or external services: default tests need no GPU, display, audio device, network, or retail tree. Retail validation is explicitly gated by the repository's provisioned read-only roots.

## Milestone completion gate

- Focused original-simulation positive/negative, identity, provider-removal, lifecycle, and deterministic-randomness tests pass.
- All four presets configure and build applicable targets and pass focused original-simulation tests.
- The complete asset-free CTest suite passes once in all four presets.
- Relevant Clang Debug ASan/UBSan tests pass with the repository's documented leak-detection limitation recorded.
- Gated read-only retail mission and skirmish map acceptance passes without private-data disclosure or writes.
- Acceptance evidence maps every M21 clause to source-owned state and records no renderer, full persistence, or LAN claim.

## Rollback and recovery

Each slice is independently revertible. Shared source extraction must leave a single canonical implementation consumed by all configured targets. Failed map loads and interrupted scenarios reset to the pre-entry state; no rollback touches retail data or user XDG data outside test-owned temporary roots.

## Execution notes

Planning completed before production edits. Investigation found that original CPU map-object parsing is co-located with W3D `WorldHeightMap`; slice 01 owns the narrow source refactor rather than introducing a parallel parser.

## Deferred follow-ups

- Hardware world presentation, interaction/media, full save/replay, and LAN remain with M22 through M25.
