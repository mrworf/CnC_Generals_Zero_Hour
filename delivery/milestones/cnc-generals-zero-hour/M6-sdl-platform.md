# Milestone M6: SDL platform and input without GPU

## Objective

Deliver SDL platform and input without GPU as the source-defined M6 outcome. This contract compiles the accepted native Linux migration; it does not mark implementation complete.

## User/System Outcome

`platform` tests pass headlessly, and one interactive window test validates keyboard, mouse, text input, focus, resize, and shutdown without creating a GPU device.

## Scope

- implement SDL lifecycle, window/events, scancode mapping, mouse/cursor/clipboard, UTF-8 text input and IME editing, focus, resize, and fullscreen state;
- keep GPU-device creation behind the renderer interface and disabled for these tests;
- test event translation through injectable SDL event fixtures so headless CI covers mappings without a window; and
- provide actionable behavior when no display server exists.

## Explicit Exclusions

GPU creation and pixel acceptance are excluded.

Only Zero Hour in GeneralsMD is ported. Base Generals executable, authoring tools, DRM, obsolete online services, unrelated engine rewrites, non-x86-64 support and retail redistribution remain excluded.

## Source Requirements

- [Authoritative port plan](../../../docs/zero-hour-linux-port-plan.md), §8 “M6 — SDL platform and input without GPU”.
- Governing sections: §6 Entry point, SDL, and input; §10 Input/text / Process; PRE-016.
- Shared §1 scope, §2 preservation rules, §3 selected stack, §7 dependency/check contract and §10 validation matrix apply. Source revision and commit are recorded in status.yaml; this contract cannot supersede that authority.

## Preconditions

M1, M3, and PRE-016 for the final interactive check; headless event work does not wait for a display. No physical GPU or Vulkan device is needed.

Direct implementation dependencies: [M1](M1-portable-foundations.md), [M3](M3-headless-startup.md).

If a precondition fails, retain completed evidence and stop only this milestone and dependants; do not claim acceptance from a partial demonstration.

- `PRE-005` — portable ABI, codecs and support libraries; provider M1; require completed acceptance evidence before entry (currently pending implementation).
- `PRE-007` — controlled headless execution and null devices; provider M3; require completed acceptance evidence before entry (currently pending implementation).
- `PRE-016` — developer supplies accessible SDL display session for interactive acceptance; currently unverified from this sandbox. Injected headless work may begin first.

## Readiness checks

- Inspect status.yaml and linked acceptance evidence for M1, M3 — each direct provider must have completed its contract; pending implementation is not completed evidence.
- After M0 exists, run `cmake --list-presets` — all four source-defined presets are listed. Inspect provider test/evidence records and use their documented checks if freshness is in doubt; do not require this milestone's unfinished features at entry.
- Inspect availability of the developer's graphical session outside restricted device/display isolation. During M6, run its documented SDL window smoke and record keyboard/mouse/text/focus/resize/shutdown without GPU creation; populated display variables alone do not establish PRE-016.

Produces `PRE-019` (SDL platform/input with interactive smoke acceptance) as part of this milestone's acceptance, not as entry requirements.

Readiness audit: [prerequisite manifest](../../readiness/cnc-generals-zero-hour/prerequisite-manifest.md). Unmet providers stop this milestone and dependants only; all implementation statuses remain pending.

## Functional Requirements

Implement the scoped work above using the concrete interface contract below.

Complete every scoped work item and its source exit together; preserve existing simulation, formats and engine-facing behavior except the explicitly replaced platform edges.

## Architecture / Security Constraints

GPU creation remains behind renderer interface and disabled here. On focus loss clear held buttons/keys and relative capture while retaining mode-specific pause behavior. Unavailable display reports actionable error.

Treat retail files as read-only, including the ignored original-game symlink. Commit only project-owned source, synthetic fixtures and permitted logical/derived evidence. Ordinary tests must run without private media, interactive devices or a network fetch.

## Interfaces and Compatibility

SDL scancodes map physical controls; keycodes are layout shortcuts, text/editing events deliver UTF-8/IME composition. Preserve drag, double-click, wheel, edge scroll, confinement, cursor, clipboard and window/fullscreen state.

Use the canonical x86-64 presets `linux-gcc-debug`, `linux-clang-debug`, `linux-gcc-release` and `linux-clang-release`. Shared primitive codecs and public engine interfaces remain compatibility boundaries. Changes outside this contract require source-authorized evidence, not incidental cleanup.

## UX Constraints

Preserve existing retail interaction and selected English locale behavior described in §6 and the scoped requirements above. Show actionable subsystem/path/asset errors and preserve safe shutdown/recovery. Do not redesign navigation or add service dependencies.

## Acceptance Criteria

- [ ] `platform` tests pass headlessly, and one interactive window test validates keyboard, mouse, text input, focus, resize, and shutdown without creating a GPU device.
- [ ] Every scoped behavior and interface above has positive evidence; the listed negative/boundary cases fail with actionable diagnostics.
- [ ] The required validation below passes; deferred work is not used to mask an unfinished path.
- [ ] Evidence records compiler/build or device/corpus context as applicable without private media or absolute private paths.

## Required Validation

Run `cmake --build --preset <preset>` and `ctest --preset <preset> -L 'platform' --output-on-failure` for applicable supported presets.

Inject SDL mappings/events headlessly, including malformed text/focus transitions; interactive PRE-016 smoke covers keyboard, mouse, composition, clipboard, resize, focus, fullscreen and shutdown with no GPU device.

Retail checks use a separate build directory with `ZH_ENABLE_RETAIL_TESTS=ON` and local `ZH_RETAIL_ZH_DATA`, `ZH_RETAIL_GENERALS_DATA`, `ZH_RETAIL_LANGUAGE`. Hardware jobs additionally enable `ZH_ENABLE_GPU_TESTS=ON`; defaults stay OFF. Do not claim future commands have run when compiling this packet.

## Known Risks / Deferred Work

Interactive exit requires accessible session, although synthetic work can proceed. Pixel acceptance remains M14.

Source §9 conditional gates remain evidence-driven; newly demonstrated blockers are recorded at the consuming milestone.
