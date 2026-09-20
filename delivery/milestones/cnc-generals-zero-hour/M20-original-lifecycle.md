# Milestone M20: integrated original runtime and data lifecycle

## Objective

Linux initializes the original engine with its complete offline runtime provider closure, advances genuine original state, and tears down every owned subsystem correctly.

## User/System Outcome

Original GameMain, GameEngine and required registered subsystems compile, link and execute together with fixture and read-only retail configuration, without a display or GPU requirement.

## Scope

Own the transitive implementation closure of original GameMain, GameEngine::init, postProcessLoadAll, execute, update, reset and destruction. Port required providers together, even where classification previously assigned them to M21–M25. Update source classification as each real provider is integrated.

Implement Linux CreateGameEngine and original subsystem factories; adapt original FileSystem/LocalFileSystem/ArchiveFileSystem to BIG/VFS and XDG. Port remaining original ABI/PCH/CRT, UnicodeString/UTF-16 call sites, MemoryPool, logging and subsystem headers. Initialize original INI dispatch, CSF/GameText, GlobalData, name keys, module/object factories, function lexicon, weapons/locomotors, particles and data stores.

Include original GameClient/W3D/WWShade resource producers, GameLogic, AI, player/team/crate services, radar, victory services, message/script/view updates, audio/media interfaces, recorder, GameState/GameStateMap and required Xfer/CRC startup/post-load relationships. Every required constructor, vtable, registered callback and teardown path resolves to original behavior or an authorized platform/device adapter. RC-002's full provider table is the minimum responsibility boundary, not an exhaustive file list.

## Explicit Exclusions

Full map/mission/skirmish scenario acceptance is M21; hardware scenes M22; interactive UI/media flows M23; full save/replay scenarios M24; POSIX lobby/match transport M25. Only behavior not required by this milestone's reachable initialization/update/reset/destruction closure may remain there. Offline TheNetwork = NULL is source-defined; required link symbols are still owned here and unsupported network entry must fail clearly. Required proprietary gaps follow the base-plan evidence gate. No ARM64, base Generals executable/tools, retail redistribution or gameplay rewrite.

## Source Requirements

[Runtime reconciliation](../../../docs/zero-hour-runtime-closure-reconciliation.md): RC-001, RC-002, RC-004, RC-005 and RC-006; RC-003 retains downstream acceptance. This supersedes conflicting ordering in the [migration supplement](../../../docs/zero-hour-source-engine-migration.md): SE-002 remaining original call sites, SE-003, reachable providers from SE-004–SE-008, SE-001/SE-010 identity and assurance. [Base port plan](../../../docs/zero-hour-linux-port-plan.md) §§1–6 and §10 remain governing scope/compatibility authority. Revisions/commits are in status.yaml.

## Preconditions

Direct providers: M19, M4, M6, M7, M8, M12, M13. M19 supplies bounded original support, M4 supplies VFS/corpus, M6/M7/M8 supply platform/recording/font components and M12/M13 supply audio/video backends. Consume evidenced contracts, not historical title/status alone. None establishes original runtime integration. M21–M25 are never prerequisites to M20. Missing original providers, ATL/header errors and portability work are deliverables here. Readiness separately audits external inputs.

- `PRE-028` — M19 bounded ten-provider original support acceptance; PRE-009 — retained M4 VFS/corpus component; PRE-019/PRE-010/PRE-020 — M6/M7/M8 platform, recording and font components; PRE-011/PRE-023/PRE-024 — M12/M13 pinned audio/video backends. These components are available; remaining original call-site portability belongs here.
- `PRE-008` — user read-only roots at retail initialization validation. PRE-029 is produced here and includes all RC-002 runtime providers, recorder/GameState/Xfer/CRC and offline network boundary; it is not an entry prerequisite.

## Readiness checks

