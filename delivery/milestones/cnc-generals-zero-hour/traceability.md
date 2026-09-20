# Source traceability

Authority is the accepted [runtime reconciliation](../../../docs/zero-hour-runtime-closure-reconciliation.md), superseding conflicting ordering/allocation in the [migration supplement](../../../docs/zero-hour-source-engine-migration.md), with the [base plan](../../../docs/zero-hour-linux-port-plan.md) as governing behavior/stack authority; exact revisions and commits are in status.yaml. Baseline includes existing native components and M19's bounded original support evidence. The architecture reviews and M20 blocker are evidence only. Historical mappings preserve initial intent, not implementation assurance.

## Runtime reconciliation coverage and moved clauses

RC-007–RC-010 refine the older integrated-owner rows below: where they say M20 implementation, independent process/ABI goes to M26, independent parser/codec/cache-path work to M27, and independent CPU/media resources to M28; M20 retains every coupled provider and full integration/negative acceptance. This allocation supersedes the former single-milestone preparation requirement without dropping a test.

| New requirement / moved clause | Provider and final acceptance | Boundary and preserved evidence |
|---|---|---|
| RC-007; RC-006 incremental preparation | M26 → M27 → M28 → M20 | Three executable original-consumer outcomes replace the all-in-M20 prerequisite grouping. No full engine/private data/device required for earlier independent tests; complete registry/lifecycle acceptance remains M20. |
| RC-007 ledger/freshness | M26 establishes; M27/M28/M20–M25 extend | Source/symbol, direct consumer, lifecycle/configuration, provider/target, callback/vtable/global, assets/writes and evidence grade; source/registry changes invalidate checks, required ownerless edges fail. Not a static completeness promise or inventory-only milestone. |
| RC-008 process/allocator; prior RC-002 ABI and RC-004 support | M26 independent; M20 integrated lifetime | Original bootstrap/logging/sync/Version, GameMemory real linkage, pre-main/static lifetime, overloads/alignment/cross-target free and original UTF-16; M19 completed evidence unchanged. Single extracted original setFPMode avoids GameLogic dependency; original INI/map callers validated in M27. |
| RC-008 source data/RNG; prior RC-002 data and RC-006 fixtures | M27 independent; M20 production dispatch | Original Xfer widths/UTF-16, six-word RNG CRC/independent streams, actual original VFS/INI precedence and two-pass directory ordering. Extract real parser methods with selected real fixture callbacks; full static INI table and registries remain M20, no fake callbacks or gc workaround. |
| RC-009 map metadata | M27 parser/cache-path primitives; M20 full cache behavior | ThingFactory/MapObject classification, GameText/map.str and INIMapCache name keys need coupled M20 providers. Nonempty maps, cold/warm/absent/stale caches, malformed data, Debug/Release and write-denial/read-only/XDG tests preserved at full integration; empty fixtures prove no full-cache claim. |
| RC-009 audio | M28 independent resource parsing; M20 init failure propagation; M23 interactive | Original GameAudio event/music lookup, owned valid/missing fixtures and bounded CD/modal removal; no forced true/reset-quitting/production fixture fallback. Full error preserved and nonzero before execute in M20. |
| RC-009 remaining startup side effects; RC-006 profile | M20 | Release fingerprint/protection removal without losing real checks; non-D3D catches/early failure, MapCache/GameLOD options/capability/benchmark writes; no-map/replay/cache-build/forced-benchmark test profile; normal defaults preserved and supported dispatch accepted M21/M24. |
| RC-010 CPU/device split; prior RC-002 presentation and RC-006 resources | M28 independent; M20 complete GameClient/display lifecycle | Real scene/light/asset-manager/loaders/font/image/WND callbacks and DX8Wrapper init/destruction capabilities; narrow shared methods and no whole-display nulling; ownership/unsupported resource failures. Real Vulkan and interactive acceptance retained M22/M23. |
| RC-010 map start/reset | M21 | Actual map INI/terrain, TerrainVisual::addProp, GameClient::preloadAssets, Recorder::initControls and initial .map dispatch; added CPU consumer behavior owned now, not M22/M23. Reset/re-entry and independent client/audio RNG checkpoint tests. |
| RC-010 save/load/recorder | M24 | GameState reset-before-load/postprocess rollback preserves pre-load state; Recorder mode/header failure rollback, initial .rep/MSG_NEW_GAME/InitRandom, cross-preset DEBUG_LOGGING compatibility and command/RNG/CRC. Headless UI errors bounded; all prior corruption/reference/sanitizer tests retained. |
| RC-010 network/return-to-menu | M25 | Map transfer/start/reset paths and actual setSawCRCMismatch ScriptActions/CRCMismatch.wnd/timer/Recorder/RNG; earlier CPU providers suffice, no later hardware dependency. Existing faulted local peers and full M16 match retained. |
| RC-004/006 identity/assertions/ownership | M26–M28 independent; M20 final; M21–M25 domains | Four presets with active tests, live contributing target identity and provider-removal failures; no extending NDEBUG workaround; exact sanitizer options and explicit allocation/resource/worker counts. All integrated fixture/retail/reset/failure checks remain M20. |
| RC-003/005/012 history/adoption | status/index; M15–M18 acceptance | Preserve all completed records/contracts including M26–M28, blocked plan 01 and plan 02 completed slices 01–05/blocker. Adopt RC-012 and create M20 plan 03 only during delivery. No acceptance clause from prior packet is removed. |
| RC-011 optional pool bootstrap | M26 | Preserve original compiled defaults and optional MemoryPools.ini semantics using bounded low-level allocation-safe operations before data-root/VFS/INI/engine-string/logging availability; characterize timing/precedence, no retuning live pools. Owned absent/valid/malformed/oversized/invalid-count fixtures, arbitrary CWD, pre-main and allocation re-entry tests avoid an M27 dependency. |
| RC-011 final release evidence | M17 consumes M22/M23 through M15, plus M16 | Original-engine hardware/interaction and integrated gameplay/LAN evidence replace stale M14-only release wording; historical component results remain supporting evidence. Actual installed-prefix original-executable launch outside source/build trees resolves installed shaders/resources with explicit retail roots and isolated XDG; missing-resource negative case prevents build-tree fallback. No new format or changed external-gate timing. |

