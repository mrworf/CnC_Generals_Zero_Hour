# Zero Hour native Linux delivery milestones

Input mode: **CHANGE_PLAN**. Product: `cnc-generals-zero-hour`. Current change: `RENDERER-BGFX-2026-09-21`, canonical reconciliation `REC-RUN-RENDERER-BGFX-2026-09-21`, resolved remediation `REC-RM-001`.

The current implementation authority is the accepted [renderer backend migration](../../../docs/zero-hour-renderer-backend-migration.md) RB-01–RB-04 at `64c7632dcc93a5a5d9fc1519467140828a3a8813`, under [port plan §9](../../../docs/zero-hour-linux-port-plan.md). The [canonical reconciliation result](../../../workflow/reconciliation/renderer-bgfx-2026-09-21/reconciliation-result.yaml) at `25ef60575bfc13aeac5a6aa7146da1af2626e59c` clears definition revalidation and delivery-planning gates; its [resolved backlog](../../../workflow/reconciliation/renderer-bgfx-2026-09-21/03-remediation-backlog.md) is the compilable delta. M22's backend-gap report and the public bgfx probe are evidence, not independent implementation authority. Exact revisions and IDs are in `status.yaml`.

Preserved earlier authority: [runtime dependency reconciliation](../../../docs/zero-hour-runtime-closure-reconciliation.md), revision `sha256:847fa3b1cc5883cb0ebcedfd11bba715326c66dd4835e3a229a13e93cf96edd2`, committed at `cb567c223beab5d63fa2f13d66718e670271d3d0`. Its RC-001–RC-012 supersede conflicting engineering order/allocation in the supplement below and preserve every required behavior. It is an accepted architecture change plan, not the current canonical Product Definition reconciliation result.

Preserved earlier authority: [original-engine migration supplement](../../../docs/zero-hour-source-engine-migration.md), revision `sha256:4e9523ec2854750b896716bfd4583e47fb65136a9a2188db9dcd4ac9b552c817`, committed at `38b608d9109d13ad1c6e3a527fe34893512d4d3f`. The [base plan](../../../docs/zero-hour-linux-port-plan.md) remains governing product/stack/compatibility authority at its revision recorded in status.yaml. No definition package governed the earlier directly authorized migration; the current renderer delta has the canonical reconciliation above. The [review](../../../evidence/qa/cnc-generals-zero-hour/architecture-revalidation-2026-09-20.md) is evidence, not additional implementation authority.

## Historical assurance correction

M0–M14 completion history, commits, tests and contract files are preserved. Those results validate components/fixtures, not a compiled, linked and running original engine. Support/startup, persistence, UI/world/effects, network/media adapters and real-source GPU scenes require the new providers below. M15 is still blocked; its useful slices are scaffolding, not playable retail gameplay. Native components are retained and adapted, not rebuilt wholesale. No completion or release claim follows from this replanning.

## Contracts in dependency-safe execution order

Numeric IDs remain stable; list order, not numeric sorting, determines execution. M0–M14, M19, M26–M28 and M20–M21 retain their completed histories. New M29 supplies the backend-neutral clear/shader contract; M30 consumes it and revalidates the public bgfx device. M22 then resumes original retail scenes, M24 resumes dependent first-tick/restore checks, and later integrated outcomes retain their existing boundaries. The current packet has 31 contracts; only two are new.

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
| M26 | [original process allocation and ABI foundation](M26-original-process-foundation.md) | M19 | completed; scoped provider evidence |
| M27 | [original data codecs and map metadata providers](M27-original-data-providers.md) | M26, M4 | completed; scoped provider evidence |
| M28 | [original CPU presentation and media resources](M28-original-cpu-resources.md) | M27, M6, M7, M8, M12, M13 | completed; scoped provider evidence |
| M20 | [integrated original runtime and data lifecycle](M20-original-lifecycle.md) | M28 | completed; original runtime evidence |
| M21 | [original map and simulation execution](M21-original-simulation.md) | M20 | completed; original simulation evidence |
| M29 | [ordered-clear renderer contract and shader closure](M29-renderer-contract-migration.md) | M8, M9, M10 | pending; RB-01/RB-03 |
| M30 | [public bgfx device and renderer revalidation](M30-bgfx-device-validation.md) | M29, M14 | pending; RB-02/RB-04 |
| M22 | [original retail scene rendering on Vulkan](M22-original-rendering.md) | M21, M30 | blocked; resume after M30 |
| M23 | [original menus controls and media integration](M23-original-interaction.md) | M22, M6, M8, M12, M13 | pending |
| M24 | [original saves replay and deterministic state](M24-original-persistence.md) | M22, M5 | blocked; completed slices retained |
| M25 | [original network lockstep integration](M25-original-network.md) | M24, M11 | pending |
| M15 | [playable single-player](M15-single-player.md) | M23, M24 | blocked on providers |
| M16 | [local multi-instance Linux LAN](M16-lan-match.md) | M25, M15 | pending |
| M17 | [reproducible x86-64 release](M17-release.md) | M15, M16 | pending |
| M18 | [optional Windows save/replay compatibility](M18-windows-compatibility.md) | M17 | pending, optional |

## Dependency rationale

