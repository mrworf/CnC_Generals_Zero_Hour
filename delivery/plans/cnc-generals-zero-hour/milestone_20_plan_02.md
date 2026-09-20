# M20 plan 02: integrated original runtime and data lifecycle

Status: active

## Outcome

The native x86-64 Linux executable enters through the original `GameMain`, constructs a Linux device factory for the original `GameEngine`, loads configuration through the complete original dispatch and registries, performs bounded source-owned client and logic updates, resets, and tears every published subsystem down exactly once. Fixture and separately gated read-only retail initialization use the same runtime. Headless operation replaces only physical device edges; it does not replace `GameEngine`, `GameClient`, `GameLogic`, registries, or source state.

## Delivery-goal context

- Product ID: `cnc-generals-zero-hour`
- Milestone contract: `delivery/milestones/cnc-generals-zero-hour/M20-original-lifecycle.md`
- Immutable packet fingerprint: `sha256:0d5f1fef5dcec3bbbbb26656c5aaf591d653e2454501b51cfb09059fbb6acd7a`
- Transaction-start `HEAD`: `4c79b0cbfc4f346cf2ae7875dd6a865b0a1a31ed`
- Accepted direct prerequisite: M28 (`5d6f8f490387548a94cfa95ecbefdd27b5000aa8`), transitively M27 (`d46a6ed1e65e494421ed2c7f80e3b2107ca25569`) and M26 (`89eeca9506aff87649bb0ee23334af83f18a51b8`)

## Reconciliation of blocked plan 01

[Plan 01](milestone_20_plan_01.md) and its [blocker evidence](../../../evidence/qa/cnc-generals-zero-hour/M20-original-lifecycle-blocker.md) remain immutable history. They correctly rejected a reduced `GameEngine`: source initialization requires concrete simulation, GameClient/W3D, media, module, and object providers before an executable boundary exists. The replacement packet resolves that source-order defect by:

1. accepting M26 process/ABI, M27 data, and M28 CPU presentation/media seams first;
2. explicitly authorizing M20 to compile and port every remaining provider reachable from original startup, even when its later domain acceptance belongs to M21-M25; and
3. keeping M21-M25 as behavior/acceptance extensions, never code prerequisites for M20 startup.

This plan therefore does not repeat plan 01's impossible ordering and does not hide it behind an empty registry, alternate lifecycle, trace-only state, linker suppression, or proxy success. The source closure discovered while compiling remains M20 work unless it demonstrates a genuine product/backend incompatibility.

## Governing authority

- `docs/zero-hour-linux-port-plan.md`
- `docs/zero-hour-source-engine-migration.md`, where not superseded
- `docs/zero-hour-runtime-closure-reconciliation.md` (`RC-001` through `RC-011`)
- M20 milestone contract and readiness manifest
- Accepted M26-M28 plans, evidence, and checked dependency ledger

No repository-local `AGENTS.md` exists. CMake presets and the existing asset-free CTest suite are canonical.

## Provider inventory and ownership boundary

M20 consumes, rather than reimplements, the accepted original process (`zh_original_process`), data (`zh_original_data`), and CPU resource (`zh_original_resources`) providers. It promotes the original runtime closure rooted at `GameMain.cpp`, `GameEngine.cpp`, `SubsystemInterface.cpp`, and the Linux `CreateGameEngine` factory. The closure includes the complete production INI table, original local/archive filesystem interfaces, GameText/GlobalData/name keys, module and thing factories, function lexicon, data stores, GameState/Xfer/CRC, GameClient, GameLogic, AI/player/team/crate/radar/victory/message/script/view/recorder/audio services, map cache/localization, and every reachable constructor/vtable/callback/global/shutdown implementation. Network remains source-defined null offline, with required symbols present and unsupported entry explicit.

M21 retains full map/mission/skirmish acceptance, M22 physical Vulkan scene and pixel acceptance, M23 interactive UI/media acceptance, M24 full save/replay scenarios, and M25 lobby/match transport. Those milestones may extend behavior but may not first supply a symbol or startup branch needed here.

## Slice index

