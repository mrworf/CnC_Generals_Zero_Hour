# Milestone M17: reproducible x86-64 release

## Objective

Deliver reproducible x86-64 release as the source-defined M17 outcome. This contract compiles the accepted native Linux migration; it does not mark implementation complete.

## User/System Outcome

a clean checkout builds with documented distribution dependencies, the Arch x86-64 host can point it at owned data and pass the playable completion checklist, Ubuntu and Fedora clean builds pass, and known unverified compatibility claims are explicit.

## Scope

- run clean x86-64 GCC and Clang Debug/Release builds and labeled asset-free suites without network access;
- run ASan/UBSan, private retail-data smoke/acceptance, shader completeness, install/uninstall staging, arbitrary-CWD, read-only input-data, and missing-device tests;
- install only executable/project assets/defaults/licenses and document data selection, XDG locations, limitations, troubleshooting, and verified hardware; and
- verify the playable release on the primary Arch Linux x86-64 host and verify source builds on Ubuntu 26.04 LTS and Fedora 44. The latter may use clean containers/VMs for build and headless checks; the Arch M14 hardware result satisfies GPU evidence.

## Explicit Exclusions

Optional Windows fixtures and new binary packaging formats are excluded.

Only Zero Hour in GeneralsMD is ported. Base Generals executable, authoring tools, DRM, obsolete online services, unrelated engine rewrites, non-x86-64 support and retail redistribution remain excluded.

## Source Requirements

- [Migration supplement](../../../docs/zero-hour-source-engine-migration.md): SE-001, SE-009, SE-010. Evidence-grade and anti-proxy rules apply.

- [Authoritative port plan](../../../docs/zero-hour-linux-port-plan.md), §8 “M17 — reproducible x86-64 release”.
- Governing sections: §1 Scope and completion criteria; §4 Target architecture and build graph; §9 Packaging after M17; §10 Validation matrix; PRE-017.
- Shared §1 scope, §2 preservation rules, §3 selected stack, §7 dependency/check contract and §10 validation matrix apply. Source revision and commit are recorded in status.yaml; this contract cannot supersede that authority.

## Preconditions

PRE-025/PRE-026 — accepted M15/M16 integrated gameplay; PRE-017 — maintainer/CI-owned clean Ubuntu 26.04/Fedora 44 x86-64 environments before release validation; PRE-008/PRE-012/PRE-016 at Arch runtime checks. Produces PRE-027.

Direct implementation dependencies: [M15](M15-single-player.md), [M16](M16-lan-match.md).

Require integrated original-engine single-player/LAN evidence and final production compile/link closure without bootstrap or toy simulation. Host dependencies do not prove clean distro portability.

Historical component completion is not original-source acceptance. Preserve prior evidence and existing M15 scaffolding; do not reuse their pass counts as proof of this contract. Existing external retail/device/distro/optional-fixture requirements remain at their consuming validation checks.

## Readiness checks

Inspect status.yaml and linked original-source acceptance for M15, M16; all direct provider contracts must be complete. Verify current authoritative source revisions and accepted packet before creating a new milestone plan. The old M15 slice plan does not cover the new provider migration.

[Readiness manifest](../../readiness/cnc-generals-zero-hour/prerequisite-manifest.md) owns external-input auditing. Unmet providers stop only consumers; planning does not claim implementation completion.

## Functional Requirements

Implement the scoped work above using the concrete interface contract below.

Complete every scoped work item and its source exit together; preserve existing simulation, formats and engine-facing behavior except the explicitly replaced platform edges.

## Architecture / Security Constraints

Arch x86-64 playable gate and clean Ubuntu 26.04/Fedora 44 x86-64 GCC/Clang Debug/Release builds. Offline after packages installed. Install only executable/project shaders/defaults/licenses, never media or private paths.

Treat retail files as read-only, including the ignored original-game symlink. Commit only project-owned source, synthetic fixtures and permitted logical/derived evidence. Ordinary tests must run without private media, interactive devices or a network fetch.

## Interfaces and Compatibility

Document source-build/configure/build/CTest/install interface, XDG/data selection, tested packages, licenses, verified hardware, troubleshooting and known unverified compatibility.

Use the canonical x86-64 presets `linux-gcc-debug`, `linux-clang-debug`, `linux-gcc-release` and `linux-clang-release`. Shared primitive codecs and public engine interfaces remain compatibility boundaries. Changes outside this contract require source-authorized evidence, not incidental cleanup.

## Acceptance Criteria

- [ ] a clean checkout builds with documented distribution dependencies, the Arch x86-64 host can point it at owned data and pass the playable completion checklist, Ubuntu and Fedora clean builds pass, and known unverified compatibility claims are explicit.
- [ ] Every scoped behavior and interface above has positive evidence; the listed negative/boundary cases fail with actionable diagnostics.
- [ ] The required validation below passes; deferred work is not used to mask an unfinished path.
- [ ] Evidence records compiler/build or device/corpus context as applicable without private media or absolute private paths.

## Required Validation

Record compile/link identity and source-owned runtime witnesses alongside observable outcomes. Removing a required original implementation, using missing/malformed actual assets, corrupting original save/replay state or introducing original peer-state mismatch must fail the relevant gate. Component/fixture evidence cannot substitute for original-source integration or retail runtime. Map historical unproven obligations to these real-source tests.

Build/test every preset in clean Arch, Ubuntu and Fedora environments. Run all mandatory asset-free labels there; run explicitly provisioned retail/GPU acceptance on Arch. Optional `compatibility` fixtures never enter the mandatory release suite.

Clean checkout builds all presets; run all asset-free labels and sanitizers, shader completeness, staged install/uninstall, arbitrary-CWD/read-only data/missing-device tests. Explicitly provision separate retail/GPU jobs; replay Arch M14–M16 acceptance.

Retail checks use a separate build directory with `ZH_ENABLE_RETAIL_TESTS=ON` and local `ZH_RETAIL_ZH_DATA`, `ZH_RETAIL_GENERALS_DATA`, `ZH_RETAIL_LANGUAGE`. Hardware jobs additionally enable `ZH_ENABLE_GPU_TESTS=ON`; defaults stay OFF. Do not claim future commands have run when compiling this packet.

## Known Risks / Deferred Work

Do not enable retail/GPU/compatibility fixtures in ordinary CI. Optional M18 never blocks release. Flatpak/AppImage/superbuild are future decisions, not release dependencies.

Source §9 conditional gates remain evidence-driven; newly demonstrated blockers are recorded at the consuming milestone.
