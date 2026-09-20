# Milestone M7: renderer core contracts with `RecordingGpuDevice`

## Objective

Deliver renderer core contracts with `RecordingGpuDevice` as the source-defined M7 outcome. This contract compiles the accepted native Linux migration; it does not mark implementation complete.

## User/System Outcome

PRE-010 is complete. `renderer-contract` tests prove correct command/resource behavior from the corpus and synthetic fixtures with no display or GPU, and no engine caller reaches SDL_GPU or Vulkan directly.

## Scope

- implement `RecordingGpuDevice` plus opaque buffers, textures, samplers, shaders, pipelines, render targets, and handles;
- implement resource creation/destruction, dynamic uploads, DDS/TGA parsing, BC fallback conversion, render-pass rules, pipeline caching, and resize/recreation command generation;
- normalize unstable IDs in snapshots while retaining state ordering and error provenance; and
- add negative tests for stale handles, missing bindings, bad ranges, illegal pass nesting, unsupported descriptors, cache explosion, and resource use after destruction.

## Explicit Exclusions

Hardware backend and rasterized pixel acceptance are M14.

Only Zero Hour in GeneralsMD is ported. Base Generals executable, authoring tools, DRM, obsolete online services, unrelated engine rewrites, non-x86-64 support and retail redistribution remain excluded.

## Source Requirements

- [Authoritative port plan](../../../docs/zero-hour-linux-port-plan.md), §8 “M7 — renderer core contracts with `RecordingGpuDevice`”.
- Governing sections: §6 GPU-independent renderer closure / Renderer implementation; PRE-010.
- Shared §1 scope, §2 preservation rules, §3 selected stack, §7 dependency/check contract and §10 validation matrix apply. Source revision and commit are recorded in status.yaml; this contract cannot supersede that authority.

## Preconditions

M2, M3, and M4. M7 produces PRE-010 from the device-interface design closed in M2.

Direct implementation dependencies: [M2](M2-renderer-closure.md), [M3](M3-headless-startup.md), [M4](M4-retail-vfs.md).

If a precondition fails, retain completed evidence and stop only this milestone and dependants; do not claim acceptance from a partial demonstration.

- `PRE-006` — complete renderer API/state/shader mapping; provider M2; require completed acceptance evidence before entry (currently pending implementation).
- `PRE-007` — controlled headless execution and null devices; provider M3; require completed acceptance evidence before entry (currently pending implementation).
- `PRE-009` — verified VFS and logical English corpus manifest; provider M4; require completed acceptance evidence before entry (currently pending implementation).

## Readiness checks

- Inspect status.yaml and linked acceptance evidence for M2, M3, M4 — each direct provider must have completed its contract; pending implementation is not completed evidence.
- After M0 exists, run `cmake --list-presets` — all four source-defined presets are listed. Inspect provider test/evidence records and use their documented checks if freshness is in doubt; do not require this milestone's unfinished features at entry.

Produces `PRE-010` (validated recording device and normalized resource/command contracts) as part of this milestone's acceptance, not as entry requirements.

Readiness audit: [prerequisite manifest](../../readiness/cnc-generals-zero-hour/prerequisite-manifest.md). Unmet providers stop this milestone and dependants only; all implementation statuses remain pending.

## Functional Requirements

Implement the scoped work above using the concrete interface contract below.

Complete every scoped work item and its source exit together; preserve existing simulation, formats and engine-facing behavior except the explicitly replaced platform edges.

## Architecture / Security Constraints

State-cache DX8Wrapper facade, immutable bounded pipeline cache, fixed-width DDS plus existing TGA; preserve DXT2/4 premultiplied alpha and BC CPU RGBA8 fallback. Engine callers cannot call SDL_GPU/Vulkan directly.

Treat retail files as read-only, including the ignored original-game symlink. Commit only project-owned source, synthetic fixtures and permitted logical/derived evidence. Ordinary tests must run without private media, interactive devices or a network fetch.

## Interfaces and Compatibility

Implement opaque resource handles and RecordingGpuDevice normalized descriptors, resource lifetimes, uploads, pipeline keys, passes, draw ordering, resize transitions and logical error labels behind M2's shared interface.

Use the canonical x86-64 presets `linux-gcc-debug`, `linux-clang-debug`, `linux-gcc-release` and `linux-clang-release`. Shared primitive codecs and public engine interfaces remain compatibility boundaries. Changes outside this contract require source-authorized evidence, not incidental cleanup.

## Acceptance Criteria

- [ ] PRE-010 is complete. `renderer-contract` tests prove correct command/resource behavior from the corpus and synthetic fixtures with no display or GPU, and no engine caller reaches SDL_GPU or Vulkan directly.
- [ ] Every scoped behavior and interface above has positive evidence; the listed negative/boundary cases fail with actionable diagnostics.
- [ ] The required validation below passes; deferred work is not used to mask an unfinished path.
- [ ] Evidence records compiler/build or device/corpus context as applicable without private media or absolute private paths.

## Required Validation

Run `cmake --build --preset <preset>` and `ctest --preset <preset> -L 'renderer-contract' --output-on-failure` for applicable supported presets.

Exercise valid resource streams and stale/destroyed handles, missing bindings, bad ranges, illegal pass nesting, unsupported descriptors, resize ordering and cache explosion. Keep stable snapshots by normalizing IDs without losing state order or provenance.

Retail checks use a separate build directory with `ZH_ENABLE_RETAIL_TESTS=ON` and local `ZH_RETAIL_ZH_DATA`, `ZH_RETAIL_GENERALS_DATA`, `ZH_RETAIL_LANGUAGE`. Hardware jobs additionally enable `ZH_ENABLE_GPU_TESTS=ON`; defaults stay OFF. Do not claim future commands have run when compiling this packet.

## Known Risks / Deferred Work

Recorder validates commands, never pixels. Actual device implementation and validation M14; CPU resource copies only when deterministic regeneration/assets cannot recreate them.

Source §9 conditional gates remain evidence-driven; newly demonstrated blockers are recorded at the consuming milestone.
