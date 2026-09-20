# Milestone M18: optional Windows save/replay compatibility

## Objective

Deliver optional Windows save/replay compatibility as the source-defined M18 outcome. This contract compiles the accepted native Linux migration; it does not mark implementation complete.

## User/System Outcome

supported fixture versions import with documented behavior, or the exact incompatibility remains documented. If fixtures never become available, status is “not validated,” not “failed,” and the Linux release remains complete.

## Scope

- import provenance-recorded Windows 1.04 save and replay fixtures, compare decoded fields and replay checkpoints, and add legal non-retail derived regression expectations;
- determine whether incompatibilities are codec defects, version differences, data/config differences, or irreproducible Windows floating-point behavior; and
- add narrow versioned compatibility handling only where evidence is sufficient and it does not regress Linux persistence/determinism.

## Explicit Exclusions

Mandatory release work and unsupported fixture compatibility claims are excluded.

Only Zero Hour in GeneralsMD is ported. Base Generals executable, authoring tools, DRM, obsolete online services, unrelated engine rewrites, non-x86-64 support and retail redistribution remain excluded.

## Source Requirements

- [Authoritative port plan](../../../docs/zero-hour-linux-port-plan.md), §8 “M18 — optional Windows save/replay compatibility”.
- Governing sections: §1 Scope and completion criteria; §5 Serialization and packets / Floating-point behavior; PRE-015.
- Shared §1 scope, §2 preservation rules, §3 selected stack, §7 dependency/check contract and §10 validation matrix apply. Source revision and commit are recorded in status.yaml; this contract cannot supersede that authority.

## Preconditions

M17 and PRE-015. This milestone is outside the release dependency chain.

Direct implementation dependencies: [M17](M17-release.md).

If a precondition fails, retain completed evidence and stop only this milestone and dependants; do not claim acceptance from a partial demonstration.

- `PRE-027` — Arch playable release and clean distribution build evidence; provider M17; require completed acceptance evidence before entry (currently pending implementation).
- `PRE-015` — optional user-supplied Windows 1.04 fixtures with legal provenance; absent inputs yield not validated without blocking the release.

## Readiness checks

- Inspect status.yaml and linked acceptance evidence for M17 — each direct provider must have completed its contract; pending implementation is not completed evidence.
- After M0 exists, run `cmake --list-presets` — all four source-defined presets are listed. Inspect provider test/evidence records and use their documented checks if freshness is in doubt; do not require this milestone's unfinished features at entry.
- Inspect fixture provenance, recorded Windows 1.04 version/configuration and local readability; if absent record not validated and leave M0–M17 unaffected.

Produces optional compatibility evidence only; no mandatory milestone consumes it.

Readiness audit: [prerequisite manifest](../../readiness/cnc-generals-zero-hour/prerequisite-manifest.md). Unmet providers stop this milestone and dependants only; all implementation statuses remain pending.

## Functional Requirements

Implement the scoped work above using the concrete interface contract below.

Complete every scoped work item and its source exit together; preserve existing simulation, formats and engine-facing behavior except the explicitly replaced platform edges.

## Architecture / Security Constraints

Narrow compatibility handling requires evidence and must preserve mandatory Linux codec/determinism tests; never commit private fixture bytes. Classify mismatch as codec/version/config/data/floating-point evidence rather than invent compatibility.

Treat retail files as read-only, including the ignored original-game symlink. Commit only project-owned source, synthetic fixtures and permitted logical/derived evidence. Ordinary tests must run without private media, interactive devices or a network fetch.

## Interfaces and Compatibility

Optionally read provenance/version-recorded Windows 1.04 save/replay fixtures through bounded versioned import paths; record exact supported versions/behavior.

Use the canonical x86-64 presets `linux-gcc-debug`, `linux-clang-debug`, `linux-gcc-release` and `linux-clang-release`. Shared primitive codecs and public engine interfaces remain compatibility boundaries. Changes outside this contract require source-authorized evidence, not incidental cleanup.

## Acceptance Criteria

- [ ] supported fixture versions import with documented behavior, or the exact incompatibility remains documented. If fixtures never become available, status is “not validated,” not “failed,” and the Linux release remains complete.
- [ ] Every scoped behavior and interface above has positive evidence; the listed negative/boundary cases fail with actionable diagnostics.
- [ ] Linux regression acceptance remains intact and the optional fixture disposition is explicit.
- [ ] Evidence records compiler/build or device/corpus context as applicable without private media or absolute private paths.

## Required Validation

Run `cmake --build --preset <preset>` and `ctest --preset <preset> -L 'compatibility' --output-on-failure` for applicable supported presets.

With fixtures, compare fields and replay checkpoints and add legal derived expectations; rerun Linux round-trip/determinism regressions and malformed/version rejection tests. Without fixtures record not validated, not failed.

Retail checks use a separate build directory with `ZH_ENABLE_RETAIL_TESTS=ON` and local `ZH_RETAIL_ZH_DATA`, `ZH_RETAIL_GENERALS_DATA`, `ZH_RETAIL_LANGUAGE`. Hardware jobs additionally enable `ZH_ENABLE_GPU_TESTS=ON`; defaults stay OFF. Do not claim future commands have run when compiling this packet.

## Known Risks / Deferred Work

Entire M18 is optional and outside M0–M17 release chain. No present fixture means no import compatibility claim; no second platform requirement imposed on release.

Source §9 conditional gates remain evidence-driven; newly demonstrated blockers are recorded at the consuming milestone.
