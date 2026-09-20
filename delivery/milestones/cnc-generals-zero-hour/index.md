# Zero Hour native Linux delivery milestones

Input mode: **CHANGE_PLAN**. Product: `cnc-generals-zero-hour`. Change: `RUNTIME-CLOSURE-2026-09-20`.

Current authority: [runtime dependency reconciliation](../../../docs/zero-hour-runtime-closure-reconciliation.md), revision `sha256:463c74ffd2cb1763e7d16247b4e218d0437b5ac70ef7f22e9fbe9f2a09e40e80`, committed at `8cf8f6ce3842cffe37505d02e610ca825ecdb532`. Its RC-001–RC-011 supersede conflicting engineering order/allocation in the supplement below and preserve every required behavior. It is an accepted architecture change plan, not a canonical Product Definition reconciliation result.

Authority: [original-engine migration supplement](../../../docs/zero-hour-source-engine-migration.md), revision `sha256:4e9523ec2854750b896716bfd4583e47fb65136a9a2188db9dcd4ac9b552c817`, committed at `38b608d9109d13ad1c6e3a527fe34893512d4d3f`. The [base plan](../../../docs/zero-hour-linux-port-plan.md) remains governing product/stack/compatibility authority at its revision recorded in status.yaml. No definition package or reconciliation result governs this directly authorized architecture remediation. The [review](../../../evidence/qa/cnc-generals-zero-hour/architecture-revalidation-2026-09-20.md) is evidence, not additional implementation authority.

## Historical assurance correction

M0–M14 completion history, commits, tests and contract files are preserved. Those results validate components/fixtures, not a compiled, linked and running original engine. Support/startup, persistence, UI/world/effects, network/media adapters and real-source GPU scenes require the new providers below. M15 is still blocked; its useful slices are scaffolding, not playable retail gameplay. Native components are retained and adapted, not rebuilt wholesale. No completion or release claim follows from this replanning.

## Contracts in dependency-safe execution order

Numeric IDs remain stable; list order, not numeric sorting, determines execution. M19 is completed. New M26–M28 prove independent original process/data/CPU providers, revised M20 owns coupled runtime integration, M21–M25 complete domain scenarios, and M15–M18 retain integrated acceptance.

| ID | Outcome / contract | Direct dependencies | Status |
|---|---|---|---|
| M0 | [reproducible build readiness](M0-build-graph.md) | PRE-001/002 | completed historically; component evidence only |
| M1 | [portable ABI and support libraries](M1-portable-foundations.md) | M0 | completed historically; component evidence only |
| M2 | [GPU-independent renderer API closure](M2-renderer-closure.md) | M0 | completed historically; component evidence only |
| M3 | [headless engine and staged startup](M3-headless-startup.md) | M1 | completed historically; component evidence only |
| M4 | [VFS, retail data, and corpus characterization](M4-retail-vfs.md) | M1, M3 | completed historically; component evidence only |
| M5 | [Linux persistence and determinism](M5-persistence.md) | M4 | completed historically; component evidence only |
| M6 | [SDL platform and input without GPU](M6-sdl-platform.md) | M1, M3 | completed historically; component evidence only |
| M7 | [renderer core contracts with `RecordingGpuDevice`](M7-renderer-core.md) | M2, M3, M4 | completed historically; component evidence only |
| M8 | [UI, fonts, and input integration](M8-ui-fonts.md) | M4, M6, M7 | completed historically; component evidence only |
| M9 | [world rendering command generation](M9-world-rendering.md) | M4, M7 | completed historically; component evidence only |
| M10 | [effects, water, and WWShade command generation](M10-effects.md) | M4, M7 | completed historically; component evidence only |
| M11 | [POSIX LAN transport and local peer harness](M11-lan-transport.md) | M3, M4 | completed historically; component evidence only |
| M12 | [audio](M12-audio.md) | M4 | completed historically; component evidence only |
| M13 | [headless video decode and presentation commands](M13-video.md) | M4, M7, M12 | completed historically; component evidence only |
| M14 | [first real-GPU integration and visual acceptance](M14-gpu-acceptance.md) | M7, M8, M9, M10, M13 | completed historically; component evidence only |
| M19 | [original support closure and source identity](M19-original-support.md) | M0, M1 | completed; bounded ten-provider evidence |
| M26 | [original process allocation and ABI foundation](M26-original-process-foundation.md) | M19 | pending |
| M27 | [original data codecs and map metadata providers](M27-original-data-providers.md) | M26, M4 | pending |
| M28 | [original CPU presentation and media resources](M28-original-cpu-resources.md) | M27, M6, M7, M8, M12, M13 | pending |
| M20 | [integrated original runtime and data lifecycle](M20-original-lifecycle.md) | M28 | blocked delivery checkpoint; revised scope awaits adoption |
| M21 | [original map and simulation execution](M21-original-simulation.md) | M20 | pending |
| M22 | [original retail scene rendering on Vulkan](M22-original-rendering.md) | M21, M14 | pending |
| M23 | [original menus controls and media integration](M23-original-interaction.md) | M22, M6, M8, M12, M13 | pending |
| M24 | [original saves replay and deterministic state](M24-original-persistence.md) | M21, M5 | pending |
| M25 | [original network lockstep integration](M25-original-network.md) | M24, M11 | pending |
| M15 | [playable single-player](M15-single-player.md) | M23, M24 | blocked on providers |
| M16 | [local multi-instance Linux LAN](M16-lan-match.md) | M25, M15 | pending |
| M17 | [reproducible x86-64 release](M17-release.md) | M15, M16 | pending |
| M18 | [optional Windows save/replay compatibility](M18-windows-compatibility.md) | M17 | pending, optional |

