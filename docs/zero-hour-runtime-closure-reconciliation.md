# Original runtime dependency reconciliation

Status: accepted engineering resolution, implementation-ready, 2026-09-20.

Authority: owner request to run architecture reconciliation and delivery replanning. Governing product scope remains `docs/zero-hour-linux-port-plan.md`. This resolves `M20_SOURCE_ORDER_DEFECT` and supersedes the ordering and allocation of work in `docs/zero-hour-source-engine-migration.md` where they conflict with this document. All SE-001–SE-010 behaviors and final acceptance obligations remain required. No new product decision is requested.

```yaml
product_id: cnc-generals-zero-hour
change_id: RUNTIME-CLOSURE-2026-09-20
input_mode: CHANGE_PLAN
implementation_required: true
implementation_ready: true
delivery_planning_ready: true
decisions_required: []
remediation_ids: [RC-001, RC-002, RC-003, RC-004, RC-005]
```

## RC-001 — One owner for the coupled runtime implementation

Choose an integrated runtime provider before claiming complete original lifecycle execution. That provider owns the transitive implementation dependencies of original `GameMain`, `GameEngine::init`, `postProcessLoadAll`, `execute`, `update`, reset and destruction, even where those implementations were previously allocated to later simulation, presentation, media, persistence or networking milestones. It is explicitly authorized to compile and port these required providers together. A later domain acceptance milestone must never be an implementation prerequisite of this provider.

The compiler determines milestone identities and grouping. Preserve completed records and stable IDs where practical; revise affected incomplete contracts so they distinguish shared runtime implementation from later feature completion/acceptance. This is a substantial source port, not a small startup wrapper. Its size warrants multiple coherent, tested implementation slices inside one milestone transaction; size or routine missing symbols do not themselves require another architecture referral.

Original source dependencies, not the current classification's milestone labels, define the implementation closure. Update that classification during implementation as each real provider is ported. Full Linux porting of every original source is unnecessary: genuinely unused tools, removed online-service integration and platform implementations have explicit exclusions/replacement providers. Every required constructor, registry entry, virtual method, parser callback and shutdown path must resolve to original behavior or an authorized OS/device/library adapter. Do not rely on linker garbage collection, weak stubs or unresolved-symbol suppression to hide required behavior.

## RC-002 — Required closure and allowed boundaries

The following table defines responsibility and independent validation boundaries. All rows required by the chosen startup/runtime mode belong to the integrated provider, without dependencies on later domain acceptance.

| Original consumer / source evidence | Required implementation | Evidence before provider completion |
|---|---|---|
| `GameMain.cpp:40–48`, `GameEngine.h` | Original class/interface, Linux factory, original entry/lifecycle methods and exception-safe ownership | Actual source initialization/update/teardown; factory failure; no alternate class |
| `Precompiled/PreRTS.h:44`, original strings/pools and subsystem types | Remaining original ABI/PCH/CRT portability, UTF-16 call sites, allocators, logging and source headers | Compile original consumers; actual string/pool boundary tests and sanitizers |
| `GameEngine.cpp:301–354,412` | Original FileSystem/LocalFileSystem/ArchiveFileSystem interfaces bound to existing VFS, original INI dispatch, CSF/GameText, GlobalData and name keys | Source parser consumes fixture and supplied retail data; source-owned values; malformed/missing inputs fail |
| `GameEngine.cpp:434–482`, `Common/Thing/ModuleFactory.cpp`, `W3DModuleFactory.cpp` | Audio manager, function lexicon, complete required module registry, object factory, particles and data stores including weapons/locomotors | Resolve real registered modules from actual configuration, instantiate and release representative modules; unknown required module fails |
| `GameEngine.cpp:493–512` | Original GameClient, AI, GameLogic, team/player/crate services, recorder, radar and victory services | Construct/init real providers, preserve globals and ordering; original update reaches real state without a toy loop |
| `GameEngine.cpp:539–553` | GameState/GameStateMap, Xfer/CRC and post-load relationships | Required startup traversal and CRC use original instances and resolve references; full save/replay scenarios remain later acceptance |
| `GameEngine.cpp:755–786,797–895` | Client/audio/radar/message updates, original simulation update, script/view timing, optional network branch | Bounded original execute/update and controlled quit; required startup and update calls cannot silently no-op |
| Original W3D/WWShade/GameClient device consumers | Original presentation producers and their CPU resources, renderer interface adapters, original font/UI/media manager interfaces as required by startup | Recording/null device tests at the external presentation boundary; no replacement GameClient/module registries |

