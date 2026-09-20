# Milestone M3: headless engine and staged startup

## Objective

Deliver headless engine and staged startup as the source-defined M3 outcome. This contract compiles the accepted native Linux migration; it does not mark implementation complete.

## User/System Outcome

PRE-007 is complete. An asset-free headless executable starts, advances controlled ticks, reports intentionally skipped devices, and shuts down cleanly on x86-64 under both compilers.

## Scope

- replace `WinMain` with argument parsing and staged subsystem construction/destruction;
- provide null platform, renderer, audio, and video implementations selected without initializing SDL video or a GPU;
- remove launcher, DRM, registry, splash, named-mutex, browser, GameSpy, CD, SEH, and working-directory dependencies from the headless path;
- expose deterministic fixed-tick smoke/scenario execution, clean termination, logs, and exit codes; and
- verify multiple simultaneous headless processes use separate writable-state directories when instructed.

## Explicit Exclusions

Retail gameplay and interactive devices are M4 onward.

Only Zero Hour in GeneralsMD is ported. Base Generals executable, authoring tools, DRM, obsolete online services, unrelated engine rewrites, non-x86-64 support and retail redistribution remain excluded.

## Source Requirements

- [Authoritative port plan](../../../docs/zero-hour-linux-port-plan.md), §8 “M3 — headless engine and staged startup”.
- Governing sections: §6 Entry point, SDL, and input / Filesystem, data roots, and XDG paths / Time, threads, diagnostics, and compression; PRE-007.
- Shared §1 scope, §2 preservation rules, §3 selected stack, §7 dependency/check contract and §10 validation matrix apply. Source revision and commit are recorded in status.yaml; this contract cannot supersede that authority.

## Preconditions

M1.

Direct implementation dependencies: [M1](M1-portable-foundations.md).

If a precondition fails, retain completed evidence and stop only this milestone and dependants; do not claim acceptance from a partial demonstration.

- `PRE-005` — portable ABI, codecs and support libraries; provider M1; require completed acceptance evidence before entry (currently pending implementation).

## Readiness checks

- Inspect status.yaml and linked acceptance evidence for M1 — each direct provider must have completed its contract; pending implementation is not completed evidence.
- After M0 exists, run `cmake --list-presets` — all four source-defined presets are listed. Inspect provider test/evidence records and use their documented checks if freshness is in doubt; do not require this milestone's unfinished features at entry.

Produces `PRE-007` (controlled headless execution and null devices) as part of this milestone's acceptance, not as entry requirements.

Readiness audit: [prerequisite manifest](../../readiness/cnc-generals-zero-hour/prerequisite-manifest.md). Unmet providers stop this milestone and dependants only; all implementation statuses remain pending.

## Functional Requirements

Implement the scoped work above using the concrete interface contract below.

Complete every scoped work item and its source exit together; preserve existing simulation, formats and engine-facing behavior except the explicitly replaced platform edges.

## Architecture / Security Constraints

No chdir, launcher/DRM/registry/mutex/browser/GameSpy/CD dependency. Null devices must not initialize SDL video/GPU/audio. Emit revision, architecture, SDL, Bink availability, roots/locale and intentionally skipped devices in capability reports.

Treat retail files as read-only, including the ignored original-game symlink. Commit only project-owned source, synthetic fixtures and permitted logical/derived evidence. Ordinary tests must run without private media, interactive devices or a network fetch.

## Interfaces and Compatibility

Apply the concrete [shared writable-path contract](index.md#shared-writable-path-contract), including every XDG fallback and read-only retail boundary.

Replace WinMain with main(int, char **), staged argument/path/log setup and reverse-order teardown. Expose deterministic fixed-tick headless execution, explicit exit codes and isolated writable-state selection; define their concrete developer syntax in implementation documentation.

Use the canonical x86-64 presets `linux-gcc-debug`, `linux-clang-debug`, `linux-gcc-release` and `linux-clang-release`. Shared primitive codecs and public engine interfaces remain compatibility boundaries. Changes outside this contract require source-authorized evidence, not incidental cleanup.

## Acceptance Criteria

- [ ] PRE-007 is complete. An asset-free headless executable starts, advances controlled ticks, reports intentionally skipped devices, and shuts down cleanly on x86-64 under both compilers.
- [ ] Every scoped behavior and interface above has positive evidence; the listed negative/boundary cases fail with actionable diagnostics.
- [ ] The required validation below passes; deferred work is not used to mask an unfinished path.
- [ ] Evidence records compiler/build or device/corpus context as applicable without private media or absolute private paths.

## Required Validation

Run `cmake --build --preset <preset>` and `ctest --preset <preset> -L 'foundation|headless' --output-on-failure` for applicable supported presets.

Run simultaneous isolated processes, controlled ticks and repeatable termination without display/audio/GPU. Inject failure at each initialization stage and verify named subsystem/path/library error, no leaked worker and safe partial teardown.

Retail checks use a separate build directory with `ZH_ENABLE_RETAIL_TESTS=ON` and local `ZH_RETAIL_ZH_DATA`, `ZH_RETAIL_GENERALS_DATA`, `ZH_RETAIL_LANGUAGE`. Hardware jobs additionally enable `ZH_ENABLE_GPU_TESTS=ON`; defaults stay OFF. Do not claim future commands have run when compiling this packet.

## Known Risks / Deferred Work

Retail gameplay scenarios begin M4; actual windows M6. Preserve existing simulation/UI behavior and factories.

Source §9 conditional gates remain evidence-driven; newly demonstrated blockers are recorded at the consuming milestone.
