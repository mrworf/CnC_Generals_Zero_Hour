# Milestone M15: playable single-player

## Objective

Deliver playable single-player as the source-defined M15 outcome. This contract compiles the accepted native Linux migration; it does not mark implementation complete.

## User/System Outcome

the game can start from arbitrary CWD, validate owned data, complete representative campaign and skirmish sessions, save/load and replay Linux-created state, and exit cleanly. No Windows artifact or network peer is required.

## Scope

- connect all completed subsystems into campaign, skirmish, loading, pause/options, user-map, save/load, replay, victory/defeat, movie, and shutdown flows;
- complete progression/state directories, error recovery, long-game and repeated load/unload testing;
- validate selected-locale text, speech, movies, effects, music, UI, maps, all factions, and representative missions; and
- run x86-64 GCC/Clang Debug/Release determinism checks plus ASan/UBSan suites.

## Explicit Exclusions

LAN complete matches M16 and optional Windows import M18 are excluded.

Only Zero Hour in GeneralsMD is ported. Base Generals executable, authoring tools, DRM, obsolete online services, unrelated engine rewrites, non-x86-64 support and retail redistribution remain excluded.

## Source Requirements

- [Authoritative port plan](../../../docs/zero-hour-linux-port-plan.md), §8 “M15 — playable single-player”.
- Governing sections: §1 Scope and completion criteria; §6 runtime subsystem work; §10 Simulation / Process; PRE-013.
- Shared §1 scope, §2 preservation rules, §3 selected stack, §7 dependency/check contract and §10 validation matrix apply. Source revision and commit are recorded in status.yaml; this contract cannot supersede that authority.

## Preconditions

M5, M8-M10, M12-M14.

Direct implementation dependencies: [M5](M5-persistence.md), [M8](M8-ui-fonts.md), [M9](M9-world-rendering.md), [M10](M10-effects.md), [M12](M12-audio.md), [M13](M13-video.md), [M14](M14-gpu-acceptance.md).

If a precondition fails, retain completed evidence and stop only this milestone and dependants; do not claim acceptance from a partial demonstration.

- `PRE-018` — Linux persistence and cross-compiler deterministic replay; provider M5; require completed acceptance evidence before entry (currently pending implementation).
- `PRE-020` — UI/font/text command generation and locale decision; provider M8; require completed acceptance evidence before entry (currently pending implementation).
- `PRE-021` — world rendering command generation; provider M9; require completed acceptance evidence before entry (currently pending implementation).
- `PRE-022` — effects/WWShade command generation; provider M10; require completed acceptance evidence before entry (currently pending implementation).
- `PRE-011` — reviewed, URL/hash/license-pinned miniaudio source; provider M12; require completed acceptance evidence before entry (currently pending implementation).
- `PRE-023` — VFS-backed audio and null sink; provider M12; require completed acceptance evidence before entry (currently pending implementation).
- `PRE-024` — headless Bink decode and synchronized presentation commands; provider M13; require completed acceptance evidence before entry (currently pending implementation).
- `PRE-013` — Arch real-GPU capability, lifecycle and visual acceptance record; provider M14; require completed acceptance evidence before entry (currently pending implementation).

## Readiness checks

- Inspect status.yaml and linked acceptance evidence for M5, M8, M9, M10, M12, M13, M14 — each direct provider must have completed its contract; pending implementation is not completed evidence.
- After M0 exists, run `cmake --list-presets` — all four source-defined presets are listed. Inspect provider test/evidence records and use their documented checks if freshness is in doubt; do not require this milestone's unfinished features at entry.

Produces `PRE-025` (complete playable single-player acceptance) as part of this milestone's acceptance, not as entry requirements.

Readiness audit: [prerequisite manifest](../../readiness/cnc-generals-zero-hour/prerequisite-manifest.md). Unmet providers stop this milestone and dependants only; all implementation statuses remain pending.

## Functional Requirements

Implement the scoped work above using the concrete interface contract below.

Complete every scoped work item and its source exit together; preserve existing simulation, formats and engine-facing behavior except the explicitly replaced platform edges.

## Architecture / Security Constraints

Use explicit roots and XDG writable directories; never chdir or modify retail. Preserve gameplay/simulation/INI/UI and narrow subsystem seams. Missing/corrupt required data errors are actionable; absent audio supports silent play.

Treat retail files as read-only, including the ignored original-game symlink. Commit only project-owned source, synthetic fixtures and permitted logical/derived evidence. Ordinary tests must run without private media, interactive devices or a network fetch.

## Interfaces and Compatibility

Integrate campaign/skirmish/user-map flows with loading, pause/options, Linux save/load/replay, progression, victory/defeat, movies/audio and clean exit; retain selected-locale existing UX.

Use the canonical x86-64 presets `linux-gcc-debug`, `linux-clang-debug`, `linux-gcc-release` and `linux-clang-release`. Shared primitive codecs and public engine interfaces remain compatibility boundaries. Changes outside this contract require source-authorized evidence, not incidental cleanup.

## UX Constraints

Preserve existing retail interaction and selected English locale behavior described in §6 and the scoped requirements above. Show actionable subsystem/path/asset errors and preserve safe shutdown/recovery. Do not redesign navigation or add service dependencies.

## Acceptance Criteria

- [ ] the game can start from arbitrary CWD, validate owned data, complete representative campaign and skirmish sessions, save/load and replay Linux-created state, and exit cleanly. No Windows artifact or network peer is required.
- [ ] Every scoped behavior and interface above has positive evidence; the listed negative/boundary cases fail with actionable diagnostics.
- [ ] The required validation below passes; deferred work is not used to mask an unfinished path.
- [ ] Evidence records compiler/build or device/corpus context as applicable without private media or absolute private paths.

## Required Validation

Build each preset and run the full non-LAN mandatory suite; keep GPU/retail tests in explicitly provisioned builds and exclude optional compatibility fixtures.

Start from arbitrary CWD, complete representative missions and all-faction skirmishes, repeated load/unload and long games; verify English text/speech/music/effects/movies, Linux persistence/replay, controlled CRC consistency and ASan/UBSan.

Retail checks use a separate build directory with `ZH_ENABLE_RETAIL_TESTS=ON` and local `ZH_RETAIL_ZH_DATA`, `ZH_RETAIL_GENERALS_DATA`, `ZH_RETAIL_LANGUAGE`. Hardware jobs additionally enable `ZH_ENABLE_GPU_TESTS=ON`; defaults stay OFF. Do not claim future commands have run when compiling this packet.

## Known Risks / Deferred Work

Windows artifacts/network peers are not prerequisites; LAN complete-match acceptance M16. Hardware gate M14 cannot be bypassed.

Source §9 conditional gates remain evidence-driven; newly demonstrated blockers are recorded at the consuming milestone.