- Run `cmake --list-presets` from the repository root: all four native presets must resolve. Inspect status.yaml and linked provider acceptance for the direct dependencies; require the original-source grade for new providers, not only historical component status.
- Inspect [prerequisite manifest](../../readiness/cnc-generals-zero-hour/prerequisite-manifest.md) for the IDs above and their current verification. Use its retail/session checks only at the named validation boundary. No unfinished feature acceptance command is an entry requirement.
- Inspect the RC-002 semantic audit in the [readiness report](../../readiness/cnc-generals-zero-hour/readiness-report.md#7-dependency-graph-and-source-semantic-audit): original factory/vtable/registry, initialization, update/reset and teardown dependencies must all have this milestone as implementation owner. No M21–M25 acceptance or completed runtime build is required before work begins.
- `test -d original_game_symlink && test -d original_game_symlink/ZH_Generals && git check-ignore original_game_symlink` — supplied roots remain available and ignored for gated retail initialization; use local retail variables without recording private paths.

Produces `PRE-029` (M20 complete original offline runtime and data lifecycle); inspect this contract's source-owned positive and negative acceptance evidence before downstream consumption.

## Functional Requirements

Witness original GameMain -> GameEngine::init/postProcessLoadAll/execute/update and source-owned initialization and update state. Controlled quitting occurs after at least one genuine original update. Missing data/factories and initialization-stage failures unwind exactly once in reverse order. Preserve original globals, registry callbacks and reset ownership. Original INI/CSF consumers must produce source-owned values; resolve, instantiate and release representative real modules from configuration. Unknown required modules fail. Required startup GameState/Xfer/CRC traversals use original instances and resolve references.

Null/recording sinks are allowed only at external graphics/audio device boundaries. Keep real original GameClient, resource ownership, registries and state. Verify-assets may exit before initialization as specified by the base plan. Any unavailable operation outside the offline closure must fail clearly when entered. Remove excluded retail deletion/protection/online-service behavior. Preserve useful native backend implementations.

Use one explicit offline headless test profile across all four presets: no active match, existing `-noshellmap` semantics, intro/sizzle disabled through explicit test-mode GlobalData configuration, bounded genuine updates, original reset, then orderly quit. Release must not rely on a Debug-only command-line option. Preserve normal gameplay defaults and original GameClient, window manager, script/view and GameLogic updates. Full cinematics, shell maps and interactive flows retain their later domain acceptance owners.

Enumerate logical resources as consumers are connected, including font/WND/INI/CSF dependencies and `Menus/BlankWindow.wnd` during reset. Original UI/layout objects and parsers remain required with null devices. Project-owned fixtures exercise real callbacks without retail-dependent skips; separately provision full read-only retail initialization without copying assets or changing data roots.

Correct partial-initialization ownership: a pending subsystem that fails before registration must be released once and any published singleton cleared. Then unwind previously registered subsystems in reverse order. Safely handle not-yet-created globals in constructors/destructors and GameEngine destruction; preserve successful initialization order and memory-pool semantics. Stop/finalize workers before owner release and preserve the original error if teardown diagnostics also fail.

## Architecture / Security Constraints

Original engine owns authoritative state/lifecycle; native adapters own OS/device/library edges. Preserve original allocators except narrow characterized portability changes. Retail roots/symlinks are read-only; never commit retail bytes/hashes/private paths. Isolate XDG writes. No alternate reduced class hidden behind a platform macro, duplicate globals, empty registry, trace-only lifecycle stage, weak stub, unresolved-symbol suppression or linker collection that hides required behavior is acceptable. Narrow platform conditionals and shared-method refactoring must preserve source behavior.

## Interfaces and Compatibility

Preserve original engine interfaces and fixed-width, char16_t/UTF-16LE, bounded serialization and floating-point rules; never -fshort-wchar. Incremental source targets coexist with fixture-only scaffolding, never parallel authoritative states. Rollback uses reviewed commits without overwriting user saves/configuration or retail. Keep valid native component APIs unless source integration proves a narrow correction necessary.

## Acceptance Criteria

- [ ] The stated outcome and all scoped consumers execute original implementations with recorded source-owned transitions or outputs.
- [ ] Original entry/init/post-load/execute/update/reset/teardown run with all RC-002 providers; genuine source state changes before controlled quit.
- [ ] Fixture and read-only retail INI/CSF initialize source-owned data; real registered modules instantiate/release; missing/malformed data and unknown modules fail.
- [ ] GameState/Xfer/CRC startup relationships work; offline network is source-defined null, required symbols resolve and unsupported network entry fails clearly.
- [ ] Original UnicodeString/pool boundaries are tested in actual consumers; M19 helper probes alone cannot satisfy them.
- [ ] All-stage failure injection proves reverse exactly-once teardown; headless/verify acquire no window/GPU; arbitrary CWD and isolated XDG behavior pass.
- [ ] Removing a required original provider fails compile/link/runtime evidence; identity controls reject reduced alternate classes and witnesses without original state transitions.
- [ ] Negative controls fail against the actual implementation; no toy/placeholder fallback satisfies acceptance.
- [ ] Existing component tests pass; evidence names its grade: component/fixture, original-source compile/link, original-source integration or retail runtime.
- [ ] The same explicit offline profile passes all four presets through genuine updates and original reset, using original UI/parser/resource providers without a GPU/window. Normal gameplay defaults remain unchanged.
- [ ] Runtime witnesses compare original state before/after update and prove required provider execution; trace strings or separately incremented probe counters cannot establish success.
- [ ] Construction/init, INI load before registration, post-load, and update/reset failures each preserve the initiating error and prove exactly-once ownership, cleared globals, and stopped workers.
- [ ] New runtime tests retain active checks in every preset; evidence discloses existing NDEBUG settings and sanitizer leak-detection configuration, with explicit relevant allocator live-count and resource/worker ownership checks.

## Required Validation

Project-owned fixtures plus gated read-only retail initialization; absent/malformed config, missing factory/module, all-stage failure/reverse teardown, repeated reset; actual original string/pool boundary tests and sanitizers. Record compilation and linked provider identities separately from source-owned runtime state. Deliberately remove a required provider and assert failure at a real compile/link/runtime gate, not solely a filename policy. Assert no GPU/window acquisition in headless/verify modes, arbitrary CWD and isolated XDG roots. Offline entry rejects unsupported network operations. Full asset-free regressions and all four presets below are mandatory.

Build applicable targets with linux-gcc-debug, linux-clang-debug, linux-gcc-release and linux-clang-release. Run full asset-free CTest regression at milestone acceptance plus relevant ASan/UBSan. Record exact commands/context and map formerly unproven historical obligations to real-source tests. Retail/GPU checks use explicit separate gates; defaults remain private-data/device independent. Only permitted logical/derived evidence is committed.

Derive before/after state expectations from the selected source path (for example original GameLogic frame advancement together with an original message/client transition); do not create an active match solely to satisfy startup evidence. Verify target-specific compile commands and contributing objects/symbols, excluding discarded link-map sections and ambiguous basename matches; strengthen existing identity tooling and review consumed source conditional branches. Disclose assertions disabled by existing NDEBUG definitions and do not extend M19's unconditional workaround to the runtime. Leak-disabled sanitizers cannot establish leak freedom: record relevant allocator live counts and worker/resource ownership independently.

## Known Risks / Deferred Work

This is a substantial coupled source port; multiple coherent tested slices belong inside this one milestone transaction. Routine missing symbols, portability or classification repairs do not justify another architecture referral. Actual scene/mission/save/network acceptance remains with M21–M25 and M15–M18 without supplying any code retroactively needed here. Preserve rejected plan 01 and its blocker evidence; on adoption generate milestone_20_plan_02.md. Unexpected required asset/backend limits follow base-plan §9 evidence-gated escalation, never reduced silent coverage.

The governing plan must inventory actual providers and owned validation boundaries before implementation. Validate incremental support/data work without demanding a complete engine build as the first gate; milestone acceptance still requires the complete stated closure. Further reachable providers discovered during porting remain within this milestone's authorized scope.