| Required source | Current owner | Preserved/moved obligation |
|---|---|---|
| RC-001 | M20 | Complete original offline runtime implementation closure; required constructors, vtables, registry callbacks, parsers, update/reset/destruction providers cannot depend on M21–M25. Classification follows source dependencies. |
| RC-002 original entry/factory and ABI rows | M20 | Original GameMain/GameEngine methods; remaining original UnicodeString/MemoryPool/PCH/CRT callers. M19 ten-provider acceptance preserved without extending its assurance. |
| RC-002 data and registry rows | M20 | Original VFS/INI/CSF/GlobalData/name keys, object/module/function registries, weapons/locomotors/particles: every required registered provider, actual configuration consumption, ownership and failure behavior. |
| RC-002 GameClient/GameLogic rows; moved M21 Scope | M20 implementation; M21 full scenarios | Original GameLogic/AI/team/player/crate/radar/victory and every startup/update dependency move into M20. M21 retains full maps, movement/attack, scripts, AI, victory/defeat, malformed map/commands and original-state checkpoints. |
| RC-002 presentation rows; moved M22 Scope | M20 reachable providers; M22 complete scenes | Original GameClient/W3D/WWShade producers, CPU resources and renderer interface adapters required by offline closure belong to M20 with device-boundary recording/null tests. M22 retains every real scene family, resource/state completion, malformed assets/material failures and zero-error Vulkan visuals/recreation. |
| RC-002 media/update rows; moved M23 Scope | M20 reachable providers; M23 interactive flows | Original UI/font/media manager interfaces and required audio/message/script/view updates belong to M20. M23 retains menu/HUD/input state transitions, cinematic/audio/subtitle behavior, fonts and focus/error recovery. |
| RC-002 recorder/game-state rows; moved M24 Scope | M20 startup/post-load/required update; M24 full persistence | Original recorder, GameState/GameStateMap, Xfer/CRC relationships required at runtime belong to M20. M24 retains full round-trip/autosave/playback, corrupt/reference rejection with unchanged state, cross-compiler checkpoints and sanitizers. |
| RC-002 offline network boundary; M25 Scope | M20 offline link/entry; M25 online match implementation | Required original symbols and source-defined offline null network belong to M20, unsupported entry fails clearly. POSIX lobby/discovery/map transfer/start/commands/disconnect/CRC and faulted local peers remain M25; full match remains M16. |
| RC-003 | M21–M25, M15–M18 | All domain, playable gameplay, full-match, clean-release and optional Windows criteria retained. Later milestones add only behavior beyond M20's accepted reachable closure and full scenario acceptance. |
| RC-004 | M20; M21–M25 identity constraints | Actual original class/method/provider/state evidence; real missing-provider failure; four presets, asset-free suite, original consumer sanitizers, fixture/retail init, malformed/factory failure, reverse teardown, CWD/XDG and no-window/GPU mode checks. No startup result counts as playable game completion. |
| RC-005 | index/status/adoption; M20 successor plan during delivery | Preserve M0–M14/M19/M26–M28 history and M20 plans 01/02 with completed slices. Adopt committed readiness-approved revision explicitly; create M20 plan 03 during delivery. Readiness checks actual RC-002/012 edges; no implementation/slice artifact produced by this packet. |
| RC-006 profile and assets | M20 | Identical offline/no-match/no-shell/intro-disabled test profile across four presets, original updates/reset/UI/parser objects and explicit logical asset inventory; independent fixture tests and separate read-only retail initialization. Normal gameplay defaults and later cinematics/shell-map/interactive acceptance remain preserved. |
| RC-006 identity and failure ownership | M20 | Actual before/after source state, target-specific compilation and contributing symbols, conditional-branch review; correction of pre-registration failures, cleared globals, reverse teardown, preserved error and stopped workers. |
| RC-006 incremental and validation boundaries | M20 governing plan and acceptance | Inventory providers/validation before delivery; incremental support/data evidence without requiring the whole runtime first. Active test checks in every preset, NDEBUG disclosure without extending workaround, exact sanitizer configuration plus allocator live-count and resource/worker ownership checks. M19 completed evidence remains bounded and unchanged. |