Paths without a prefix are under `GeneralsMD/Code/GameEngine/Source/Common/`; W3DModuleFactory is `GeneralsMD/Code/GameEngineDevice/Source/W3DDevice/Common/Thing/W3DModuleFactory.cpp`. Line references describe baseline `5f4949e` and are evidence anchors rather than an exhaustive source list.

Headless mode may use the plan's null/recording graphics and audio sinks at device boundaries. They must preserve real original GameClient, game state, module registration, data loading and resource ownership; an empty factory is not a null device. Asset verification can exit before initialization as already specified. A bounded runtime test may request controlled quitting after genuine original updates; setting quitting before all updates cannot prove execute acceptance.

Original startup intentionally leaves `TheNetwork = NULL` outside a network game (`GameEngine.cpp:565–566`). Preserve that source-defined offline state. Compile any network symbols required by the offline closure and explicitly reject unsupported network entry until later LAN work implements it. Full POSIX lobby/match transport, loss handling and external peers are not prerequisites to offline startup. Likewise, all required recorder/save types and startup operations belong to this closure; later save/replay acceptance owns remaining operations not reachable at this boundary. An unavailable operation must fail clearly if entered, never return success or fabricate state.

Selected SDL_GPU, miniaudio, FFmpeg and font backends already exist as components. Bind their original interfaces as needed by the closure. Later real-GPU visuals, interactive media/UI flows and gameplay scenarios may require further feature implementation; moving their startup dependencies earlier does not waive those later gates. Remove the original retail-file deletion and protection/online-service behavior already excluded by the base plan. Never modify the retail symlink's content.

## RC-003 — Preserve the whole acceptance chain

After integrated provider acceptance, retain independent original map/script/AI/skirmish behavior, real-source rendering/Vulkan, original UI/input/audio/video, original save/replay determinism and original LAN contracts. These consumers may implement additional behavior outside the earlier reachable closure and extend tests, but cannot retrospectively supply code required for its accepted runtime. Keep playable M15, full-match M16, clean-release M17 and optional Windows M18 obligations intact. All factions, real assets and long-run criteria remain governed by the base plan.

If the compiler combines contracts, map every moved clause explicitly and retain history; no formerly required test disappears. Source packet order must account for both compile/link dependencies and runtime dependencies, including vtables, registry callbacks and teardown. Paper acyclicity alone is insufficient.

## RC-004 — Original behavior and scope of M19 evidence

Preserve M19's completed commits and bounded ten-provider evidence. Those do not establish portability of original GameEngine UnicodeString, MemoryPool or INI consumers: remaining work is expressly authorized in the integrated provider. M19's UTF-16/rounding probes call new foundation helpers, so they cannot discharge corresponding original call-site migration. No redo of already evidenced RefPack/WW support is required without a demonstrated defect.

For every runtime claim require the actual original class definitions, method bodies, registered providers and observable state changes. A new implementation under an original filename behind a platform macro is a replacement, not a source port. Narrow platform conditionals, shared-method refactoring that preserves behavior, and explicit device adapters are allowed; reduced duplicate classes/globals and trace-only lifecycle stages are prohibited. Strengthen identity tests to reject alternate reduced classes, missing registry providers and witnesses that succeed without original state transitions. Removing a required original provider must fail a compile/link/runtime gate, not merely a filename policy test.

Provider acceptance requires four compiler/configuration builds, full asset-free regressions, focused sanitizers, fixture and read-only retail initialization, missing/malformed configuration and factory failures, reverse exactly-once teardown, arbitrary CWD, isolated XDG state and proof that headless/verify modes acquire no window/GPU. Test startup/one genuine update with real subsystem instances; it need not prove a complete campaign or hardware scene, which have later owners. No gameplay completion claim is authorized by successful startup.

## RC-005 — Durable planning and adoption

Historical M0–M14 component qualifications and M19 acceptance remain unchanged. Preserve the blocked M20 plan 01 and its rejected-approach evidence. A subsequent delivery resume must adopt the new committed packet explicitly and use a new governing plan for any changed active milestone. The compiler owns stable IDs, contract allocation, dependencies and execution order; readiness must check the actual provider edges in RC-002, including the recorder/game-state and offline network boundary, before declaring readiness.

Readiness is permission to begin porting from existing source/toolchain, not a claim the closure already builds. ATL errors, missing original provider implementations and classification corrections are deliverables within the revised scope. Retail access is required for runtime integration acceptance; display/GPU access begins at real presentation acceptance; clean distro environments and optional Windows fixtures remain their established later gates. No product code or implementation slice is created during this reconciliation/planning request.
