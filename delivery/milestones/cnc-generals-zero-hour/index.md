# Zero Hour native Linux delivery milestones

Input mode: **CHANGE_PLAN**. Product: `cnc-generals-zero-hour`.

Authority: [zero-hour-linux-port-plan.md](../../../docs/zero-hour-linux-port-plan.md), accepted implementation-ready migration plan at source commit `8dc0285ff6c3346273973dcd874e6983060bb3e7`, SHA-256 `25554c2db1273e263b574b8563615c88ed48eb4f7672e5a21e19fe119eae7b33`. No definition-package or reconciliation result governs this direct accepted change plan.

Baseline: the legacy Zero Hour source does not build on Linux. Preserve engine simulation, W3D/BIG/INI/CSF, UI and source-established serialization behavior; replace platform edges and repair unsafe host representations. Target native Arch Linux x86-64 playable acceptance, English owned retail corpus, and Ubuntu 26.04/Fedora 44 clean-build portability. All 19 contracts are pending implementation. M18 is optional.

## Contracts

Each title names the independently observable outcome; links contain full scope and acceptance contracts. All rows trace to the corresponding §8 milestone plus the shared requirements in [traceability.md](traceability.md).

| ID | Outcome / contract | Direct dependencies | Status |
|---|---|---|---|
| M0 | [reproducible build readiness](M0-build-graph.md) | PRE-001/002 | pending |
| M1 | [portable ABI and support libraries](M1-portable-foundations.md) | M0 | pending |
| M2 | [GPU-independent renderer API closure](M2-renderer-closure.md) | M0 | pending |
| M3 | [headless engine and staged startup](M3-headless-startup.md) | M1 | pending |
| M4 | [VFS, retail data, and corpus characterization](M4-retail-vfs.md) | M1, M3 | pending |
| M5 | [Linux persistence and determinism](M5-persistence.md) | M4 | pending |
| M6 | [SDL platform and input without GPU](M6-sdl-platform.md) | M1, M3 | pending |
| M7 | [renderer core contracts with `RecordingGpuDevice`](M7-renderer-core.md) | M2, M3, M4 | pending |
| M8 | [UI, fonts, and input integration](M8-ui-fonts.md) | M4, M6, M7 | pending |
| M9 | [world rendering command generation](M9-world-rendering.md) | M4, M7 | pending |
| M10 | [effects, water, and WWShade command generation](M10-effects.md) | M4, M7 | pending |
| M11 | [POSIX LAN transport and local peer harness](M11-lan-transport.md) | M3, M4 | pending |
| M12 | [audio](M12-audio.md) | M4 | pending |
| M13 | [headless video decode and presentation commands](M13-video.md) | M4, M7, M12 | pending |
| M14 | [first real-GPU integration and visual acceptance](M14-gpu-acceptance.md) | M7, M8, M9, M10, M13 | pending |
| M15 | [playable single-player](M15-single-player.md) | M5, M8, M9, M10, M12, M13, M14 | pending |
| M16 | [local multi-instance Linux LAN](M16-lan-match.md) | M8, M11, M15 | pending |
| M17 | [reproducible x86-64 release](M17-release.md) | M15, M16 | pending |
| M18 | [optional Windows save/replay compatibility](M18-windows-compatibility.md) | M17 | pending (optional) |

## Shared writable-path contract

Use `$XDG_CONFIG_HOME/generals-zero-hour/` for configuration, `$XDG_DATA_HOME/generals-zero-hour/` for saves/replays/user maps/screenshots, `$XDG_STATE_HOME/generals-zero-hour/` for logs/run state, and `$XDG_CACHE_HOME/generals-zero-hour/` for regenerable indices/caches. If unset, fall back respectively to `$HOME/.config/generals-zero-hour/`, `$HOME/.local/share/generals-zero-hour/`, `$HOME/.local/state/generals-zero-hour/`, and `$HOME/.cache/generals-zero-hour/`. Never use CWD or retail roots for writable data. M1 establishes helpers, M3/M4 consume them and M15–M17 validate gameplay/multi-process/install behavior.

## Dependency and acceptance boundaries

M0 produces the common build graph; M1→M3→M4 enables data/simulation/audio/LAN work, while M2 independently closes the source/API renderer inventory. M7 consumes M2/M3/M4; M8 also requires platform M6, and M9/M10 consume M4/M7. M13 joins media and renderer commands before M14 hardware acceptance. M15 joins complete single-player subsystems; M16 adds full local LAN; M17 accepts release. M18 has no reverse dependency and cannot block M0–M17.

The source-defined M0 is retained; no extra setup milestone or implementation slices are created. Source-listed direct dependencies are preserved even where transitive. Milestones remain open until their complete exit is met.

## Known external and conditional gates

- PRE-008: supplied retail symlink resolves ZH root and ZH_Generals base root, English. M4 must still produce formal read-only verification and corpus metadata; initial observations are not acceptance.
- PRE-016: M6 interactive display smoke, with headless event development available earlier.
- PRE-011: M12 begins with reviewed/hash-pinned miniaudio acquisition before integration; normal configure/build remains offline.
- PRE-012: M14 requires accessible RTX 4070 Vulkan host and Khronos validation tooling; install the missing Arch validation-layer package before device checks. SDL_GPU opening is verified after the M14 backend is implemented.
- PRE-017: clean Ubuntu/Fedora environments at M17; optional PRE-015 Windows fixtures at M18.
- Source §9 backend fallback, locale shaping, unexpected formats and loopback discovery decisions remain conditional evidence gates, not guessed approvals or current planning blockers.

## Validation and handoff

The compiler checks contract structure, provenance, source coverage, filenames and backward dependency order. Readiness status is **READY_WITH_EXTERNAL_DEPENDENCIES**: M0 can begin; display access at M6, GPU validation tooling/access at M14 and clean distribution environments at M17 remain explicit future gates. Optional M18 fixtures never block release. [Readiness report](../../readiness/cnc-generals-zero-hour/readiness-report.md) and [prerequisite manifest](../../readiness/cnc-generals-zero-hour/prerequisite-manifest.md) record providers, checks and clean-execution simulation. Planning readiness is not implementation acceptance; all milestones remain pending in [status.yaml](status.yaml). [traceability.md](traceability.md) distinguishes obligations, constraints, evidence and exclusions. The parent planning transaction must finalize its commit provenance before downstream handoff.
