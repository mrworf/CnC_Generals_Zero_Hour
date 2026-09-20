# Milestone M21: original map and simulation execution

## Objective

Original GameLogic loads maps and advances actual mission/skirmish state, commands, AI and scripts.

## User/System Outcome

Original GameLogic loads maps and advances actual mission/skirmish state, commands, AI and scripts.

## Scope

Complete original map/chunk loading, terrain logic, object modules, scripting, AI/pathfinding, player/side setup, command/message routing and victory/defeat beyond the runtime already accepted in M20. Preserve update order and source ownership; prove complete mission/skirmish behavior. Existing original GameLogic, registries and all startup/update dependencies come from M20.

Allocation rule: the [runtime reconciliation](../../../docs/zero-hour-runtime-closure-reconciliation.md) RC-001–RC-010 overrides historical allocation below. M26–M28 own independently accepted process/data/CPU providers; M20 integrates every remaining coupled dependency of its original offline initialization, post-load, update, reset and destruction. This contract adds later behavior and full domain acceptance, never a deferred half of that accepted runtime. Maintain the checked source dependency ledger, configuration/lifecycle edges, owner and freshness gates through this domain. Existing acceptance clauses and negative tests remain required in full.

## Explicit Exclusions

Rendered sessions follow M22/M23/M15; full save/replay cross-compiler checks are M24. Preserve valid component implementations/tests. No ARM64, base Generals executable/tools, retail redistribution or gameplay rewrite.

## Source Requirements

Runtime reconciliation RC-002/RC-003 preserves this domain's whole acceptance chain; RC-004 original-behavior identity and RC-007–RC-010 provider/ledger and later-path rules apply. Exact authority revisions are recorded in status.yaml.

[Migration supplement](../../../docs/zero-hour-source-engine-migration.md): SE-004; SE-010 assurance/evidence constraints. [Base port plan](../../../docs/zero-hour-linux-port-plan.md) §§1–6 and §10 govern preservation, stack, formats and validation. Revisions/commits are in status.yaml.

## Preconditions

Direct providers: M20. Consume evidenced contracts, not historical title/status alone. Historical providers supply qualified component evidence; this contract provides the missing original-source assurance. Its unfinished deliverables are not entry prerequisites. Readiness separately audits external inputs.

- `PRE-029` — M20 original initialization acceptance; PRE-008 — readable owned maps at retail validation. PRE-030 is produced here.

## Readiness checks

- Run `cmake --list-presets` from the repository root: all four native presets must resolve. Inspect status.yaml and linked provider acceptance for the direct dependencies; require the original-source grade for new providers, not only historical component status.
- Inspect [prerequisite manifest](../../readiness/cnc-generals-zero-hour/prerequisite-manifest.md) for the IDs above and their current verification. Use its retail/session checks only at the named validation boundary. No unfinished feature acceptance command is an entry requirement.

Produces `PRE-030` (M21 original maps/simulation); inspect this contract's source-owned positive and negative acceptance evidence before downstream consumption.

## Functional Requirements

Run scripted mission and skirmish setup through original parsers. Witness object creation, movement/attack effects, scripts, AI and victory/defeat with original-state checkpoints. Labels, category manifests or toy entities cannot satisfy acceptance.

RC-010: extend the checked source dependency ledger through GameLogic::startNewGame, map INI/terrain, TerrainVisual::addProp, GameClient::preloadAssets and Recorder::initControls. Consume earlier CPU/data providers and implement additional map-start consumer behavior here, never defer it to M22/M23. Exercise supported initial .map dispatch and return/reset/re-entry with real nonempty original map/object/script state. Required providers without owners or stale source/registry evidence fail validation; no physical device is needed for headless scenario acceptance.

## Architecture / Security Constraints

Original engine owns authoritative state/lifecycle; native adapters own OS/device/library edges. Preserve original allocators except narrow characterized portability changes. Retail roots/symlinks are read-only; never commit retail bytes/hashes/private paths. Isolate XDG writes. Never suppress unresolved symbols or use fake success-returning production subsystems.

## Interfaces and Compatibility

Preserve original engine interfaces and fixed-width, char16_t/UTF-16LE, bounded serialization and floating-point rules; never -fshort-wchar. Incremental source targets coexist with fixture-only scaffolding, never parallel authoritative states. Rollback uses reviewed commits without overwriting user saves/configuration or retail. Keep valid native component APIs unless source integration proves a narrow correction necessary.

## Acceptance Criteria

- [ ] The stated outcome and all scoped consumers execute original implementations with recorded source-owned transitions or outputs.
- [ ] Run scripted mission and skirmish setup through original parsers. Witness object creation, movement/attack effects, scripts, AI and victory/defeat with original-state checkpoints. Labels, category manifests or toy entities cannot satisfy acceptance.
- [ ] Negative controls fail against the actual implementation; no toy/placeholder fallback satisfies acceptance.
- [ ] Existing component tests pass; evidence names its grade: component/fixture, original-source compile/link, original-source integration or retail runtime.

## Required Validation

Actual corpus maps plus owned parser/command fixtures; missing map/module, malformed chunks and invalid commands without partial-state success; repeated checkpoints and sanitizers. Disabling original GameLogic/map loader must fail runtime acceptance.

Prove map-start CPU resource preloading, prop creation and recorder-control initialization through original consumers; reset/re-entry releases resources/callbacks without destroyed-global access. Vary independent client/audio random activity while comparing simulation checkpoints. Validate initial .map dispatch and error paths without bypassing source initialization.

Build applicable targets with linux-gcc-debug, linux-clang-debug, linux-gcc-release and linux-clang-release. Run full asset-free CTest regression at milestone acceptance plus relevant ASan/UBSan. Record exact commands/context and map formerly unproven historical obligations to real-source tests. Retail/GPU checks use explicit separate gates; defaults remain private-data/device independent. Only permitted logical/derived evidence is committed.

## Known Risks / Deferred Work

Rendered sessions follow M22/M23/M15; full save/replay cross-compiler checks are M24. Component tests do not establish original execution. Unexpected required asset/backend limits follow base-plan §9 evidence-gated escalation, never reduced silent coverage.
