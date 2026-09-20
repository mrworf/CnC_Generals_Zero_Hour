# Milestone M22: original retail scene rendering on Vulkan

## Objective

Original W3D/WWShade/GameClient producers render actual retail scenes on the SDL_GPU Vulkan device.

## User/System Outcome

Original W3D/WWShade/GameClient producers render actual retail scenes on the SDL_GPU Vulkan device.

## Scope

Port actual DX8Wrapper consumers and material/state/resource semantics. Load W3D/HLOD/animation/textures and map terrain; integrate original camera, terrain, object, lighting, fog/shroud, shadow, particle, water and effect producers.

## Explicit Exclusions

UI/media flows remain M23; complete sessions M15. Required SDL_GPU gaps follow existing backend escalation, never private API escape hatches. Preserve valid component implementations/tests. No ARM64, base Generals executable/tools, retail redistribution or gameplay rewrite.

## Source Requirements

[Migration supplement](../../../docs/zero-hour-source-engine-migration.md): SE-005; SE-010 assurance/evidence constraints. [Base port plan](../../../docs/zero-hour-linux-port-plan.md) §§1–6 and §10 govern preservation, stack, formats and validation. Revisions/commits are in status.yaml.

## Preconditions

Direct providers: M21, M14. Consume evidenced contracts, not historical title/status alone. Historical providers supply qualified component evidence; this contract provides the missing original-source assurance. Its unfinished deliverables are not entry prerequisites. Readiness separately audits external inputs.

- `PRE-030` — M21 original simulation; PRE-013 — M14 component GPU backend only. PRE-008/PRE-012/PRE-016 — retail, developer Vulkan/layers and graphical session required at real-scene validation, not asset-free implementation. PRE-031 is produced here.

## Readiness checks

- Run `cmake --list-presets` from the repository root: all four native presets must resolve. Inspect status.yaml and linked provider acceptance for the direct dependencies; require the original-source grade for new providers, not only historical component status.
- Inspect [prerequisite manifest](../../readiness/cnc-generals-zero-hour/prerequisite-manifest.md) for the IDs above and their current verification. Use its retail/session checks only at the named validation boundary. No unfinished feature acceptance command is an entry requirement.

Produces `PRE-031` (M22 original producers and real-scene Vulkan); inspect this contract's source-owned positive and negative acceptance evidence before downstream consumption.

## Functional Requirements

Recording tests witness actual producers and reject unsupported required states/assets. Execute the same integrated scenes on RTX Vulkan with zero validation errors, captured visual review and bounded resize/resource recreation.

## UX Constraints

Preserve original English retail presentation/interaction from base plan §6, including keyboard/mouse bindings, source navigation, loading/error/recovery, camera/viewport and focus behavior. Review legibility and resize behavior; missing assets produce actionable errors, not placeholders. No redesign or invented accessibility claim; retain source subtitles where provided.

## Architecture / Security Constraints

Original engine owns authoritative state/lifecycle; native adapters own OS/device/library edges. Preserve original allocators except narrow characterized portability changes. Retail roots/symlinks are read-only; never commit retail bytes/hashes/private paths. Isolate XDG writes. Never suppress unresolved symbols or use fake success-returning production subsystems.

## Interfaces and Compatibility

Preserve original engine interfaces and fixed-width, char16_t/UTF-16LE, bounded serialization and floating-point rules; never -fshort-wchar. Incremental source targets coexist with fixture-only scaffolding, never parallel authoritative states. Rollback uses reviewed commits without overwriting user saves/configuration or retail. Keep valid native component APIs unless source integration proves a narrow correction necessary.

## Acceptance Criteria

- [ ] The stated outcome and all scoped consumers execute original implementations with recorded source-owned transitions or outputs.
- [ ] Recording tests witness actual producers and reject unsupported required states/assets. Execute the same integrated scenes on RTX Vulkan with zero validation errors, captured visual review and bounded resize/resource recreation.
- [ ] Negative controls fail against the actual implementation; no toy/placeholder fallback satisfies acceptance.
- [ ] Existing component tests pass; evidence names its grade: component/fixture, original-source compile/link, original-source integration or retail runtime.

## Required Validation

Real producer command witnesses; missing/malformed asset and unsupported material failures; explicit Khronos layer with Validation Error/VUID failure; review every required real scene family, resize/recreation and retained synthetic tests. Synthetic shader scenes/TSV families are not acceptance.

Build applicable targets with linux-gcc-debug, linux-clang-debug, linux-gcc-release and linux-clang-release. Run full asset-free CTest regression at milestone acceptance plus relevant ASan/UBSan. Record exact commands/context and map formerly unproven historical obligations to real-source tests. Retail/GPU checks use explicit separate gates; defaults remain private-data/device independent. Only permitted logical/derived evidence is committed.

## Known Risks / Deferred Work

UI/media flows remain M23; complete sessions M15. Required SDL_GPU gaps follow existing backend escalation, never private API escape hatches. Component tests do not establish original execution. Unexpected required asset/backend limits follow base-plan §9 evidence-gated escalation, never reduced silent coverage.
