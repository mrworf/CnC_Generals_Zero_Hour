# Milestone M9: world rendering command generation

## Objective

Deliver world rendering command generation as the source-defined M9 outcome. This contract compiles the accepted native Linux migration; it does not mark implementation complete.

## User/System Outcome

representative maps and faction scenes produce deterministic, validated command streams with no unmapped corpus state and no leaked renderer handles.

## Scope

- port meshes, vertex/index streaming, cameras, terrain, roads/decals, trees, shadows, factions, animation-visible geometry, selection markers, and render-target dependencies;
- translate relevant fixed-function transforms, lighting, fog, alpha test, texture stages, addressing, culling, depth, and color masks into shader/pipeline descriptors; and
- assert bounded pipeline/resource growth across map load, repeated frames, teardown, and reload.

## Explicit Exclusions

Effects M10 and hardware pixel acceptance M14 are excluded.

Only Zero Hour in GeneralsMD is ported. Base Generals executable, authoring tools, DRM, obsolete online services, unrelated engine rewrites, non-x86-64 support and retail redistribution remain excluded.

## Source Requirements

- [Authoritative port plan](../../../docs/zero-hour-linux-port-plan.md), §8 “M9 — world rendering command generation”.
- Governing sections: §6 Renderer implementation; §10 Renderer contracts.
- Shared §1 scope, §2 preservation rules, §3 selected stack, §7 dependency/check contract and §10 validation matrix apply. Source revision and commit are recorded in status.yaml; this contract cannot supersede that authority.

## Preconditions

M4 and M7.

Direct implementation dependencies: [M4](M4-retail-vfs.md), [M7](M7-renderer-core.md).

If a precondition fails, retain completed evidence and stop only this milestone and dependants; do not claim acceptance from a partial demonstration.

- `PRE-009` — verified VFS and logical English corpus manifest; provider M4; require completed acceptance evidence before entry (currently pending implementation).
- `PRE-010` — validated recording device and normalized resource/command contracts; provider M7; require completed acceptance evidence before entry (currently pending implementation).

## Readiness checks

- Inspect status.yaml and linked acceptance evidence for M4, M7 — each direct provider must have completed its contract; pending implementation is not completed evidence.
- After M0 exists, run `cmake --list-presets` — all four source-defined presets are listed. Inspect provider test/evidence records and use their documented checks if freshness is in doubt; do not require this milestone's unfinished features at entry.

Produces `PRE-021` (world rendering command generation) as part of this milestone's acceptance, not as entry requirements.

Readiness audit: [prerequisite manifest](../../readiness/cnc-generals-zero-hour/prerequisite-manifest.md). Unmet providers stop this milestone and dependants only; all implementation statuses remain pending.

## Functional Requirements

Implement the scoped work above using the concrete interface contract below.

Complete every scoped work item and its source exit together; preserve existing simulation, formats and engine-facing behavior except the explicitly replaced platform edges.

## Architecture / Security Constraints

Preserve effective fixed-function transforms, lighting, fog, alpha test, texture stages/addressing, depth, cull and color masks. Lock handedness, 0–1 depth, winding, origins and coordinate conventions with focused fixtures.

Treat retail files as read-only, including the ignored original-game symlink. Commit only project-owned source, synthetic fixtures and permitted logical/derived evidence. Ordinary tests must run without private media, interactive devices or a network fetch.

## Interfaces and Compatibility

Generate world resource/pipeline/draw commands for mesh/animation-visible geometry, terrain, cameras, roads/decals, vegetation, shadows, faction scenes, selection markers and render-target dependencies.

Use the canonical x86-64 presets `linux-gcc-debug`, `linux-clang-debug`, `linux-gcc-release` and `linux-clang-release`. Shared primitive codecs and public engine interfaces remain compatibility boundaries. Changes outside this contract require source-authorized evidence, not incidental cleanup.

## Acceptance Criteria

- [ ] representative maps and faction scenes produce deterministic, validated command streams with no unmapped corpus state and no leaked renderer handles.
- [ ] Every scoped behavior and interface above has positive evidence; the listed negative/boundary cases fail with actionable diagnostics.
- [ ] The required validation below passes; deferred work is not used to mask an unfinished path.
- [ ] Evidence records compiler/build or device/corpus context as applicable without private media or absolute private paths.

## Required Validation

Run `cmake --build --preset <preset>` and `ctest --preset <preset> -L 'renderer-contract' --output-on-failure` for applicable supported presets.

Compare representative corpus maps/factions and synthetic edge-state streams; repeated frames/map teardown/reload keep bounded pipeline count, deterministic ordering and no leaked handles. Fail for unmapped required corpus states.

Retail checks use a separate build directory with `ZH_ENABLE_RETAIL_TESTS=ON` and local `ZH_RETAIL_ZH_DATA`, `ZH_RETAIL_GENERALS_DATA`, `ZH_RETAIL_LANGUAGE`. Hardware jobs additionally enable `ZH_ENABLE_GPU_TESTS=ON`; defaults stay OFF. Do not claim future commands have run when compiling this packet.

## Known Risks / Deferred Work

No hardware/pixel acceptance here; that is M14. Do not replace W3D assets, simulation, terrain semantics or scene architecture.

Source §9 conditional gates remain evidence-driven; newly demonstrated blockers are recorded at the consuming milestone.
