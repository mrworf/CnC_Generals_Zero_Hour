# Milestone M2: GPU-independent renderer API closure

## Objective

Deliver GPU-independent renderer API closure as the source-defined M2 outcome. This contract compiles the accepted native Linux migration; it does not mark implementation complete.

## User/System Outcome

PRE-006 is complete, the mapping table has no unexplained entries, public engine-facing headers no longer need new D3D-shaped dependencies, all listed shaders compile, and SDL_GPU is recorded as the provisional backend. Actual device behavior remains unverified until M14.

## Scope

- inventory D3D/D3DX types, direct-device calls, formats, fixed-function states, FVF layouts, primitives, shader assembly, render targets, lost-device hooks, and WWShade paths;
- map every entry to SDL_GPU public APIs, repository GLSL, CPU format conversion, or an explicit unsupported decision;
- define engine-owned renderer descriptors and the internal device interface shared by the recorder and real backend;
- define uniform packing, resource limits, pipeline-key composition, shader/effect lookup, coordinate/depth/color conventions, and resize state transitions; and
- compile representative and table-generated GLSL to SPIR-V offline.

## Explicit Exclusions

Hardware device/pixel/performance acceptance is M14.

Only Zero Hour in GeneralsMD is ported. Base Generals executable, authoring tools, DRM, obsolete online services, unrelated engine rewrites, non-x86-64 support and retail redistribution remain excluded.

## Source Requirements

- [Authoritative port plan](../../../docs/zero-hour-linux-port-plan.md), §8 “M2 — GPU-independent renderer API closure”.
- Governing sections: §6 GPU-independent renderer closure / Renderer implementation; §9 Renderer backend at M14; PRE-006.
- Shared §1 scope, §2 preservation rules, §3 selected stack, §7 dependency/check contract and §10 validation matrix apply. Source revision and commit are recorded in status.yaml; this contract cannot supersede that authority.

## Preconditions

M0; no display, Vulkan loader, GPU, or retail data is required.

Direct implementation dependencies: [M0](M0-build-graph.md).

If a precondition fails, retain completed evidence and stop only this milestone and dependants; do not claim acceptance from a partial demonstration.

- `PRE-003` — audited source/dependency manifests; provider M0; require completed acceptance evidence before entry (currently pending implementation).
- `PRE-004` — offline CMake presets, probes, smoke tests and shared CI instructions; provider M0; require completed acceptance evidence before entry (currently pending implementation).

## Readiness checks

- Inspect status.yaml and linked acceptance evidence for M0 — each direct provider must have completed its contract; pending implementation is not completed evidence.
- After M0 exists, run `cmake --list-presets` — all four source-defined presets are listed. Inspect provider test/evidence records and use their documented checks if freshness is in doubt; do not require this milestone's unfinished features at entry.

Produces `PRE-006` (complete renderer API/state/shader mapping) as part of this milestone's acceptance, not as entry requirements.

Readiness audit: [prerequisite manifest](../../readiness/cnc-generals-zero-hour/prerequisite-manifest.md). Unmet providers stop this milestone and dependants only; all implementation statuses remain pending.

## Functional Requirements

Implement the scoped work above using the concrete interface contract below.

Complete every scoped work item and its source exit together; preserve existing simulation, formats and engine-facing behavior except the explicitly replaced platform edges.

## Architecture / Security Constraints

Map every direct D3D escape, primitive/point-size path, render target/depth format, source-used 2D/cube/3D texture, BC1/2/3 fallback, upload and fixed-function/WWShade combination. No engine Vulkan escape hatch; no GPU/display required.

Treat retail files as read-only, including the ignored original-game symlink. Commit only project-owned source, synthetic fixtures and permitted logical/derived evidence. Ordinary tests must run without private media, interactive devices or a network fetch.

## Interfaces and Compatibility

Publish engine-owned descriptors, uniform packing (at most four uniform buffers per shader stage), immutable pipeline keys and shared internal device interface; create legacy state/shader/effect mapping table.

Use the canonical x86-64 presets `linux-gcc-debug`, `linux-clang-debug`, `linux-gcc-release` and `linux-clang-release`. Shared primitive codecs and public engine interfaces remain compatibility boundaries. Changes outside this contract require source-authorized evidence, not incidental cleanup.

## Acceptance Criteria

- [ ] PRE-006 is complete, the mapping table has no unexplained entries, public engine-facing headers no longer need new D3D-shaped dependencies, all listed shaders compile, and SDL_GPU is recorded as the provisional backend. Actual device behavior remains unverified until M14.
- [ ] Every scoped behavior and interface above has positive evidence; the listed negative/boundary cases fail with actionable diagnostics.
- [ ] The required validation below passes; deferred work is not used to mask an unfinished path.
- [ ] Evidence records compiler/build or device/corpus context as applicable without private media or absolute private paths.

## Required Validation

Run `cmake --build --preset <preset>` and `ctest --preset <preset> -L 'renderer-contract' --output-on-failure` for applicable supported presets.

Check complete inventory classifications, descriptor resource limits, pipeline/shader table coverage and offline GLSL compilation for UI, terrain, water, points and WWShade; unsupported required behavior cannot disappear behind an unexplained rejection.

Retail checks use a separate build directory with `ZH_ENABLE_RETAIL_TESTS=ON` and local `ZH_RETAIL_ZH_DATA`, `ZH_RETAIL_GENERALS_DATA`, `ZH_RETAIL_LANGUAGE`. Hardware jobs additionally enable `ZH_ENABLE_GPU_TESTS=ON`; defaults stay OFF. Do not claim future commands have run when compiling this packet.

## Known Risks / Deferred Work

SDL_GPU selection remains provisional; only M14 validates pixels/device synchronization. Demonstrated gaps follow §9 and reopen affected renderer contracts.

Source §9 conditional gates remain evidence-driven; newly demonstrated blockers are recorded at the consuming milestone.
