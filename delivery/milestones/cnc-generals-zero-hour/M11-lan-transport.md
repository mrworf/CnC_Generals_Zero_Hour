# Milestone M11: POSIX LAN transport and local peer harness

## Objective

Deliver POSIX LAN transport and local peer harness as the source-defined M11 outcome. This contract compiles the accepted native Linux migration; it does not mark implementation complete.

## User/System Outcome

PRE-014 is complete. `lan` tests cover protocol failures and two local peers. If loopback broadcast is unavailable, that limitation is recorded while virtual discovery and real direct-connect still pass. Neither a second host nor Windows peer is required.

## Scope

- finish the fixed-width packet codec and POSIX nonblocking UDP transport with bounded decoding, timeouts, wrap-safe clocks, version rejection, and diagnostics;
- add the three developer network overrides and distinct loopback identities;
- add a virtual datagram transport for deterministic discovery, loss, duplication, reorder, corruption, delay, and disconnect tests; and
- prove two real headless processes can advertise or direct-connect, join, negotiate data/map identity, exchange commands, and disconnect cleanly.

## Explicit Exclusions

Full LAN matches M16, Internet services and Windows peers are excluded.

Only Zero Hour in GeneralsMD is ported. Base Generals executable, authoring tools, DRM, obsolete online services, unrelated engine rewrites, non-x86-64 support and retail redistribution remain excluded.

## Source Requirements

- [Authoritative port plan](../../../docs/zero-hour-linux-port-plan.md), §8 “M11 — POSIX LAN transport and local peer harness”.
- Governing sections: §5 Serialization and packets; §6 LAN and removed online services; §9 Loopback discovery at M11/M16; PRE-014.
- Shared §1 scope, §2 preservation rules, §3 selected stack, §7 dependency/check contract and §10 validation matrix apply. Source revision and commit are recorded in status.yaml; this contract cannot supersede that authority.

## Preconditions

M3 and M4. M11 produces PRE-014.

Direct implementation dependencies: [M3](M3-headless-startup.md), [M4](M4-retail-vfs.md).

If a precondition fails, retain completed evidence and stop only this milestone and dependants; do not claim acceptance from a partial demonstration.

- `PRE-007` — controlled headless execution and null devices; provider M3; require completed acceptance evidence before entry (currently pending implementation).
- `PRE-009` — verified VFS and logical English corpus manifest; provider M4; require completed acceptance evidence before entry (currently pending implementation).

## Readiness checks

- Inspect status.yaml and linked acceptance evidence for M3, M4 — each direct provider must have completed its contract; pending implementation is not completed evidence.
- After M0 exists, run `cmake --list-presets` — all four source-defined presets are listed. Inspect provider test/evidence records and use their documented checks if freshness is in doubt; do not require this milestone's unfinished features at entry.

Produces `PRE-014` (POSIX local peers, identity overrides and virtual transport) as part of this milestone's acceptance, not as entry requirements.

Readiness audit: [prerequisite manifest](../../readiness/cnc-generals-zero-hour/prerequisite-manifest.md). Unmet providers stop this milestone and dependants only; all implementation statuses remain pending.

## Functional Requirements

Implement the scoped work above using the concrete interface contract below.

Complete every scoped work item and its source exit together; preserve existing simulation, formats and engine-facing behavior except the explicitly replaced platform edges.

## Architecture / Security Constraints

Packet type/version/length/count/strings validated before allocation/mutation; encode named fixed-width fields. Retain source-proven packet bytes, wrap-safe clocks and data/map identity checks; no required Windows peer.

Treat retail files as read-only, including the ignored original-game symlink. Commit only project-owned source, synthetic fixtures and permitted logical/derived evidence. Ordinary tests must run without private media, interactive devices or a network fetch.

## Interfaces and Compatibility

Expose developer --lan-bind-address, --lan-discovery-address and --lan-direct-connect; keep lobby 8086/game base 8088 identities and distinct 127.0.0.2/127.0.0.3 peers. POSIX UDP/nonblocking poll retains existing ordering/reliability layers.

Use the canonical x86-64 presets `linux-gcc-debug`, `linux-clang-debug`, `linux-gcc-release` and `linux-clang-release`. Shared primitive codecs and public engine interfaces remain compatibility boundaries. Changes outside this contract require source-authorized evidence, not incidental cleanup.

## Acceptance Criteria

- [ ] PRE-014 is complete. `lan` tests cover protocol failures and two local peers. If loopback broadcast is unavailable, that limitation is recorded while virtual discovery and real direct-connect still pass. Neither a second host nor Windows peer is required.
- [ ] Every scoped behavior and interface above has positive evidence; the listed negative/boundary cases fail with actionable diagnostics.
- [ ] The required validation below passes; deferred work is not used to mask an unfinished path.
- [ ] Evidence records compiler/build or device/corpus context as applicable without private media or absolute private paths.

## Required Validation

Run `cmake --build --preset <preset>` and `ctest --preset <preset> -L 'lan|headless' --output-on-failure` for applicable supported presets.

Virtual datagrams cover discovery, loss/duplicate/reorder/corrupt/delay/timeout/disconnect/version/data/map mismatch. Two real headless processes advertise or direct-connect, join, negotiate, exchange commands and shut down.

Retail checks use a separate build directory with `ZH_ENABLE_RETAIL_TESTS=ON` and local `ZH_RETAIL_ZH_DATA`, `ZH_RETAIL_GENERALS_DATA`, `ZH_RETAIL_LANGUAGE`. Hardware jobs additionally enable `ZH_ENABLE_GPU_TESTS=ON`; defaults stay OFF. Do not claim future commands have run when compiling this packet.

## Known Risks / Deferred Work

If loopback broadcast fails, record physical discovery unverified and use virtual discovery plus real direct-connect. If distinct binding fails, repair injection or use available namespaces. Full match is M16.

Source §9 conditional gates remain evidence-driven; newly demonstrated blockers are recorded at the consuming milestone.