## Dependency rationale

M19's bounded ten-provider original support is preserved. M26 proves original process/allocation/ABI and the shared FP helper; M27 proves independent original data/codec/metadata operations, and M28 proves CPU resource/device seams and audio definitions. Their small original-consumer targets do not require a whole engine: full INI dispatch, ThingFactory-dependent map classification and complete registries stay with M20. M20 consumes these providers and integrates complete original offline startup/post-load/update/reset/destruction, including process ownership, MapCache/GameLOD side effects and noninteractive failures. M21 adds full map start/simulation (including CPU preload/props/recorder), never deferred to hardware milestones. M22 hardware and M24 save/replay branch from M21; M23 interaction uses M22; M25 networking uses M24. M15 requires M23/M24, M16 M25/M15, M17 the integrated release. Null sinks are device-only; source-defined offline network remains null. No new M0 or implementation slices are created.

## Replacement-packet adoption

RC-007–RC-010 authorize three independently executable prerequisites and a checked source-dependency ledger, replacing the earlier one-milestone preparation constraint. Process/static allocation, original data/codec/map metadata and CPU device/resource seams are separate evidence-producing outcomes, not inventory-only milestones. Full registry/startup coupling stays in M20. All RC-001–RC-006 requirements and later acceptance remain mapped in traceability.

RC-011 preserves this order: M26 owns optional allocation-safe MemoryPools.ini bootstrap before the game filesystem exists, without a backward dependency on M27. M17 requires original-engine M22/M23 plus integrated M15/M16 evidence and actual installed-prefix execution; historical M14 component evidence or staged copying alone is insufficient. These are strengthened existing outcomes, not separate milestones.

The active goal adopted M19–M25/M15–M18 and completed M19 before the M20 order blocker. Preserve this goal/history and explicitly adopt the committed readiness-approved packet with new M26–M28 membership. Remaining order is M26, M27, M28, M20, M21, M22, M23, M24, M25, M15, M16, M17, optional M18. First create M26 plan 01 during delivery. Preserve blocked M20 plan 01 and use M20 plan 02 after its providers complete. M24/M25 may run before the independent rendering branch when safe. Planning ends at handoff; no implementation starts here.

## Shared constraints and known gates

## Shared writable-path contract


Use `$XDG_CONFIG_HOME/generals-zero-hour/` for configuration, `$XDG_DATA_HOME/generals-zero-hour/` for saves/replays/user maps/screenshots, `$XDG_STATE_HOME/generals-zero-hour/` for logs/run state, and `$XDG_CACHE_HOME/generals-zero-hour/` for regenerable indices/caches. If unset, fall back respectively to `$HOME/.config/generals-zero-hour/`, `$HOME/.local/share/generals-zero-hour/`, `$HOME/.local/state/generals-zero-hour/`, and `$HOME/.cache/generals-zero-hour/`. Never use CWD or retail roots for writable data. M1 establishes helpers, M3/M4 consume them and M15–M17 validate gameplay/multi-process/install behavior.

## Current readiness

**READY_WITH_EXTERNAL_DEPENDENCIES** following the RC-011 re-audit of this 29-contract revision. The [readiness report](../../readiness/cnc-generals-zero-hour/readiness-report.md) and [manifest](../../readiness/cnc-generals-zero-hour/prerequisite-manifest.md) retain the independent M26–M28 seams and coupled M20 closure, and verify allocation-safe pre-VFS configuration plus original-engine installed-release evidence. M26 entry is clear after committed handoff/adoption. Later retail/display/GPU/clean-distro and optional Windows gates retain their acceptance boundaries. Historical M0–M14/M19 remain unchanged; blocked M20 history stays preserved. Readiness is not runtime acceptance or authorization to skip provider milestones.

Preserve original engine/simulation/UI/asset/serialization semantics, Linux x86-64 and the selected stack. All writes use isolated XDG roots, never CWD/retail; the original symlink remains read-only. Ordinary tests remain asset/device independent; real-source retail and hardware tests are explicitly gated. Never commit retail bytes/hashes/private paths. Historical tests remain useful regression evidence. Each new acceptance result names its evidence grade and proves compile, link and actual source execution separately; missing required original providers must fail, never fall back to toys.

Retail data and Arch Vulkan/validation layers have prior component evidence; source-consumer tests remain unimplemented. Clean Ubuntu/Fedora release environments remain later external gates. Optional Windows fixtures never block release. Existing ATL errors and unported sources are owned implementation deliverables in M26–M28/M20, not external prerequisites to begin M26.

## Validation and handoff

The packet has 29 stable contracts: 15 historically completed component milestones, completed bounded-source M19, new pending M26–M28, blocked M20 awaiting adoption, pending M21–M25, blocked M15, pending M16/M17 and optional M18. [Traceability](traceability.md) maps all eleven RC and ten SE IDs and preserves every base-plan obligation. [Status](status.yaml) preserves history/provenance. Compiler checks structure, coverage and backward dependencies in list order; readiness owns prerequisite audit. Parent planning must finalize readiness and commit provenance before downstream use.
