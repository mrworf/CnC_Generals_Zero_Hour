# Milestone M8: UI, fonts, and input integration

## Objective

Deliver UI, fonts, and input integration as the source-defined M8 outcome. This contract compiles the accepted native Linux migration; it does not mark implementation complete.

## User/System Outcome

menu flows are navigable using the null/recording renderer, the locale decision is closed with tests, every UI shader compiles, and recorded commands cover all selected-corpus UI/font paths. Pixel appearance remains a M14 concern.

## Scope

- port 2D/UI draw generation, clipping, blending, half-pixel behavior, cursor layers, and menu/loading-screen transitions to renderer commands;
- integrate FreeType memory faces and Fontconfig fallbacks, then decide whether the selected locale requires HarfBuzz and/or FriBidi;
- connect SDL physical input and text composition to existing UI controls; and
- cover menu, tooltip, subtitle, save/player names, chat edit, wrapping, truncation, fallback glyph, focus, and resize command streams.

## Explicit Exclusions

UI redesign, universal locale support and pixel acceptance are excluded.

Only Zero Hour in GeneralsMD is ported. Base Generals executable, authoring tools, DRM, obsolete online services, unrelated engine rewrites, non-x86-64 support and retail redistribution remain excluded.

## Source Requirements

- [Authoritative port plan](../../../docs/zero-hour-linux-port-plan.md), §8 “M8 — UI, fonts, and input integration”.
- Governing sections: §6 Fonts and locale / LAN and removed online services; §9 Supplied locale at M8; §10 Input/text.
- Shared §1 scope, §2 preservation rules, §3 selected stack, §7 dependency/check contract and §10 validation matrix apply. Source revision and commit are recorded in status.yaml; this contract cannot supersede that authority.

## Preconditions

M4, M6, and M7.

Direct implementation dependencies: [M4](M4-retail-vfs.md), [M6](M6-sdl-platform.md), [M7](M7-renderer-core.md).

If a precondition fails, retain completed evidence and stop only this milestone and dependants; do not claim acceptance from a partial demonstration.

- `PRE-009` — verified VFS and logical English corpus manifest; provider M4; require completed acceptance evidence before entry (currently pending implementation).
- `PRE-019` — SDL platform/input with interactive smoke acceptance; provider M6; require completed acceptance evidence before entry (currently pending implementation).
- `PRE-010` — validated recording device and normalized resource/command contracts; provider M7; require completed acceptance evidence before entry (currently pending implementation).

## Readiness checks

- Inspect status.yaml and linked acceptance evidence for M4, M6, M7 — each direct provider must have completed its contract; pending implementation is not completed evidence.
- After M0 exists, run `cmake --list-presets` — all four source-defined presets are listed. Inspect provider test/evidence records and use their documented checks if freshness is in doubt; do not require this milestone's unfinished features at entry.

Produces `PRE-020` (UI/font/text command generation and locale decision) as part of this milestone's acceptance, not as entry requirements.

Readiness audit: [prerequisite manifest](../../readiness/cnc-generals-zero-hour/prerequisite-manifest.md). Unmet providers stop this milestone and dependants only; all implementation statuses remain pending.

## Functional Requirements

Implement the scoped work above using the concrete interface contract below.

Complete every scoped work item and its source exit together; preserve existing simulation, formats and engine-facing behavior except the explicitly replaced platform edges.

## Architecture / Security Constraints

Retain clipping, blend, half-pixel placement, cursor layers, atlas metrics/baseline/bearing/advance/overlap/hinting/alpha; log selected face/fallback and bound missing glyph access. English locale gate still requires tested layout; add HarfBuzz or FriBidi only on §9 evidence.

Treat retail files as read-only, including the ignored original-game symlink. Commit only project-owned source, synthetic fixtures and permitted logical/derived evidence. Ordinary tests must run without private media, interactive devices or a network fetch.

## Interfaces and Compatibility

Existing UI controls consume SDL input and renderer commands. Read UnicodeFontName/LocalFontFile; load VFS fonts with FT_New_Memory_Face and retain bytes through face lifetime. Fontconfig resolves system fallback only when no supplied font file is chosen.

Use the canonical x86-64 presets `linux-gcc-debug`, `linux-clang-debug`, `linux-gcc-release` and `linux-clang-release`. Shared primitive codecs and public engine interfaces remain compatibility boundaries. Changes outside this contract require source-authorized evidence, not incidental cleanup.

## UX Constraints

Preserve existing retail interaction and selected English locale behavior described in §6 and the scoped requirements above. Show actionable subsystem/path/asset errors and preserve safe shutdown/recovery. Do not redesign navigation or add service dependencies. Unavailable Internet actions are removed or show a local non-blocking explanation; no network attempt or waiting UI is allowed.

## Acceptance Criteria

- [ ] menu flows are navigable using the null/recording renderer, the locale decision is closed with tests, every UI shader compiles, and recorded commands cover all selected-corpus UI/font paths. Pixel appearance remains a M14 concern.
- [ ] Every scoped behavior and interface above has positive evidence; the listed negative/boundary cases fail with actionable diagnostics.
- [ ] The required validation below passes; deferred work is not used to mask an unfinished path.
- [ ] Evidence records compiler/build or device/corpus context as applicable without private media or absolute private paths.

## Required Validation

Run `cmake --build --preset <preset>` and `ctest --preset <preset> -L 'platform|renderer-contract|ui' --output-on-failure` for applicable supported presets.

Navigate menus/loading using recorder; test tooltip/subtitle/caption, save/player/game name and chat editing, composition, focus/resize, truncation/wrapping, mixed ASCII/non-ASCII, missing glyph and font fallback. Compile all UI shaders and record representative command streams.

Retail checks use a separate build directory with `ZH_ENABLE_RETAIL_TESTS=ON` and local `ZH_RETAIL_ZH_DATA`, `ZH_RETAIL_GENERALS_DATA`, `ZH_RETAIL_LANGUAGE`. Hardware jobs additionally enable `ZH_ENABLE_GPU_TESTS=ON`; defaults stay OFF. Do not claim future commands have run when compiling this packet.

## Known Risks / Deferred Work

No redesign or universal-locale promise. Internet entries removed or give one local nonblocking unavailable explanation, never DNS/retired-host connection/waiting UI. Pixel checks M14.

Source §9 conditional gates remain evidence-driven; newly demonstrated blockers are recorded at the consuming milestone.