The allocation table supersedes earlier wording that deferred implementation solely by domain label. Every pre-existing test/acceptance clause remains with its listed domain unless strengthened in M20 as a prerequisite runtime test; none is dropped. Shared SE/RC mappings describe implementation plus later integration acceptance, not repeated rewrites.

## Current remediation coverage

| Required source ID | Provider / acceptance | Original-source evidence obligation |
|---|---|---|
| SE-001 | M19; final closure M17 | Every inventory source classified; actual compile/link records; runtime identity and missing-provider negative control; no bootstrap/toy production closure. |
| SE-002 | M19 bounded completed support; M20 remaining original consumers | Preserve ten-provider support evidence; port original UnicodeString/MemoryPool and remaining ABI/PCH/CRT consumers in M20, with four configurations and original-consumer sanitizers. |
| SE-003 | M20 | Original factory/init/INI/CSF/VFS globals, null-device mode and reverse teardown with failures. |
| SE-004 | M20 required runtime; M21 full scenarios; M15 sessions | Original maps, modules, scripts, AI/pathfinding, commands and victory; actual state checkpoints. |
| SE-005 | M20 required producers; M22 real scenes; M15 sessions | Actual W3D/WWShade/GameClient consumers, real materials/assets, recording and zero-error Vulkan visual acceptance. |
| SE-006 | M20 required interfaces/providers; M23 interactive flows; M15 sessions | Original WND/input/HUD/audio/video managers; real navigation, command transitions, locale and media failures. |
| SE-007 | M20 required recorder/game-state/Xfer/CRC; M24 full scenarios; M15; optional M18 | Original Xfer/SaveGame/recorder/CRC, transactional corrupt-state rejection and cross-compiler replay. |
| SE-008 | M20 offline required symbols/entry; M25 networking; M16 full match | Original network consumers/packets/lockstep, map transfer and independent processes with faults. |
| SE-009 | M15, M16, M17; optional M18 | Real campaign/skirmish/factions, persistence/media, original LAN, packaged launch and distro matrix; existing duration/soak requirements retained. |
| SE-010 | packet index/status; M19–M25 and M15–M18 | Preserve historical facts, qualify component-only assurance, map reopened tests, explicitly adopt replacement goal scope after readiness. |

Every affected pending milestone consumes the supplement's Fixed decisions and boundaries and Evidence grades and anti-proxy checks, with Engineering order superseded by RC-001–RC-012 and the explicit provider redistribution above. No required item is unmapped or blocked by an unresolved product decision.

## RC-012 remaining implementation delta

