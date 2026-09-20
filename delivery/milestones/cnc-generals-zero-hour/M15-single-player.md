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

- [Migration supplement](../../../docs/zero-hour-source-engine-migration.md): SE-009, SE-010; source providers SE-001–SE-007. Evidence-grade and anti-proxy rules apply.

- [Authoritative port plan](../../../docs/zero-hour-linux-port-plan.md), §8 “M15 — playable single-player”.
- Governing sections: §1 Scope and completion criteria; §6 runtime subsystem work; §10 Simulation / Process; PRE-013.
- Shared §1 scope, §2 preservation rules, §3 selected stack, §7 dependency/check contract and §10 validation matrix apply. Source revision and commit are recorded in status.yaml; this contract cannot supersede that authority.

## Preconditions

PRE-032/PRE-033 — accepted M23 original UI/media and M24 original persistence; PRE-008/PRE-012/PRE-016 at integrated retail/device checks. Produces PRE-025.

Direct implementation dependencies: [M23](M23-original-interaction.md), [M24](M24-original-persistence.md).

Source-integrated presentation/UI/media and original save/replay acceptance must complete first. M14 synthetic GPU acceptance is retained as component evidence, not real-scene acceptance; M22 supplies the latter.

Historical component completion is not original-source acceptance. Preserve prior evidence and existing M15 scaffolding; do not reuse their pass counts as proof of this contract. Existing external retail/device/distro/optional-fixture requirements remain at their consuming validation checks.

## Readiness checks

Inspect status.yaml and linked original-source acceptance for M23, M24; all direct provider contracts must be complete. Verify current authoritative source revisions and accepted packet before creating a new milestone plan. The old M15 slice plan does not cover the new provider migration.

[Readiness manifest](../../readiness/cnc-generals-zero-hour/prerequisite-manifest.md) owns external-input auditing. Unmet providers stop only consumers; planning does not claim implementation completion.

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

Record compile/link identity and source-owned runtime witnesses alongside observable outcomes. Removing a required original implementation, using missing/malformed actual assets, corrupting original save/replay state or introducing original peer-state mismatch must fail the relevant gate. Component/fixture evidence cannot substitute for original-source integration or retail runtime. Map historical unproven obligations to these real-source tests.

Build each preset and run the full non-LAN mandatory suite; keep GPU/retail tests in explicitly provisioned builds and exclude optional compatibility fixtures.

Start from arbitrary CWD, complete representative missions and all-faction skirmishes, repeated load/unload and long games; verify English text/speech/music/effects/movies, Linux persistence/replay, controlled CRC consistency and ASan/UBSan.

Retail checks use a separate build directory with `ZH_ENABLE_RETAIL_TESTS=ON` and local `ZH_RETAIL_ZH_DATA`, `ZH_RETAIL_GENERALS_DATA`, `ZH_RETAIL_LANGUAGE`. Hardware jobs additionally enable `ZH_ENABLE_GPU_TESTS=ON`; defaults stay OFF. Do not claim future commands have run when compiling this packet.

## Known Risks / Deferred Work

Windows artifacts/network peers are not prerequisites; LAN complete-match acceptance M16. Original-scene hardware gate M22 cannot be bypassed; M14 remains supporting component evidence.

Source §9 conditional gates remain evidence-driven; newly demonstrated blockers are recorded at the consuming milestone.