| Slice | Plan | Independently reviewable outcome | Status | Commit |
|---|---|---|---|---|
| 01 | [Original subsystem ownership and lifecycle spine](milestone_20_plan_02_slice_01.md) | Actual subsystem list and original entry/lifecycle translation units compile behind the Linux factory boundary; pending-init failures and reverse ownership are executable without an alternate engine. | complete | `a424490a505c5a662d370d2e9f1bc8c4dab56b4a` |
| 02 | [Complete dispatch and live provider closure](milestone_20_plan_02_slice_02.md) | The complete production INI table and every referenced callback/provider body compile and link live; actual dispatch accepts a representative production block and rejects an unknown block without discarded-section or reduced-table evidence. | complete | `3fe448f6a157db79fd7fef99e5866c07f19e8a3b` |
| 03 | [Complete data/configuration startup](milestone_20_plan_02_slice_03.md) | Original filesystem, INI/CSF/GlobalData/name-key, nonempty map-cache/localization, and GameState/Xfer/CRC consumers execute on owned fixtures with XDG-only writes. | complete | `b7c83dc34d441a093f5c31dad33e0c48bb6ab5d8` |
| 04 | [Integrated headless update and reset](milestone_20_plan_02_slice_04.md) | Original GameClient/GameLogic and services perform bounded genuine updates, reset through original UI/layout resources, and reject unsupported network entry without devices. | complete | `c4a2a4d9e5b8a07145860ce6f4213852ffa4fd47` |
| 05 | [Entry, side effects, and failure semantics](milestone_20_plan_02_slice_05.md) | Original `GameMain` is the executable path; LOD/cache/benchmark and every staged failure preserve diagnostics, XDG isolation, no-device behavior, and exactly-once unwind. | complete | `4202b6b50eec9791bf6b78952bd904710bd06ec8` |
| 06 | [Identity, retail gate, and milestone assurance](milestone_20_plan_02_slice_06.md) | Four presets, full regression, sanitizers, provider-removal, source identity, fixture and gated read-only retail evidence close every M20 criterion. | blocked | [retail W3D registry evidence](../../../evidence/qa/cnc-generals-zero-hour/M20-plan02-retail-w3d-blocker.md) |

## Cross-slice constraints

- Native x86-64 Linux only; no ARM64 work.
- Original classes and state are authoritative. Platform/device adapters may replace only OS, window, GPU, audio-device, online-service, and excluded protection edges.
- No retail bytes, hashes, or private paths are copied, committed, logged, or embedded. Retail roots remain read-only and all generated output uses isolated XDG roots.
- No `NDEBUG` workaround is added to an M20 target. Tests use active checks in Debug and Release.
- The checked dependency ledger and source classification are extended whenever a provider is consumed; changed source or registry lists invalidate stale evidence.
- Each slice has positive and negative executable tests. The canonical full suite runs once after the final slice stabilizes; focused suites run per slice.
- A reviewer can revert any slice without needing a later slice to preserve the earlier slice's stated executable behavior.

## Discovered provider closure

Slice 01 reached the actual `GameEngine.cpp` include graph under both GCC and Clang. Its additional closure is mechanical rather than a new architecture dependency: case-correct Linux includes, a narrow PCH/CRT compatibility surface, and 47 legacy opaque enum declaration/definition pairs whose underlying `int` size and enumerator bodies are checked against the activation baseline in `docs/original-runtime-enum-abi.tsv`. These sources remain M20 startup prerequisites even where the owning domain is accepted later; no slice reordering or new milestone is required.

Slice 02 then proved that a compile-only callback set is not an executable boundary. A construct-only probe linked only because section collection discarded `INI::load`, `theTypeTable`, and the callbacks; it was rejected. The first probe that actually invoked `INI::load` retained the complete production table and exposed its required ThingFactory, image, weapon, animation, AI, ControlBar, terrain, particle, upgrade, video, and related provider bodies. Those were formerly assigned to slice 03, so the original slice 02/03 order repeated a narrower form of the source-order defect preserved from plan 01. This amendment moves the live callback/provider link closure into slice 02 and makes full nonempty data/cache/localization execution slice 03. No reduced table, weak symbol, unresolved suppression, or dead-section identity is accepted.

The executable closure is configuration-sensitive: Release retained additional real vtables and command/state paths that Debug section collection did not expose. M20 therefore also owns the reached ActionManager, Squad, AI dock/guard/tunnel state machines, supply/hack/prone updates, resource/tunnel tracking, and Line2D providers. This is not later-domain acceptance; it is the production link closure required by the 64-entry INI dispatch table. `docs/original-runtime-dependency-ledger.tsv` records the representative runtime/link edges, while `original_config_dispatch_identity` derives the complete callback inventory from `INI.cpp` and requires every callback to resolve to the exact dispatch object or one unambiguously compiled and actually extracted archive member in every preset.

