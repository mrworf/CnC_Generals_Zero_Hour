# Milestone M5: Linux persistence and determinism

## Objective

Deliver Linux persistence and determinism as the source-defined M5 outcome. This contract compiles the accepted native Linux migration; it does not mark implementation complete.

## User/System Outcome

new Linux saves load to equivalent state, Linux replays reproduce matching x86-64 CRC checkpoints across supported compilers/build types, malformed inputs fail before mutation/allocation abuse, and no Windows fixture is required. Windows import remains explicitly unverified until M18.

## Scope

- replace save, replay, CRC, and snapshot raw-memory serialization with versioned fixed-width fields;
- implement Linux save/load and replay write/read round trips, autosave metadata, bounded collection decoding, and clear version rejection;
- run deterministic headless scenarios repeatedly across GCC/Clang and Debug/Release on x86-64; and
- isolate simulation floating-point state from third-party libraries and diagnose every divergent checkpoint.

## Explicit Exclusions

Windows fixture import acceptance is optional M18.

Only Zero Hour in GeneralsMD is ported. Base Generals executable, authoring tools, DRM, obsolete online services, unrelated engine rewrites, non-x86-64 support and retail redistribution remain excluded.

## Source Requirements

- [Authoritative port plan](../../../docs/zero-hour-linux-port-plan.md), §8 “M5 — Linux persistence and determinism”.
- Governing sections: §5 Serialization and packets / Floating-point behavior; §10 Serialization / Numeric/determinism / Simulation.
- Shared §1 scope, §2 preservation rules, §3 selected stack, §7 dependency/check contract and §10 validation matrix apply. Source revision and commit are recorded in status.yaml; this contract cannot supersede that authority.

## Preconditions

M4.

Direct implementation dependencies: [M4](M4-retail-vfs.md).

If a precondition fails, retain completed evidence and stop only this milestone and dependants; do not claim acceptance from a partial demonstration.

- `PRE-009` — verified VFS and logical English corpus manifest; provider M4; require completed acceptance evidence before entry (currently pending implementation).

## Readiness checks

- Inspect status.yaml and linked acceptance evidence for M4 — each direct provider must have completed its contract; pending implementation is not completed evidence.
- After M0 exists, run `cmake --list-presets` — all four source-defined presets are listed. Inspect provider test/evidence records and use their documented checks if freshness is in doubt; do not require this milestone's unfinished features at entry.

Produces `PRE-018` (Linux persistence and cross-compiler deterministic replay) as part of this milestone's acceptance, not as entry requirements.

Readiness audit: [prerequisite manifest](../../readiness/cnc-generals-zero-hour/prerequisite-manifest.md). Unmet providers stop this milestone and dependants only; all implementation statuses remain pending.

## Functional Requirements

Implement the scoped work above using the concrete interface contract below.

Complete every scoped work item and its source exit together; preserve existing simulation, formats and engine-facing behavior except the explicitly replaced platform edges.

## Architecture / Security Constraints

No raw host memory, host Bool/enum/long/wchar_t or unchecked allocation. Keep existing save architecture and simulation single-threaded. Prevent media/render libraries changing simulation floating environment.

Treat retail files as read-only, including the ignored original-game symlink. Commit only project-owned source, synthetic fixtures and permitted logical/derived evidence. Ordinary tests must run without private media, interactive devices or a network fetch.

## Interfaces and Compatibility

Versioned Linux save/load/replay/CRC/snapshot fields use shared endian/UTF-16 codecs, bounded lengths and explicit field widths; preserve source-proven Win32 1.04 field encodings without promising Windows import.

Use the canonical x86-64 presets `linux-gcc-debug`, `linux-clang-debug`, `linux-gcc-release` and `linux-clang-release`. Shared primitive codecs and public engine interfaces remain compatibility boundaries. Changes outside this contract require source-authorized evidence, not incidental cleanup.

## Acceptance Criteria

- [ ] new Linux saves load to equivalent state, Linux replays reproduce matching x86-64 CRC checkpoints across supported compilers/build types, malformed inputs fail before mutation/allocation abuse, and no Windows fixture is required. Windows import remains explicitly unverified until M18.
- [ ] Every scoped behavior and interface above has positive evidence; the listed negative/boundary cases fail with actionable diagnostics.
- [ ] The required validation below passes; deferred work is not used to mask an unfinished path.
- [ ] Evidence records compiler/build or device/corpus context as applicable without private media or absolute private paths.

## Required Validation

Run `cmake --build --preset <preset>` and `ctest --preset <preset> -L 'foundation|determinism' --output-on-failure` for applicable supported presets.

Round-trip saves/autosaves and replay commands to equivalent state; compare repeated checkpoint CRCs across GCC/Clang Debug/Release. Reject wrong versions/endian/count/length/UTF-16 before mutation; diagnose divergence with scenario/config/checkpoint and detect changed rounding mode.

Retail checks use a separate build directory with `ZH_ENABLE_RETAIL_TESTS=ON` and local `ZH_RETAIL_ZH_DATA`, `ZH_RETAIL_GENERALS_DATA`, `ZH_RETAIL_LANGUAGE`. Hardware jobs additionally enable `ZH_ENABLE_GPU_TESTS=ON`; defaults stay OFF. Do not claim future commands have run when compiling this packet.

## Known Risks / Deferred Work

Windows reference files and any narrow import compatibility are optional M18; do not let fixture absence weaken Linux determinism.

Source §9 conditional gates remain evidence-driven; newly demonstrated blockers are recorded at the consuming milestone.
