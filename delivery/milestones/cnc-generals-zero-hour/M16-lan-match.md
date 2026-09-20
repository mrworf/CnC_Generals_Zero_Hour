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

- [Migration supplement](../../../docs/zero-hour-source-engine-migration.md): SE-008, SE-009, SE-010. Evidence-grade and anti-proxy rules apply.

- [Authoritative port plan](../../../docs/zero-hour-linux-port-plan.md), §8 “M16 — local multi-instance Linux LAN”.
- Governing sections: §6 LAN and removed online services; §9 Loopback discovery at M11/M16; §10 LAN.
- Shared §1 scope, §2 preservation rules, §3 selected stack, §7 dependency/check contract and §10 validation matrix apply. Source revision and commit are recorded in status.yaml; this contract cannot supersede that authority.

## Preconditions

PRE-034/PRE-025 — accepted M25 original lockstep and M15 gameplay; PRE-008 at actual-map checks; isolated local loopback processes as documented by M11. Produces PRE-026.

Direct implementation dependencies: [M25](M25-original-network.md), [M15](M15-single-player.md).

M25 supplies original network/lockstep assurance; M11 transport tests alone cannot admit this milestone.

Historical component completion is not original-source acceptance. Preserve prior evidence and existing M15 scaffolding; do not reuse their pass counts as proof of this contract. Existing external retail/device/distro/optional-fixture requirements remain at their consuming validation checks.

## Readiness checks

Inspect status.yaml and linked original-source acceptance for M25, M15; all direct provider contracts must be complete. Verify current authoritative source revisions and accepted packet before creating a new milestone plan. The old M15 slice plan does not cover the new provider migration.

[Readiness manifest](../../readiness/cnc-generals-zero-hour/prerequisite-manifest.md) owns external-input auditing. Unmet providers stop only consumers; planning does not claim implementation completion.

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

Record compile/link identity and source-owned runtime witnesses alongside observable outcomes. Removing a required original implementation, using missing/malformed actual assets, corrupting original save/replay state or introducing original peer-state mismatch must fail the relevant gate. Component/fixture evidence cannot substitute for original-source integration or retail runtime. Map historical unproven obligations to these real-source tests.

Run `cmake --build --preset <preset>` and `ctest --preset <preset> -L 'lan|headless' --output-on-failure` for applicable supported presets.

Complete two-process match and compare checkpoint CRCs; exercise discovery if supported plus direct-connect, pause/focus/minimize, disconnect, desync reporting and virtual loss/reorder/duplicate/malformed/version/data/map mismatches.

Retail checks use a separate build directory with `ZH_ENABLE_RETAIL_TESTS=ON` and local `ZH_RETAIL_ZH_DATA`, `ZH_RETAIL_GENERALS_DATA`, `ZH_RETAIL_LANGUAGE`. Hardware jobs additionally enable `ZH_ENABLE_GPU_TESTS=ON`; defaults stay OFF. Do not claim future commands have run when compiling this packet.

## Known Risks / Deferred Work

When real loopback discovery unavailable, virtual discovery and actual direct-connect remain mandatory; document physical broadcast/mixed-OS LAN unverified. Internet service replacements excluded.

Source §9 conditional gates remain evidence-driven; newly demonstrated blockers are recorded at the consuming milestone.