Slice 04's first genuine `GameEngine::update` and `reset` execution reached further source-owned lifecycle providers: GameLogic, GameClient, MessageStream, PlayerList, Radar, InGameUI, audio/CD, WindowLayout, and GameWindowManagerScript, plus their allocator and teardown dependencies. Release retention additionally made the complete DockUpdate base vtable live, so its original provider and two standard C++ loop-scope declarations remain M20 closure work. Release teardown also exposed an original empty-list null dereference in `PolygonTrigger::deleteTriggers`; the narrow null guard preserves populated-list destruction and permits genuine empty TerrainLogic teardown. The focused executable calls the original lifecycle methods and the link map proves the named provider archive members were extracted; deleting the extracted GameLogic member makes the link fail on its required symbols. Its callback-free `BlankWindow` uses test-only draw and null-lexicon boundaries solely to exercise original reset without a physical display. A callback-bearing WND is rejected at that boundary. Slice 05 must replace those bounded harness seams with actual GameText/localization, FunctionLexicon callbacks, and the production factory path; they cannot establish production-entry acceptance.

Slice 05's production entry replaces those test-only seams with actual GameText and the complete 242-name `FunctionLexicon`. The checked online boundary contains 96 exact name-specific fail-closed callbacks (95 lexicon entries plus the reached Buddy control callback); the remaining 147 lexicon callbacks resolve live from their original providers. `cmake/LegacySourceInventory.cmake` is the authoritative repository record for the excluded GameSpy SDK and retired online-service sources, and `original_production_function_lexicon` checks that record, live symbols, boundary-object extraction, and name ownership together. The owned production fixture reached the end of original initialization, Shell/MainMenu state, two genuine GameLogic/GameClient updates, original reset, and GameMain destruction. This exposed two optional-resource null defects (`GameWindowTransitionsHandler::remove` and `ControlBar::update`) and two parsed process globals (weather and water transparency) outside subsystem ownership; the narrow guards and explicit end-of-engine override-chain destruction preserve populated behavior while restoring exact logical allocation baseline. It also proved that legacy `_DEBUG/_INTERNAL` guards made `-buildmapcache` and `-benchmark` unreachable in every CMake preset, so their existing parsers/state were made preset-independent. Cache-build now follows its original controlled init-time exit, while bounded benchmark remains device-free in Debug and Release. No slice reorder or later-domain prerequisite results.

The optimized preset pass additionally exposed legacy null-`this` deletion in released or empty owners (`MemoryPoolObjectHolder`, `SidesInfo`, and the optional script-condition cache). Explicit pointer guards preserve populated destruction and make the empty lifecycle well-defined. The complete Clang closure reached five more opaque enum families and `QuickTrig`'s obsolete `register` specifiers; the former are added to the checked int-size/value ABI inventory and the latter are removed without semantic change. These findings remain slice-05 portability work and do not add a provider or milestone dependency.

Slice 06's explicitly provisioned read-only retail gate exposed a remaining source-order dependency not covered by the accepted provider seams. Retail object initialization reaches `W3DDefaultDraw`; the common `ModuleFactory` has no W3D `ModuleData` provider, while the authoritative 19-entry `W3DModuleFactory` closure is still classified to M22-M23 and directly reaches the excluded Direct3D 8 SDK at compile time. The [blocker evidence](../../../evidence/qa/cnc-generals-zero-hour/M20-plan02-retail-w3d-blocker.md) records the exact source path and permitted resolution choices. No reduced registry, generic data parser, or success/no-op draw provider is retained. Plan 02 remains governing history but cannot complete until the M20/M22-M23 boundary is reconciled again.

## Completion gate

All six slice contracts must be complete. The same explicit offline profile must pass GCC/Clang Debug/Release through original initialization, post-load, genuine client/logic updates, reset, and destruction. Source-owned state before/after updates, complete registry behavior, process/allocator enclosure, exact lifecycle counts, compile/live-symbol provenance, provider removal, arbitrary CWD, isolated XDG, no-device interception, malformed/missing inputs, and every staged failure must pass. Focused Clang ASan/UBSan records exact options and independent ownership counts; leak-disabled execution is not represented as leak proof. The full asset-free CTest suite and the separately provisioned read-only retail initialization gate must pass.

## Rollback and recovery

Each slice is one commit. A failed slice remains uncommitted unless stable useful progress plus a genuine blocker must be preserved. Plan 01 and its evidence are never rewritten. Generated build trees are disposable; rollback never changes user data, retail roots, or XDG state.
