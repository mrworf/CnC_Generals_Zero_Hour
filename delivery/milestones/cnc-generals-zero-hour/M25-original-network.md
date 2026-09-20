# Milestone M25: original network lockstep integration

## Objective

Independent local processes use original network consumers and simulation for synchronized commands/CRC.

## User/System Outcome

Independent local processes use original network consumers and simulation for synchronized commands/CRC.

## Scope

Complete original packet/reliability/message integration with POSIX transport beyond M20's offline runtime/link closure. Audit enum/bool/coordinate/UTF-16 widths, bounds and timeouts; integrate discovery/direct-connect, lobby, map transfer, start synchronization and disconnect. M20 preserves source-defined offline TheNetwork = NULL and explicitly rejects unsupported network entry until this implementation exists.

Allocation rule: the [runtime reconciliation](../../../docs/zero-hour-runtime-closure-reconciliation.md) RC-001–RC-010 overrides historical allocation below. M26–M28 own independently accepted process/data/CPU providers; M20 integrates every remaining coupled dependency of its original offline initialization, post-load, update, reset and destruction. This contract adds later behavior and full domain acceptance, never a deferred half of that accepted runtime. Maintain the checked source dependency ledger, configuration/lifecycle edges, owner and freshness gates through this domain. Existing acceptance clauses and negative tests remain required in full.

## Explicit Exclusions

Complete-match user acceptance is M16 after M15. No second host, Windows client or external matchmaking required. Preserve valid component implementations/tests. No ARM64, base Generals executable/tools, retail redistribution or gameplay rewrite.

## Source Requirements

Runtime reconciliation RC-002/RC-003 preserves this domain's whole acceptance chain; RC-004 original-behavior identity and RC-007–RC-010 provider/ledger and later-path rules apply. Exact authority revisions are recorded in status.yaml.

[Migration supplement](../../../docs/zero-hour-source-engine-migration.md): SE-008; SE-010 assurance/evidence constraints. [Base port plan](../../../docs/zero-hour-linux-port-plan.md) §§1–6 and §10 govern preservation, stack, formats and validation. Revisions/commits are in status.yaml.

## Preconditions

Direct providers: M24, M11. Consume evidenced contracts, not historical title/status alone. Historical providers supply qualified component evidence; this contract provides the missing original-source assurance. Its unfinished deliverables are not entry prerequisites. Readiness separately audits external inputs.

- `PRE-033` — M24 original snapshots/CRC; PRE-014 — M11 qualified POSIX/virtual transport. PRE-008 at real-map validation; developer must permit two local loopback processes as in M11, no remote host. PRE-034 is produced here.

## Readiness checks

- Run `cmake --list-presets` from the repository root: all four native presets must resolve. Inspect status.yaml and linked provider acceptance for the direct dependencies; require the original-source grade for new providers, not only historical component status.
- Inspect [prerequisite manifest](../../readiness/cnc-generals-zero-hour/prerequisite-manifest.md) for the IDs above and their current verification. Use its retail/session checks only at the named validation boundary. No unfinished feature acceptance command is an entry requirement.

Produces `PRE-034` (M25 original network/lockstep); inspect this contract's source-owned positive and negative acceptance evidence before downstream consumption.

## Functional Requirements

Two isolated processes load actual maps, exchange original commands and compare original-state checkpoints. Source map transfer/negotiation and lobby work without online services; peer loss/mismatch report actionable diagnostics.

RC-010: extend the checked ledger through original network start/map transfer, lockstep, disconnect and return-to-menu. Network::setSawCRCMismatch consumes ScriptActions, Menus/CRCMismatch.wnd, timer, Recorder and random-state CRC; these CPU resources and callbacks must work here using earlier providers, never depend on a future hardware/UI implementation. Preserve real mismatch handling with bounded headless diagnostics and cleanup, not a transport-only error substitute.

## Architecture / Security Constraints

Original engine owns authoritative state/lifecycle; native adapters own OS/device/library edges. Preserve original allocators except narrow characterized portability changes. Retail roots/symlinks are read-only; never commit retail bytes/hashes/private paths. Isolate XDG writes. Never suppress unresolved symbols or use fake success-returning production subsystems.

## Interfaces and Compatibility

Preserve original engine interfaces and fixed-width, char16_t/UTF-16LE, bounded serialization and floating-point rules; never -fshort-wchar. Incremental source targets coexist with fixture-only scaffolding, never parallel authoritative states. Rollback uses reviewed commits without overwriting user saves/configuration or retail. Keep valid native component APIs unless source integration proves a narrow correction necessary.

## Acceptance Criteria

- [ ] The stated outcome and all scoped consumers execute original implementations with recorded source-owned transitions or outputs.
- [ ] Two isolated processes load actual maps, exchange original commands and compare original-state checkpoints. Source map transfer/negotiation and lobby work without online services; peer loss/mismatch report actionable diagnostics.
- [ ] Negative controls fail against the actual implementation; no toy/placeholder fallback satisfies acceptance.
- [ ] Existing component tests pass; evidence names its grade: component/fixture, original-source compile/link, original-source integration or retail runtime.

## Required Validation

Owned malformed packet and virtual loss/reorder/duplicate tests plus real independent local processes on actual maps; version/data/map mismatch, peer exit/desync. If broadcast unavailable require virtual discovery plus real direct-connect.

Trigger the original CRC mismatch path and verify required script/UI/timer/recorder/RNG behavior, noninteractive failure and no destroyed-global callback use. Repeat return/reset/re-entry after transfer/start/disconnect failures. Independent client/audio random activity must not perturb synchronized simulation checkpoints. Ledger freshness and ownerless-edge controls remain required.

Build applicable targets with linux-gcc-debug, linux-clang-debug, linux-gcc-release and linux-clang-release. Run full asset-free CTest regression at milestone acceptance plus relevant ASan/UBSan. Record exact commands/context and map formerly unproven historical obligations to real-source tests. Retail/GPU checks use explicit separate gates; defaults remain private-data/device independent. Only permitted logical/derived evidence is committed.

## Known Risks / Deferred Work

Complete-match user acceptance is M16 after M15. No second host, Windows client or external matchmaking required. Component tests do not establish original execution. Unexpected required asset/backend limits follow base-plan §9 evidence-gated escalation, never reduced silent coverage.
