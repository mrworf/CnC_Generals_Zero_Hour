# Milestone M0: reproducible build readiness

## Objective

Deliver reproducible build readiness as the source-defined M0 outcome. This contract compiles the accepted native Linux migration; it does not mark implementation complete.

## User/System Outcome

PRE-003 and PRE-004 are complete; GCC and Clang Debug/Release presets configure from a clean checkout on x86-64, tests can be selected by label, missing packages name their package/capability, and configure performs no network access.

## Scope

- translate all relevant Zero Hour project manifests into explicit CMake source lists and classify every exclusion;
- model `zh_main`, engine/device, W3D, WWShade, support, compression, test, and null-backend targets without source globs;
- add the four native presets, dependency/version probes, `compile_commands.json`, generated build metadata, CTest labels, and offline behavior;
- document exact Arch Linux, Ubuntu 26.04, and Fedora 44 packages, including CMake, Ninja, both compilers, SDL3, `glslc`, FreeType, Fontconfig, zlib, FFmpeg development packages, and Vulkan validation tooling; record the tested Arch package snapshot; and
- add an asset-free CI/smoke target that proves the build graph without pretending the game links yet.

## Explicit Exclusions

Full game linking and runtime acceptance are M3 onward.

Only Zero Hour in GeneralsMD is ported. Base Generals executable, authoring tools, DRM, obsolete online services, unrelated engine rewrites, non-x86-64 support and retail redistribution remain excluded.

## Source Requirements

- [Authoritative port plan](../../../docs/zero-hour-linux-port-plan.md), §8 “M0 — reproducible build readiness”.
- Governing sections: §3 Selected replacement stack; §4 Target architecture and build graph; PRE-001–004.
- Shared §1 scope, §2 preservation rules, §3 selected stack, §7 dependency/check contract and §10 validation matrix apply. Source revision and commit are recorded in status.yaml; this contract cannot supersede that authority.

## Preconditions

PRE-001 and the user-installed portion of PRE-002.

Direct implementation dependencies: none; the existing source-defined M0 establishes the bootstrap.

If a precondition fails, retain completed evidence and stop only this milestone and dependants; do not claim acceptance from a partial demonstration.

- `PRE-001` — repository Zero Hour sources and legacy manifests exist; verified input.
- `PRE-002` — baseline compiler/build/development packages resolve on Arch; installed portion verified. M0 owns configure/probe evidence; missing generated presets are an output, not an entry blocker.

## Readiness checks

- `test -r GeneralsMD/Code/RTS.dsw && test -r GeneralsMD/Code/RTS.dsp` — both legacy manifests are readable.
- `command -v cmake ninja gcc g++ clang clang++ pkg-config glslc` and `pkg-config --modversion sdl3 freetype2 fontconfig zlib libavformat libavcodec libavutil libswscale libswresample` — every executable/module resolves; inspect versions against source §3/§4 minima.
- Inspect the source tree and Git status before implementation; preserve the ignored retail symlink. Do not run nonexistent presets until M0 creates them.

Produces `PRE-003` (audited source/dependency manifests); `PRE-004` (offline CMake presets, probes, smoke tests and shared CI instructions) as part of this milestone's acceptance, not as entry requirements.

Readiness audit: [prerequisite manifest](../../readiness/cnc-generals-zero-hour/prerequisite-manifest.md). Unmet providers stop this milestone and dependants only; all implementation statuses remain pending.

## Functional Requirements

Implement the scoped work above using the concrete interface contract below.

Complete every scoped work item and its source exit together; preserve existing simulation, formats and engine-facing behavior except the explicitly replaced platform edges.

## Architecture / Security Constraints

Use CMake 3.25+, C++17, SDL3 3.2+, imported distribution dependencies and offline glslc commands. Never glob production sources or fetch during configure. Reject non-x86-64; deterministic units use -fno-fast-math and -ffp-contract=off.

Treat retail files as read-only, including the ignored original-game symlink. Commit only project-owned source, synthetic fixtures and permitted logical/derived evidence. Ordinary tests must run without private media, interactive devices or a network fetch.

## Interfaces and Compatibility

Expose only ZH_BUILD_TESTS, ZH_ENABLE_ASAN, ZH_ENABLE_UBSAN, ZH_ENABLE_LAN, ZH_ENABLE_GPU_TESTS and ZH_ENABLE_RETAIL_TESTS; GPU/retail default OFF. Preserve explicit target ownership including WWShade and generated build revision.

Use the canonical x86-64 presets `linux-gcc-debug`, `linux-clang-debug`, `linux-gcc-release` and `linux-clang-release`. Shared primitive codecs and public engine interfaces remain compatibility boundaries. Changes outside this contract require source-authorized evidence, not incidental cleanup.

## Acceptance Criteria

- [ ] PRE-003 and PRE-004 are complete; GCC and Clang Debug/Release presets configure from a clean checkout on x86-64, tests can be selected by label, missing packages name their package/capability, and configure performs no network access.
- [ ] Every scoped behavior and interface above has positive evidence; the listed negative/boundary cases fail with actionable diagnostics.
- [ ] The required validation below passes; deferred work is not used to mask an unfinished path.
- [ ] Evidence records compiler/build or device/corpus context as applicable without private media or absolute private paths.

## Required Validation

Run `cmake --preset <preset>`, `cmake --build --preset <preset>` and `ctest --preset <preset> --output-on-failure` for the asset-free smoke target under each preset.

Configure/build the asset-free smoke target with all four presets; missing compiler/dependency/glslc must name its requirement. Clean generated directories must expose stale shader/header mistakes. Record tested Arch package versions and exact Ubuntu/Fedora package names.

Retail checks use a separate build directory with `ZH_ENABLE_RETAIL_TESTS=ON` and local `ZH_RETAIL_ZH_DATA`, `ZH_RETAIL_GENERALS_DATA`, `ZH_RETAIL_LANGUAGE`. Hardware jobs additionally enable `ZH_ENABLE_GPU_TESTS=ON`; defaults stay OFF. Do not claim future commands have run when compiling this packet.

## Known Risks / Deferred Work

Full game linking and runtime acceptance are M3 onward; no new environment-only milestone is introduced.

This is the M0 explicitly required by source §8, preserved without synthesizing another readiness milestone.