| Required clause | Owner / consumer | Observable obligation |
|---|---|---|
| Canonical W3D schema closure | M20; consumed by M22/M23 | Production original W3DModuleFactory with all 19 registrations and concrete data defaults/inheritance/fields/callbacks/post-processing/allocation/destruction. Narrow shared original method extraction preserves names, masks, tags and overrides without D3D headers/libraries or replacement schemas. |
| Schema/draw boundary | M20 reachable closure; M22/M23 remaining behavior | All init/post-load/update/reset-reachable original draw operations port now. Only proven-unreachable operations fail closed; ledger records exact operations/callers/owners/tests. Later consuming domains implement them before entry, with no backward M20 dependency. |
| Production positive/negative identity | M20; preservation in M22/M23 | Owned ThingTemplate/INI fixtures cover all registrations and inherited schemas, defaults/fields/malformed values/overrides/teardown. Missing or unknown required provider fails before unsafe dereference; omitted-registration negative control, contributing target identity and no-device checks. |
| Retained BIG adapter and integrated gate | M20 | Malformed/truncated/range/overflow, canonical lookup/override and cleanup tests across four presets; focused sanitizers plus full read-only retail init/update/reset. Earlier fixtures and limited BIG checks are not new acceptance. |
| Preserve history and fresh governing plan | packet/adoption; M20 | Completed provider contracts and slices unchanged, historical blocker retained with superseded classification diagnosis; plan 03 only after readiness-approved committed adoption. |

## Historical assurance reopening

M0/M1 support and M3/M4 engine-facing data assurance reopen through M19/M20; M5 persistence through M21/M24; M2/M7–M10 renderer/UI/effects and M14 real-source scenes through M22/M23; M6 input and M12/M13 original media consumers through M23; M11 original network consumers through M25/M16. M15/M17 prove integrated production closure. All completed contract files, commits and historical test results remain unchanged; no past fixture claim is promoted to original-source acceptance. Existing native APIs, pinned dependencies, VFS/media parsers, Vulkan device and test harnesses are preservation baseline, not wholesale replacement work.

## Required outcomes and governing sections

