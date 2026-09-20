# Source traceability

Authority is the accepted [migration supplement](../../../docs/zero-hour-source-engine-migration.md) with the [base plan](../../../docs/zero-hour-linux-port-plan.md) as governing behavior/stack authority; exact revisions and commits are in status.yaml. Baseline is existing native components plus unported original source. The architecture review is evidence only. The historical mappings below preserve initial intent, not implementation assurance.

## Current remediation coverage

| Required source ID | Provider / acceptance | Original-source evidence obligation |
|---|---|---|
| SE-001 | M19; final closure M17 | Every inventory source classified; actual compile/link records; runtime identity and missing-provider negative control; no bootstrap/toy production closure. |
| SE-002 | M19 | Original portable support, allocators/strings/chunks/codecs; four compilers/configurations and real support sanitizers. |
| SE-003 | M20 | Original factory/init/INI/CSF/VFS globals, null-device mode and reverse teardown with failures. |
| SE-004 | M21; whole sessions M15 | Original maps, modules, scripts, AI/pathfinding, commands and victory; actual state checkpoints. |
| SE-005 | M22; whole sessions M15 | Actual W3D/WWShade/GameClient consumers, real materials/assets, recording and zero-error Vulkan visual acceptance. |
| SE-006 | M23; whole sessions M15 | Original WND/input/HUD/audio/video managers; real navigation, command transitions, locale and media failures. |
| SE-007 | M24; gameplay M15; optional import M18 | Original Xfer/SaveGame/recorder/CRC, transactional corrupt-state rejection and cross-compiler replay. |
| SE-008 | M25; full match M16 | Original network consumers/packets/lockstep, map transfer and independent processes with faults. |
| SE-009 | M15, M16, M17; optional M18 | Real campaign/skirmish/factions, persistence/media, original LAN, packaged launch and distro matrix; existing duration/soak requirements retained. |
| SE-010 | packet index/status; M19–M25 and M15–M18 | Preserve historical facts, qualify component-only assurance, map reopened tests, explicitly adopt replacement goal scope after readiness. |

Every new and affected pending milestone also consumes the supplement's Fixed decisions and boundaries, Engineering order and readiness boundary, and Evidence grades and anti-proxy checks. Repeated mapping separates provider implementation from integrated acceptance, not duplicate implementation. No required item is unmapped or blocked by an unresolved product decision.

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
- Unmapped implementation-required items: none. SE-001–SE-010 and all base-plan obligations have providers/integrated acceptance owners above; M18 remains separately optional. No implementation work arises solely from review observations.
