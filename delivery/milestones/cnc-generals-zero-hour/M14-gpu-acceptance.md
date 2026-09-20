# Milestone M14: first real-GPU integration and visual acceptance

## Objective

Deliver first real-GPU integration and visual acceptance as the source-defined M14 outcome. This contract compiles the accepted native Linux migration; it does not mark implementation complete.

## User/System Outcome

PRE-013 is complete on one real x86-64 Vulkan GPU. Required scenes render acceptably, device lifecycle tests pass, no validation errors remain, and performance is sufficient for functional acceptance. A second GPU or driver family is useful evidence but not a gate. If a public SDL_GPU capability is genuinely insufficient, M14 fails and triggers the backend decision in section 9.

## Scope

- implement/connect the SDL_GPU device using the M2 interface and exercise resource, pass, upload, pipeline, resize, loss/recreation, and presentation paths with validation enabled;
- run representative UI, fonts, terrain, each faction, water, particles, shadows, roads/decals, trees, stealth, WWShade, movies, focus, resize, and fullscreen cases;
- capture project-owned reference screenshots or metrics without committing retail assets, and review coordinate, depth, winding, half-pixel, color, alpha, fog, bias, texture-origin, and A/V behavior; and
- record device/driver/SDL versions, capability queries, validation output, scene coverage, deviations, and dispositions.

## Explicit Exclusions

Ungated backend replacement and mandatory multi-GPU certification are excluded.

Only Zero Hour in GeneralsMD is ported. Base Generals executable, authoring tools, DRM, obsolete online services, unrelated engine rewrites, non-x86-64 support and retail redistribution remain excluded.

## Source Requirements

- [Authoritative port plan](../../../docs/zero-hour-linux-port-plan.md), §8 “M14 — first real-GPU integration and visual acceptance”.
- Governing sections: §6 Renderer implementation; §9 Renderer backend at M14; PRE-012–013.
- Shared §1 scope, §2 preservation rules, §3 selected stack, §7 dependency/check contract and §10 validation matrix apply. Source revision and commit are recorded in status.yaml; this contract cannot supersede that authority.

## Preconditions

M7-M10, M13, PRE-012, and a user-supplied retail corpus. This is the first milestone that requires an actual GPU.

Direct implementation dependencies: [M7](M7-renderer-core.md), [M8](M8-ui-fonts.md), [M9](M9-world-rendering.md), [M10](M10-effects.md), [M13](M13-video.md).

If a precondition fails, retain completed evidence and stop only this milestone and dependants; do not claim acceptance from a partial demonstration.

- `PRE-010` — validated recording device and normalized resource/command contracts; provider M7; require completed acceptance evidence before entry (currently pending implementation).
- `PRE-020` — UI/font/text command generation and locale decision; provider M8; require completed acceptance evidence before entry (currently pending implementation).
- `PRE-021` — world rendering command generation; provider M9; require completed acceptance evidence before entry (currently pending implementation).
- `PRE-022` — effects/WWShade command generation; provider M10; require completed acceptance evidence before entry (currently pending implementation).
- `PRE-024` — headless Bink decode and synchronized presentation commands; provider M13; require completed acceptance evidence before entry (currently pending implementation).
- `PRE-008`, `PRE-012`, `PRE-016` — user/developer supplies readable corpus, Arch RTX 4070/Vulkan access, Khronos validation layers and display; validation package is currently absent. SDL_GPU backend-open success is produced during M14, not required before its implementation.

## Readiness checks

- Inspect status.yaml and linked acceptance evidence for M7, M8, M9, M10, M13 — each direct provider must have completed its contract; pending implementation is not completed evidence.
- After M0 exists, run `cmake --list-presets` — all four source-defined presets are listed. Inspect provider test/evidence records and use their documented checks if freshness is in doubt; do not require this milestone's unfinished features at entry.
- Outside the restricted sandbox, `vulkaninfo --summary` must identify the RTX 4070 and enumerate `VK_LAYER_KHRONOS_validation`; `pacman -Q vulkan-validation-layers` must succeed. Recheck M6 interactive-session evidence; backend creation and validation-error-free scenes remain M14 acceptance.

