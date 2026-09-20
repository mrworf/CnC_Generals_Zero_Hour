# Milestone M10: effects, water, and WWShade command generation

## Objective

Deliver effects, water, and WWShade command generation as the source-defined M10 outcome. This contract compiles the accepted native Linux migration; it does not mark implementation complete.

## User/System Outcome

every effect in the corpus manifest compiles and produces a valid recorded command sequence; unsupported formats use tested CPU fallbacks or keep the milestone open.

## Scope

- include WWShade in the real build graph and translate its runtime effect mappings;
- port water, particles and point sprites, projected textures, bump/environment effects, stealth, post-effects, multipass materials, depth bias, and premultiplied-alpha paths;
- maintain a checked table from every observed legacy shader/effect to GLSL and pipeline state; and
- reject unknown required effects with the logical asset/material rather than silently falling back.

## Explicit Exclusions

World geometry M9 and hardware pixel acceptance M14 are excluded.

Only Zero Hour in GeneralsMD is ported. Base Generals executable, authoring tools, DRM, obsolete online services, unrelated engine rewrites, non-x86-64 support and retail redistribution remain excluded.

## Source Requirements

- [Authoritative port plan](../../../docs/zero-hour-linux-port-plan.md), §8 “M10 — effects, water, and WWShade command generation”.
- Governing sections: §4 Target architecture and build graph; §6 Renderer implementation; §9 Unexpected required asset formats at M4 or consuming milestone.
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

Produces `PRE-022` (effects/WWShade command generation) as part of this milestone's acceptance, not as entry requirements.

Readiness audit: [prerequisite manifest](../../readiness/cnc-generals-zero-hour/prerequisite-manifest.md). Unmet providers stop this milestone and dependants only; all implementation statuses remain pending.

## Functional Requirements

Implement the scoped work above using the concrete interface contract below.

Complete every scoped work item and its source exit together; preserve existing simulation, formats and engine-facing behavior except the explicitly replaced platform edges.

## Architecture / Security Constraints

Preserve point size, depth bias, premultiplied alpha and render-target dependencies through public backend contracts; unsupported required formats need tested CPU fallback or keep milestone open.

Treat retail files as read-only, including the ignored original-game symlink. Commit only project-owned source, synthetic fixtures and permitted logical/derived evidence. Ordinary tests must run without private media, interactive devices or a network fetch.

## Interfaces and Compatibility

Include WWShade runtime build closure and map each observed legacy effect/state to GLSL/pipeline definitions for water, particles/points, projected textures, bump/environment, stealth, post-effects and multipass.

Use the canonical x86-64 presets `linux-gcc-debug`, `linux-clang-debug`, `linux-gcc-release` and `linux-clang-release`. Shared primitive codecs and public engine interfaces remain compatibility boundaries. Changes outside this contract require source-authorized evidence, not incidental cleanup.

## Acceptance Criteria

- [ ] every effect in the corpus manifest compiles and produces a valid recorded command sequence; unsupported formats use tested CPU fallbacks or keep the milestone open.
- [ ] Every scoped behavior and interface above has positive evidence; the listed negative/boundary cases fail with actionable diagnostics.
- [ ] The required validation below passes; deferred work is not used to mask an unfinished path.
- [ ] Evidence records compiler/build or device/corpus context as applicable without private media or absolute private paths.

## Required Validation

Run `cmake --build --preset <preset>` and `ctest --preset <preset> -L 'renderer-contract' --output-on-failure` for applicable supported presets.

Compile every effect table entry and validate representative corpus/synthetic command sequences, multipass ordering, missing/unknown effect diagnostics and lifetime/cache bounds.

Retail checks use a separate build directory with `ZH_ENABLE_RETAIL_TESTS=ON` and local `ZH_RETAIL_ZH_DATA`, `ZH_RETAIL_GENERALS_DATA`, `ZH_RETAIL_LANGUAGE`. Hardware jobs additionally enable `ZH_ENABLE_GPU_TESTS=ON`; defaults stay OFF. Do not claim future commands have run when compiling this packet.

## Known Risks / Deferred Work

Unknown material/effect errors identify logical asset and state; visual equivalence awaits M14. Granny stays excluded unless normal retail gameplay proves requirement.

Source §9 conditional gates remain evidence-driven; newly demonstrated blockers are recorded at the consuming milestone.
