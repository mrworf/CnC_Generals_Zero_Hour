# Milestone M24: original saves replay and deterministic state

## Objective

Linux saves/replays restore original objects/scripts/AI/player state and reproduce checkpoints across compilers.

## User/System Outcome

Linux saves/replays restore original objects/scripts/AI/player state and reproduce checkpoints across compilers.

## Scope

Complete original Xfer traversal, SaveGame, recorder/playback and CRC for full real-game persistence and replay with bounded fixed-width source-established layouts. Preserve autosave/transactional load and isolate toy ZHSG as fixture-only. Original recorder, GameState/GameStateMap and Xfer/CRC implementations needed for startup/post-load/update already belong to M20.

Allocation rule: the [runtime reconciliation](../../../docs/zero-hour-runtime-closure-reconciliation.md) RC-001–RC-004 overrides any historical allocation below. M20 owns every transitive implementation dependency of its accepted original offline initialization, post-load, update, reset and destruction. This contract may add later feature behavior and full domain acceptance, but cannot supply a deferred half of M20's runtime. Existing acceptance clauses and negative tests remain required in full.

## Explicit Exclusions

Windows fixture compatibility remains optional M18; no compatibility promise for synthetic ZHSG. Preserve valid component implementations/tests. No ARM64, base Generals executable/tools, retail redistribution or gameplay rewrite.

## Source Requirements

Runtime reconciliation RC-002/RC-003 preserves this domain's whole acceptance chain; RC-004 original-behavior identity rules apply. Exact authority revisions are recorded in status.yaml.

[Migration supplement](../../../docs/zero-hour-source-engine-migration.md): SE-007; SE-010 assurance/evidence constraints. [Base port plan](../../../docs/zero-hour-linux-port-plan.md) §§1–6 and §10 govern preservation, stack, formats and validation. Revisions/commits are in status.yaml.

## Preconditions

Direct providers: M21, M5. Consume evidenced contracts, not historical title/status alone. Historical providers supply qualified component evidence; this contract provides the missing original-source assurance. Its unfinished deliverables are not entry prerequisites. Readiness separately audits external inputs.

- `PRE-030` — M21 original simulation; PRE-018 — qualified M5 codec/fixture component, not game snapshots; PRE-008 at real-map validation. PRE-033 is produced here; Windows PRE-015 is not required.

## Readiness checks

- Run `cmake --list-presets` from the repository root: all four native presets must resolve. Inspect status.yaml and linked provider acceptance for the direct dependencies; require the original-source grade for new providers, not only historical component status.
- Inspect [prerequisite manifest](../../readiness/cnc-generals-zero-hour/prerequisite-manifest.md) for the IDs above and their current verification. Use its retail/session checks only at the named validation boundary. No unfinished feature acceptance command is an entry requirement.

Produces `PRE-033` (M24 original Xfer/replay/CRC); inspect this contract's source-owned positive and negative acceptance evidence before downstream consumption.

## Functional Requirements

Round-trip a real source-engine map with objects, script/AI/player state and commands; resume and compare original checkpoints. Replay actual source commands across GCC/Clang Debug/Release rather than a parallel synthetic snapshot.

## Architecture / Security Constraints

Original engine owns authoritative state/lifecycle; native adapters own OS/device/library edges. Preserve original allocators except narrow characterized portability changes. Retail roots/symlinks are read-only; never commit retail bytes/hashes/private paths. Isolate XDG writes. Never suppress unresolved symbols or use fake success-returning production subsystems.

## Interfaces and Compatibility

Preserve original engine interfaces and fixed-width, char16_t/UTF-16LE, bounded serialization and floating-point rules; never -fshort-wchar. Incremental source targets coexist with fixture-only scaffolding, never parallel authoritative states. Rollback uses reviewed commits without overwriting user saves/configuration or retail. Keep valid native component APIs unless source integration proves a narrow correction necessary.

## Acceptance Criteria

- [ ] The stated outcome and all scoped consumers execute original implementations with recorded source-owned transitions or outputs.
- [ ] Round-trip a real source-engine map with objects, script/AI/player state and commands; resume and compare original checkpoints. Replay actual source commands across GCC/Clang Debug/Release rather than a parallel synthetic snapshot.
- [ ] Negative controls fail against the actual implementation; no toy/placeholder fallback satisfies acceptance.
- [ ] Existing component tests pass; evidence names its grade: component/fixture, original-source compile/link, original-source integration or retail runtime.

## Required Validation

Round-trip/autosave/record-playback; truncated/unknown-version/corrupt saves, invalid references, replay mismatch and unchanged live state on failed load; cross-preset original checkpoints and sanitizer tests. Corrupt real save/replay state to prove rejection.

Build applicable targets with linux-gcc-debug, linux-clang-debug, linux-gcc-release and linux-clang-release. Run full asset-free CTest regression at milestone acceptance plus relevant ASan/UBSan. Record exact commands/context and map formerly unproven historical obligations to real-source tests. Retail/GPU checks use explicit separate gates; defaults remain private-data/device independent. Only permitted logical/derived evidence is committed.

## Known Risks / Deferred Work

Windows fixture compatibility remains optional M18; no compatibility promise for synthetic ZHSG. Component tests do not establish original execution. Unexpected required asset/backend limits follow base-plan §9 evidence-gated escalation, never reduced silent coverage.
