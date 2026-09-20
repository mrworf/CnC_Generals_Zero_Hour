# Milestone M23: original menus controls and media integration

## Objective

Original menus/HUD and input/audio/video managers operate together and route actual game commands.

## User/System Outcome

Original menus/HUD and input/audio/video managers operate together and route actual game commands.

## Scope

Complete SDL integration with original message translators, WND/gadget callbacks, command queues and camera; render supplied English fonts. Complete original audio/video integration with miniaudio/FFmpeg, voice categories, positional audio, ownership, cinematic completion/skip and source subtitles. M20 already owns all these interfaces/providers required by offline startup/update; this milestone finishes interactive behavior and acceptance.

Allocation rule: the [runtime reconciliation](../../../docs/zero-hour-runtime-closure-reconciliation.md) RC-001–RC-004 overrides any historical allocation below. M20 owns every transitive implementation dependency of its accepted original offline initialization, post-load, update, reset and destruction. This contract may add later feature behavior and full domain acceptance, but cannot supply a deferred half of M20's runtime. Existing acceptance clauses and negative tests remain required in full.

## Explicit Exclusions

Persistence assurance is M24; long-session acceptance M15. Preserve original UX; no extra locale shaping without the existing evidence gate. Preserve valid component implementations/tests. No ARM64, base Generals executable/tools, retail redistribution or gameplay rewrite.

## Source Requirements

Runtime reconciliation RC-002/RC-003 preserves this domain's whole acceptance chain; RC-004 original-behavior identity rules apply. Exact authority revisions are recorded in status.yaml.

[Migration supplement](../../../docs/zero-hour-source-engine-migration.md): SE-006; SE-010 assurance/evidence constraints. [Base port plan](../../../docs/zero-hour-linux-port-plan.md) §§1–6 and §10 govern preservation, stack, formats and validation. Revisions/commits are in status.yaml.

## Preconditions

Direct providers: M22, M6, M8, M12, M13. Consume evidenced contracts, not historical title/status alone. Historical providers supply qualified component evidence; this contract provides the missing original-source assurance. Its unfinished deliverables are not entry prerequisites. Readiness separately audits external inputs.

- `PRE-031` — M22 original scene/device acceptance; PRE-019/PRE-020/PRE-011/PRE-023/PRE-024 — qualified M6/M8/M12/M13 platform/font/media components; PRE-008/PRE-016 — retail and developer graphical session at interactive validation. PRE-032 is produced here.

## Readiness checks

- Run `cmake --list-presets` from the repository root: all four native presets must resolve. Inspect status.yaml and linked provider acceptance for the direct dependencies; require the original-source grade for new providers, not only historical component status.
- Inspect [prerequisite manifest](../../readiness/cnc-generals-zero-hour/prerequisite-manifest.md) for the IDs above and their current verification. Use its retail/session checks only at the named validation boundary. No unfinished feature acceptance command is an entry requirement.

Produces `PRE-032` (M23 original UI/input/media); inspect this contract's source-owned positive and negative acceptance evidence before downstream consumption.

## Functional Requirements

Navigate original menu, mission/skirmish selection, loading, HUD commands, pause/options and return-to-menu. Prove input changes original state and media events originate from original managers; isolated XDG writes and silent audio remain supported.

## UX Constraints

Preserve original English retail presentation/interaction from base plan §6, including keyboard/mouse bindings, source navigation, loading/error/recovery, camera/viewport and focus behavior. Review legibility and resize behavior; missing assets produce actionable errors, not placeholders. No redesign or invented accessibility claim; retain source subtitles where provided.

## Architecture / Security Constraints

Original engine owns authoritative state/lifecycle; native adapters own OS/device/library edges. Preserve original allocators except narrow characterized portability changes. Retail roots/symlinks are read-only; never commit retail bytes/hashes/private paths. Isolate XDG writes. Never suppress unresolved symbols or use fake success-returning production subsystems.

## Interfaces and Compatibility

Preserve original engine interfaces and fixed-width, char16_t/UTF-16LE, bounded serialization and floating-point rules; never -fshort-wchar. Incremental source targets coexist with fixture-only scaffolding, never parallel authoritative states. Rollback uses reviewed commits without overwriting user saves/configuration or retail. Keep valid native component APIs unless source integration proves a narrow correction necessary.

## Acceptance Criteria

- [ ] The stated outcome and all scoped consumers execute original implementations with recorded source-owned transitions or outputs.
- [ ] Navigate original menu, mission/skirmish selection, loading, HUD commands, pause/options and return-to-menu. Prove input changes original state and media events originate from original managers; isolated XDG writes and silent audio remain supported.
- [ ] Negative controls fail against the actual implementation; no toy/placeholder fallback satisfies acceptance.
- [ ] Existing component tests pass; evidence names its grade: component/fixture, original-source compile/link, original-source integration or retail runtime.

## Required Validation

Interactive actual menu/HUD/camera plus focus/resize/input failures; missing/corrupt media, skip/pause/EOS, English/source subtitle rendering and font lifetime; audio callback/shutdown regressions. Disabling original managers fails integration.

Build applicable targets with linux-gcc-debug, linux-clang-debug, linux-gcc-release and linux-clang-release. Run full asset-free CTest regression at milestone acceptance plus relevant ASan/UBSan. Record exact commands/context and map formerly unproven historical obligations to real-source tests. Retail/GPU checks use explicit separate gates; defaults remain private-data/device independent. Only permitted logical/derived evidence is committed.

## Known Risks / Deferred Work

Persistence assurance is M24; long-session acceptance M15. Preserve original UX; no extra locale shaping without the existing evidence gate. Component tests do not establish original execution. Unexpected required asset/backend limits follow base-plan §9 evidence-gated escalation, never reduced silent coverage.