Produces `PRE-013` (Arch real-GPU capability, lifecycle and visual acceptance record) as part of this milestone's acceptance, not as entry requirements.

Readiness audit: [prerequisite manifest](../../readiness/cnc-generals-zero-hour/prerequisite-manifest.md). Unmet providers stop this milestone and dependants only; all implementation statuses remain pending.

## Functional Requirements

Implement the scoped work above using the concrete interface contract below.

Complete every scoped work item and its source exit together; preserve existing simulation, formats and engine-facing behavior except the explicitly replaced platform edges.

## Architecture / Security Constraints

Arch x86-64 RTX 4070 is primary target. Install Khronos validation tooling before device validation; implement device then run SDL_GPU open/validation probe as M14 evidence, not a requirement for a pre-existing completed backend. No runtime requirement for validation layers/RenderDoc.

Treat retail files as read-only, including the ignored original-game symlink. Commit only project-owned source, synthetic fixtures and permitted logical/derived evidence. Ordinary tests must run without private media, interactive devices or a network fetch.

## Interfaces and Compatibility

Connect SDL_GPU Vulkan device to M2/M7 interface; record device/driver/SDL, capability queries, debug labels, validation output, scene captures/metrics and dispositions.

Use the canonical x86-64 presets `linux-gcc-debug`, `linux-clang-debug`, `linux-gcc-release` and `linux-clang-release`. Shared primitive codecs and public engine interfaces remain compatibility boundaries. Changes outside this contract require source-authorized evidence, not incidental cleanup.

## UX Constraints

Preserve existing retail interaction and selected English locale behavior described in §6 and the scoped requirements above. Show actionable subsystem/path/asset errors and preserve safe shutdown/recovery. Do not redesign navigation or add service dependencies.

## Acceptance Criteria

- [ ] PRE-013 is complete on one real x86-64 Vulkan GPU. Required scenes render acceptably, device lifecycle tests pass, no validation errors remain, and performance is sufficient for functional acceptance. A second GPU or driver family is useful evidence but not a gate. If a public SDL_GPU capability is genuinely insufficient, M14 fails and triggers the backend decision in section 9.
- [ ] Every scoped behavior and interface above has positive evidence; the listed negative/boundary cases fail with actionable diagnostics.
- [ ] The required validation below passes; deferred work is not used to mask an unfinished path.
- [ ] Evidence records compiler/build or device/corpus context as applicable without private media or absolute private paths.

## Required Validation

Run `cmake --build --preset <preset>` and `ctest --preset <preset> -L 'gpu|ui|video' --output-on-failure` for applicable supported presets.

Exercise uploads/passes/pipelines/presentation, resize/fullscreen/focus, loss/recreation and all UI/font/world/faction/water/particle/shadow/road/tree/stealth/WWShade/movie scenes. Check coordinate/depth/winding/half-pixel/color/alpha/fog/bias/origins and A/V; zero validation errors and adequate functional performance.

Retail checks use a separate build directory with `ZH_ENABLE_RETAIL_TESTS=ON` and local `ZH_RETAIL_ZH_DATA`, `ZH_RETAIL_GENERALS_DATA`, `ZH_RETAIL_LANGUAGE`. Hardware jobs additionally enable `ZH_ENABLE_GPU_TESTS=ON`; defaults stay OFF. Do not claim future commands have run when compiling this packet.

## Known Risks / Deferred Work

Demonstrated public SDL_GPU gap requires failed capability/consumer/minimal reproducer, then §9 bgfx or raw Vulkan decision and rework/retest M2/M7–M10/M14. Preference/performance/driver bug alone is not abstraction failure; one GPU suffices.

Source §9 conditional gates remain evidence-driven; newly demonstrated blockers are recorded at the consuming milestone.