| Exact source section | Authority class | Milestones | Coverage |
|---|---|---|---|
| §1 Scope and completion criteria | Implementation required + governing constraint | M0–M17 | Native x86-64, explicit roots, complete selected-locale gameplay/render/media/input, Linux persistence/determinism/LAN, GCC/Clang and sanitizers; optional Windows import M18. |
| §2 What is good and should be preserved | Governing constraint | M0–M17 | Preserve existing factories, simulation, W3D, VFS and packet-layer seams. |
| §2 What is risky or blocking | Context/evidence | M0–M17 | Motivates source inventory and targeted changes; no standalone audit-findings milestones. |
| §2 What should change / What should not be refactored yet | Governing constraint | M0–M17 | Narrow platform replacements, no engine/allocator/UI/ECS rewrites. |
| §3 Selected replacement stack | Implementation required + governing constraint | M0, M1, M2, M6–M14, M17 | CMake/C++17/SDL3/SDL_GPU/offline GLSL/miniaudio/FFmpeg/FreeType/Fontconfig/POSIX/zlib; conditional locale additions only. |
| §4 Target architecture and build graph | Implementation required | M0, M2, M7, M10, M12, M13, M17 | Explicit source targets including WWShade; six supported options; imported dependencies, version headers, offline shaders, four presets/labels, install/license boundary. |
| §5 Fixed-width types | Implementation required | M1, M5, M11 | Integer/pointer widths, no implicit Win32 layouts or unaligned host reads. |
| §5 Unicode | Implementation required | M1, M4, M5, M6, M8, M11 | char16_t, UTF-16LE, bounded helpers/surrogates, UTF-8 boundary conversion; never -fshort-wchar. |
| §5 Serialization and packets | Implementation required | M1, M5, M11, M18 | Shared fixed-width codecs, source-proven layouts, versioned fail-before-mutation readers. |
| §5 Floating-point behavior | Implementation required | M1, M5, M15, M16, M18 | Characterization, checked helpers, FE_TONEAREST/isolation, repeatable CRCs and optional Windows comparison. |
| §6 Entry point, SDL, and input | Implementation required | M3, M6, M8, M15 | Staged startup/teardown, modes, input/IME, no CWD mutation, focus recovery. |
| §6 Filesystem, data roots, and XDG paths | Implementation required | M1, M3, M4, M5, M15, M16, M17 | CLI/config precedence, locale ambiguity, XDG, exact VFS ordering/case collisions, bounded BIG and read-only verification. |
| §6 GPU-independent renderer closure | Implementation required | M2, M7, M14 | Complete public-API mapping/GLSL/device interface, normalized recorder, real device gate. |
| §6 Renderer implementation | Implementation required | M2, M7–M10, M13, M14 | Opaque handles/cache, fixed DDS/TGA/BC fallbacks, effect tables, conventions, recreation/debug labels. |
| §6 Audio | Implementation required | M12, M13, M15 | VFS streams/full manager semantics, callback safety, format completeness, silent no-device behavior. |
| §6 Video | Implementation required | M13–M15 | Bounded FFmpeg custom I/O, Bink/audio clock/fallback/skip/pause/EOS/aspect and licenses. |
| §6 Fonts and locale | Implementation required | M8, M14, M15 | Memory faces/lifetime, Fontconfig fallbacks, atlas metrics/glyph safety and tested locale gate. |
| §6 Time, threads, diagnostics, and compression | Implementation required | M1, M3, M4, M5, M7, M11–M15 | Monotonic/wrap clocks, joined workers, capability reporting, preserved compression, typed NOX error, bounded fuzzable readers. |
| §6 LAN and removed online services | Implementation required | M0, M3, M8, M11, M16 | POSIX transport, local overrides, fixed packets, discovery alternatives; remove online network/waiting paths. |
| §7 Prerequisite ledger and dependency order | Governing constraint + implementation required | M0–M18 | PRE mapping below, canonical commands/labels, existing graph, separated retail/GPU builds. |
| §9 Renderer backend at M14 | Conditional governing constraint | M2, M7–M10, M14 | Evidence-proven abstraction gap triggers bgfx then raw Vulkan choice and revalidation; no private escape hatch. |
| §9 Supplied locale at M8 | Conditional governing constraint | M8, M14, M15 | Only demonstrated shaping/bidi need adds HarfBuzz/FriBidi. |
| §9 Unexpected required asset formats at M4 or consuming milestone | Conditional governing constraint | M4, M7–M10, M12, M13 | Narrow supported adapter or maintained licensed in-process decoder; otherwise block with exact asset, never skip/convert. |
| §9 Loopback discovery at M11/M16 | Conditional governing constraint | M11, M16 | Virtual discovery plus real direct-connect fallback; distinct identity injection/namespaces, no second host. |
| §9 Packaging after M17 | Excluded/deferred | None | Flatpak/AppImage/superbuild not required for source-build release. |
| §10 Validation matrix — Build | Implementation required | M0, M2, M17 | Four presets, shaders, offline/failure cases and distribution checks. |
| §10 Validation matrix — Types/Unicode; Numeric/determinism; Serialization | Implementation required | M1, M5, M8, M11, M15, M18 | Primitive/codec negative boundaries, CRCs and optional fixture isolation. |
| §10 Validation matrix — Data/VFS; Renderer contracts; Input/text | Implementation required | M4, M6–M10, M13–M15 | Synthetic + private corpus, command validation, focus/text safety and visual gate. |
| §10 Validation matrix — Audio; Video | Implementation required | M12–M15 | Decode/scheduling/no-device, bounds/corruption, A/V on hardware. |
| §10 Validation matrix — Simulation; LAN; Process | Implementation required | M3, M5, M11, M15–M17 | Headless/persistence/long runs, local peers, CRC/errors, XDG/multi-process/partial failure. |
| §11 Overall recommendation | Governing summary/context | M0–M17 | Compatibility-focused native port and M14 hard hardware gate; no extra obligations beyond preceding sections. |

## Source-defined milestones