M19's bounded ten-provider original support is preserved. M26–M28 and M20–M21 have completed their scoped provider/runtime/simulation gates. The renderer migration branches from accepted M8–M10 component commands: M29 establishes the backend-neutral ordered-clear and offline shader contract, then M30 consumes it and revalidates M14-grade hardware behavior on bgfx. M22 consumes M21 original simulation and M30 device acceptance; M24's remaining mission/skirmish paths require M22's original draw provider, while its completed persistence slices remain intact. M23 interaction uses M22; M25 networking uses M24. M15 requires M23/M24, M16 M25/M15, M17 integrated release. No generic M0, renumbering, or implementation slices are created. M29 can be planned before M22 without implying that M22's historical work is discarded.

## Replacement-packet adoption

RC-007–RC-010 authorize three independently executable prerequisites and a checked source-dependency ledger, replacing the earlier one-milestone preparation constraint. Process/static allocation, original data/codec/map metadata and CPU device/resource seams are separate evidence-producing outcomes, not inventory-only milestones. Full registry/startup coupling stays in M20. All RC-001–RC-006 requirements and later acceptance remain mapped in traceability.

RC-011 preserves this order: M26 owns optional allocation-safe MemoryPools.ini bootstrap before the game filesystem exists, without a backward dependency on M27. M17 requires original-engine M22/M23 plus integrated M15/M16 evidence and actual installed-prefix execution; historical M14 component evidence or staged copying alone is insufficient. These are strengthened existing outcomes, not separate milestones.

RC-012 clarified the original 19-registration W3D schema factory and draw-instance boundary within now-completed M20, plus the retained BIG adapter's incomplete acceptance. These shared production ThingTemplate/INI and full startup lifecycle acceptance; a separate schema-only milestone would have split that coupled outcome without an independent integrated gate. M22/M23 consume the same canonical schemas and replace ledger-deferred draw operations before their paths execute. The present M29/M30 migration does not weaken the retail gate.

The prior RC-012 adoption and subsequent M20/M21 acceptances remain history, not work to rerun. Preserve M22 plan 01 and its completed source-owned slices through 06C3B3B, and M24 plan 01 with completed slices 01–05B. New planning order is M29 → M30 → M22; M24 cannot complete before M22's draw provider. M23 may branch after M22, and all later dependencies remain. Create M29/M30 implementation plans only during delivery, resume M22 at 06C3C after M30, and resume M24 at 05C after M22. Planning ends at handoff; no implementation starts here.

## Shared constraints and known gates

## Shared writable-path contract


Use `$XDG_CONFIG_HOME/generals-zero-hour/` for configuration, `$XDG_DATA_HOME/generals-zero-hour/` for saves/replays/user maps/screenshots, `$XDG_STATE_HOME/generals-zero-hour/` for logs/run state, and `$XDG_CACHE_HOME/generals-zero-hour/` for regenerable indices/caches. If unset, fall back respectively to `$HOME/.config/generals-zero-hour/`, `$HOME/.local/share/generals-zero-hour/`, `$HOME/.local/state/generals-zero-hour/`, and `$HOME/.cache/generals-zero-hour/`. Never use CWD or retail roots for writable data. M1 establishes helpers, M3/M4 consume them and M15–M17 validate gameplay/multi-process/install behavior.

## Current readiness

The renderer-migration packet is **READY_WITH_EXTERNAL_DEPENDENCIES** under the refreshed [readiness report](../../readiness/cnc-generals-zero-hour/readiness-report.md) and [prerequisite manifest](../../readiness/cnc-generals-zero-hour/prerequisite-manifest.md). M29 is the first executable milestone; M30's renewed GPU acceptance requires the declared RTX/Vulkan/graphical session, and M22 still owns original retail scenes. Clean-distro and optional Windows gates retain their boundaries. Readiness is consumable only after the parent planning transaction commits and finalizes its handoff; it does not clear blocked delivery state by itself.

Preserve original engine/simulation/UI/asset/serialization semantics, Linux x86-64 and the selected stack. All writes use isolated XDG roots, never CWD/retail; the original symlink remains read-only. Ordinary tests remain asset/device independent; real-source retail and hardware tests are explicitly gated. Never commit retail bytes/hashes/private paths. Historical tests remain useful regression evidence. Each new acceptance result names its evidence grade and proves compile, link and actual source execution separately; missing required original providers must fail, never fall back to toys.

Retail data and Arch Vulkan/validation layers have prior component evidence; full original-source retail integration remains unaccepted. Clean Ubuntu/Fedora release environments remain later external gates. Optional Windows fixtures never block release. M20 startup/provider closure is completed history; remaining renderer device and original draw work is assigned to M29/M30/M22, not an external Direct3D SDK prerequisite.

## Validation and handoff

The packet has 31 stable contracts: 29 prior contracts plus M29/M30. [Traceability](traceability.md) maps REC-RM-001/RB-01–RB-04 and preserves all prior RC, SE and base-plan obligations. [Status](status.yaml) preserves completed history and records reconciliation provenance. Compiler checks structure, coverage and backward dependencies in list order; readiness owns prerequisite audit. Parent planning must finalize readiness and commit provenance before downstream use.
