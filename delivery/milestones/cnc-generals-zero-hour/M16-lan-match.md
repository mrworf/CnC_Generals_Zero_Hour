# Milestone M16: local multi-instance Linux LAN

## Objective

Deliver local multi-instance Linux LAN as the source-defined M16 outcome. This contract compiles the accepted native Linux migration; it does not mark implementation complete.

## User/System Outcome

two processes on one Linux machine complete a LAN match with matching CRCs and clean shutdown. Physical-subnet broadcast and mixed Windows/Linux LAN are documented as unverified unless separately tested; neither blocks release.

## Scope

- launch two local windowed or headless instances with isolated writable state and `127.0.0.2`/`127.0.0.3` identities;
- validate discovery where supported, direct connection, lobby/join, localized names/chat, map negotiation/transfer, ready/start, synchronized gameplay, pause/focus/minimize, completion, disconnect, and desync reporting;
- exercise virtual loss/reorder/duplication and malformed/version/data/map mismatch cases; and
- compare simulation CRC checkpoints between the local peers.

## Explicit Exclusions

Mixed Windows/Linux and mandatory physical multi-host certification are excluded.

Only Zero Hour in GeneralsMD is ported. Base Generals executable, authoring tools, DRM, obsolete online services, unrelated engine rewrites, non-x86-64 support and retail redistribution remain excluded.

## Source Requirements

- [Authoritative port plan](../../../docs/zero-hour-linux-port-plan.md), §8 “M16 — local multi-instance Linux LAN”.
- Governing sections: §6 LAN and removed online services; §9 Loopback discovery at M11/M16; §10 LAN.
- Shared §1 scope, §2 preservation rules, §3 selected stack, §7 dependency/check contract and §10 validation matrix apply. Source revision and commit are recorded in status.yaml; this contract cannot supersede that authority.

## Preconditions

M8, M11, and M15.

Direct implementation dependencies: [M8](M8-ui-fonts.md), [M11](M11-lan-transport.md), [M15](M15-single-player.md).

If a precondition fails, retain completed evidence and stop only this milestone and dependants; do not claim acceptance from a partial demonstration.

- `PRE-020` — UI/font/text command generation and locale decision; provider M8; require completed acceptance evidence before entry (currently pending implementation).
- `PRE-014` — POSIX local peers, identity overrides and virtual transport; provider M11; require completed acceptance evidence before entry (currently pending implementation).
- `PRE-025` — complete playable single-player acceptance; provider M15; require completed acceptance evidence before entry (currently pending implementation).

## Readiness checks

- Inspect status.yaml and linked acceptance evidence for M8, M11, M15 — each direct provider must have completed its contract; pending implementation is not completed evidence.
- After M0 exists, run `cmake --list-presets` — all four source-defined presets are listed. Inspect provider test/evidence records and use their documented checks if freshness is in doubt; do not require this milestone's unfinished features at entry.

Produces `PRE-026` (complete two-process LAN match acceptance) as part of this milestone's acceptance, not as entry requirements.

Readiness audit: [prerequisite manifest](../../readiness/cnc-generals-zero-hour/prerequisite-manifest.md). Unmet providers stop this milestone and dependants only; all implementation statuses remain pending.

## Functional Requirements

Implement the scoped work above using the concrete interface contract below.

Complete every scoped work item and its source exit together; preserve existing simulation, formats and engine-facing behavior except the explicitly replaced platform edges.

## Architecture / Security Constraints

Use M11 fixed-width transport and established simulation CRC contract. Keep processes independent; no mandatory second host, Windows client or physical broadcast.

Treat retail files as read-only, including the ignored original-game symlink. Commit only project-owned source, synthetic fixtures and permitted logical/derived evidence. Ordinary tests must run without private media, interactive devices or a network fetch.

## Interfaces and Compatibility

Two local instances use isolated writable roots and distinct 127.0.0.2/127.0.0.3 identities; exercise lobby, localized names/chat, map negotiation/transfer, ready/start, synchronized gameplay and completion.

Use the canonical x86-64 presets `linux-gcc-debug`, `linux-clang-debug`, `linux-gcc-release` and `linux-clang-release`. Shared primitive codecs and public engine interfaces remain compatibility boundaries. Changes outside this contract require source-authorized evidence, not incidental cleanup.

## UX Constraints

Preserve existing retail interaction and selected English locale behavior described in §6 and the scoped requirements above. Show actionable subsystem/path/asset errors and preserve safe shutdown/recovery. Do not redesign navigation or add service dependencies.

## Acceptance Criteria

- [ ] two processes on one Linux machine complete a LAN match with matching CRCs and clean shutdown. Physical-subnet broadcast and mixed Windows/Linux LAN are documented as unverified unless separately tested; neither blocks release.
- [ ] Every scoped behavior and interface above has positive evidence; the listed negative/boundary cases fail with actionable diagnostics.
- [ ] The required validation below passes; deferred work is not used to mask an unfinished path.
- [ ] Evidence records compiler/build or device/corpus context as applicable without private media or absolute private paths.

## Required Validation

Run `cmake --build --preset <preset>` and `ctest --preset <preset> -L 'lan|headless' --output-on-failure` for applicable supported presets.

Complete two-process match and compare checkpoint CRCs; exercise discovery if supported plus direct-connect, pause/focus/minimize, disconnect, desync reporting and virtual loss/reorder/duplicate/malformed/version/data/map mismatches.

Retail checks use a separate build directory with `ZH_ENABLE_RETAIL_TESTS=ON` and local `ZH_RETAIL_ZH_DATA`, `ZH_RETAIL_GENERALS_DATA`, `ZH_RETAIL_LANGUAGE`. Hardware jobs additionally enable `ZH_ENABLE_GPU_TESTS=ON`; defaults stay OFF. Do not claim future commands have run when compiling this packet.

## Known Risks / Deferred Work

When real loopback discovery unavailable, virtual discovery and actual direct-connect remain mandatory; document physical broadcast/mixed-OS LAN unverified. Internet service replacements excluded.

Source §9 conditional gates remain evidence-driven; newly demonstrated blockers are recorded at the consuming milestone.