| Source §8 heading | Contract | Disposition |
|---|---|---|
| M0 — reproducible build readiness | [M0](M0-build-graph.md) | Implementation required; scope and complete exit preserved |
| M1 — portable ABI and support libraries | [M1](M1-portable-foundations.md) | Implementation required; scope and complete exit preserved |
| M2 — GPU-independent renderer API closure | [M2](M2-renderer-closure.md) | Implementation required; scope and complete exit preserved |
| M3 — headless engine and staged startup | [M3](M3-headless-startup.md) | Implementation required; scope and complete exit preserved |
| M4 — VFS, retail data, and corpus characterization | [M4](M4-retail-vfs.md) | Implementation required; scope and complete exit preserved |
| M5 — Linux persistence and determinism | [M5](M5-persistence.md) | Implementation required; scope and complete exit preserved |
| M6 — SDL platform and input without GPU | [M6](M6-sdl-platform.md) | Implementation required; scope and complete exit preserved |
| M7 — renderer core contracts with `RecordingGpuDevice` | [M7](M7-renderer-core.md) | Implementation required; scope and complete exit preserved |
| M8 — UI, fonts, and input integration | [M8](M8-ui-fonts.md) | Implementation required; scope and complete exit preserved |
| M9 — world rendering command generation | [M9](M9-world-rendering.md) | Implementation required; scope and complete exit preserved |
| M10 — effects, water, and WWShade command generation | [M10](M10-effects.md) | Implementation required; scope and complete exit preserved |
| M11 — POSIX LAN transport and local peer harness | [M11](M11-lan-transport.md) | Implementation required; scope and complete exit preserved |
| M12 — audio | [M12](M12-audio.md) | Implementation required; scope and complete exit preserved |
| M13 — headless video decode and presentation commands | [M13](M13-video.md) | Implementation required; scope and complete exit preserved |
| M14 — first real-GPU integration and visual acceptance | [M14](M14-gpu-acceptance.md) | Implementation required; scope and complete exit preserved |
| M15 — playable single-player | [M15](M15-single-player.md) | Implementation required; scope and complete exit preserved |
| M16 — local multi-instance Linux LAN | [M16](M16-lan-match.md) | Implementation required; scope and complete exit preserved |
| M17 — reproducible x86-64 release | [M17](M17-release.md) | Implementation required; scope and complete exit preserved |
| M18 — optional Windows save/replay compatibility | [M18](M18-windows-compatibility.md) | Optional; no mandatory release dependency |

## Prerequisite production and consumption

| ID | Owner milestone / first consumer | Disposition |
|---|---|---|
| PRE-001 | M0 | Repository source/manifests input |
| PRE-002 | M0 | Developer-installed toolchain/dependencies; M0 documents/probes |
| PRE-003 | M0 | M0 produces include/exclude/dependency manifests |
| PRE-004 | M0 | M0 produces CMake/presets/labels/CI fixtures |
| PRE-005 | M1 | M1 produces portable support consumed downstream |
| PRE-006 | M2 → M7 | M2 closes source/API mapping |
| PRE-007 | M3 → M4/M11 | M3 provides deterministic null-device execution |
| PRE-008 | M4 | User-owned read-only retail roots/English; subsequent corpus consumers |
| PRE-009 | M4 → M7–M17 | M4 produces logical corpus metadata |
| PRE-010 | M7 → M8–M10/M13 | M7 produces recording device/schema |
| PRE-011 | M12 | Initial M12 pin/license review precedes integration |
| PRE-012 | M14 | User/host GPU + validation layer; backend-open acceptance after implementation |
| PRE-013 | M14 → M15 | M14 creates visual/lifecycle capability acceptance |
| PRE-014 | M11 → M16 | M11 creates local identity overrides/virtual datagrams |
| PRE-015 | M18 only | Optional Windows fixtures; unavailable means not validated |
| PRE-016 | M6 | Interactive window/input check; injected tests proceed independently |
| PRE-017 | M17 | Clean Ubuntu/Fedora environments, not additional physical GPU hosts |

## Excluded, deferred, blocked and unmapped work

- Excluded by §1/§2: original Generals executable, authoring/utility tools, DRM/launcher/serial behavior, GameSpy/Internet replacement, browser/Windows Media/recording, controllers/IPv6/mod manager/conversion/remaster, non-x86-64, and unrelated simulation/ECS/ownership refactors. No milestone is created for them.
- Deferred by authority: Windows 1.04 import validation is optional M18; mixed Windows/Linux LAN, extra GPU families and physical broadcast remain unverified unless separately evidenced. Flatpak/AppImage/superbuild follow M17 only under future authority.
- Conditional inclusion: Granny or unexpected formats only when ordinary required retail paths demonstrate necessity under §9. Locale and backend alternatives follow their evidence gates.
- Current evidence: historical retail/archive and Vulkan/validation-layer component tests exist, but do not replace M20–M25 original-source integration. Readiness must re-audit host availability and future clean distro environments. Windows fixtures remain optional.
- Blocked implementation-required source decisions: none at compilation. External prerequisites are explicit at their consumers and are audited by readiness after compilation.
- Unmapped implementation-required items: none. RC-001–RC-012, SE-001–SE-010 and all base-plan obligations have providers/integrated acceptance owners above; M18 remains separately optional. No implementation work arises solely from review observations. RC-005/012 adoption awaits parent readiness and committed handoff; preserved blocked status is delivery history, not an unresolved source decision.
