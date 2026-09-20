# Milestone M28: original CPU presentation and media resources

## Objective

Make original presentation and media resource consumers usable without acquiring graphics/audio hardware.

## User/System Outcome

Original scenes, loaders, UI/font/layout callbacks and audio definitions operate through recording/null device adapters with observable resource ownership and failures.

## Scope

Separate W3DDisplay's original CPU scene/light/asset-manager/loader work from WW3D device selection, shaders and excluded browser integration using narrow shared-method refactoring. Adapt independently testable original W3D/GameClient resource, font/image/WND and media interfaces to existing native components. Audit DX8Wrapper initialization/destruction capabilities as well as drawing. Port original GameAudio definition parsing/lookup and bounded missing-music behavior. Full GameClient/registry orchestration remains M20.

## Explicit Exclusions

No hardware scene/pixel acceptance (M22), interactive/menu/cinematic flow acceptance (M23), full simulation (M21) or complete original runtime acceptance (M20). Do not null the whole display/GameClient, replace original resource classes or require private assets/device acquisition to accept owned fixture consumers.

## Source Requirements

[Runtime reconciliation](../../../docs/zero-hour-runtime-closure-reconciliation.md) RC-002 presentation/media edges, RC-004/006 resource/identity rules, RC-007 independent providers, RC-009 audio, RC-010 CPU/device split and later consumers; SE-005/006/010 and base plan §§3/6/10. Provenance is in status.yaml.

## Preconditions

M27 original data providers (including M26 process/ABI); M6/M7/M8 native platform/recording/fonts and M12/M13 audio/video components. These are consumed contracts, not full-engine proof. Physical devices, retail initialization and later registry linkage are not entry or independent-provider acceptance requirements.

- `PRE-036` — accepted M27 independent original data consumers; `PRE-019/PRE-010/PRE-020/PRE-011/PRE-023/PRE-024` — retained native platform, recording, font and media components. This milestone produces `PRE-037`; full runtime PRE-029 remains M20-owned.

## Readiness checks

- Run `cmake --list-presets`; inspect status.yaml and linked M27/M6/M7/M8/M12/M13 acceptance for the listed PRE IDs and the [prerequisite manifest](../../readiness/cnc-generals-zero-hour/prerequisite-manifest.md). Do not treat historical native components as original-provider acceptance.
- Inspect source/ledger ownership of W3DDisplay CPU scene/loaders, original font/image/WND callbacks and GameAudio music lookup; earlier data providers support bounded consumer fixtures, and full GameClient/registry startup stays M20-owned. No window/GPU/audio device or full integrated engine build is an entry requirement.

## Functional Requirements

Keep actual CPU scene/resource ownership, loaders and font/image/WND parser callbacks when device acquisition is disabled. Enumerate logical resources consumed by fixture targets, including the reset BlankWindow layout where independent. Required unsupported resource/adapter operations fail visibly; remove excluded browser/service paths. Device-boundary recorders must observe real original producer work.

Run original audio definition parsing and music lookup on owned valid resources. Replace excluded CD search/modal prompt acquisition with bounded noninteractive missing-resource failure for headless operation and preserve meaningful interactive reporting. Never force isMusicAlreadyLoaded true, reset quitting, or use fixture fallback in production. Release workers/resources before owners and clear callbacks/globals safely.

Extend the checked source ledger with CPU/device lifecycle and callback/vtable/global dependencies, logical assets and exact later consumers. Full map start's TerrainVisual::addProp/GameClient::preloadAssets, reset/UI and CRC error layouts must map to provider interfaces here plus integrated/feature work at M20/M21/M25; no startup dependency may first appear at M22/M23.

## Architecture / Security Constraints

Original classes/state and semantics remain authoritative. Narrow shared-method extraction and explicit device adapters are permitted, reduced alternate implementations and empty registries are not. Linux x86-64 only; retail read-only; XDG writes; existing SDL_GPU/miniaudio/FFmpeg/font components preserved. Ledger freshness, live target identity and active four-preset tests follow M26.

## Interfaces and Compatibility

Keep original scene/resource/media interfaces and normal gameplay defaults. Independently executable consumer targets establish CPU/provider behavior without pretending whole GameClient/registries link. Original resources missing at runtime fail with actionable diagnostics; no new product UX is introduced.

## Acceptance Criteria

- [ ] Actual original CPU scene/loaders/font/image/WND and audio-definition consumers execute owned fixtures in all four presets without a device or complete engine initialization.
- [ ] Source-owned resources and registered loaders/callbacks are observed; missing resource/unsupported operation and malformed definitions fail visibly.
- [ ] Valid owned music succeeds; missing music terminates promptly with preserved diagnostic and no CD search/modal wait/fabricated success.
- [ ] Repeated creation/use/destruction and partial failures release callbacks/resources/workers exactly once with live counts restored.
- [ ] Headless consumers acquire no window/GPU/audio hardware; removing required original providers fails real identity/runtime gates.
- [ ] Ledger names all remaining coupled M20 and later domain owners, including initialization/destruction and DX8Wrapper capabilities; asset-free regressions remain green.

## Required Validation

All four GCC/Clang Debug/Release builds of actual source consumer targets, owned positive/malformed/missing resource fixtures, no-device interception, callback/loader identity, repeated release and failure injection. Full asset-free CTest plus focused ASan/UBSan with exact configuration and explicit allocator/resource/worker counts. Distinguish source inspection, compile/live-link evidence and executed state; source drift invalidates affected ledger checks. Retail and hardware checks remain separate later gates.

## Known Risks / Deferred Work

W3D constructors and vtables may expose more tightly coupled closure than directory names imply. Use real independent source seams; remaining complete GameClient/display/registry lifecycle is M20's responsibility. M21 owns additional map-start CPU behavior; M22/M23 complete hardware/interactive functionality but never retroactively provide required startup code.
