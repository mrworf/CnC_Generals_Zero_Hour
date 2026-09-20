# Milestone M4: VFS, retail data, and corpus characterization

## Objective

Deliver VFS, retail data, and corpus characterization as the source-defined M4 outcome. This contract compiles the accepted native Linux migration; it does not mark implementation complete.

## User/System Outcome

PRE-009 is complete. Verification works from any CWD, names each resolved root and locale, produces a stable corpus manifest, accepts the selected owned installation, and gives actionable failures for missing, corrupt, colliding, traversing, oversized, or unsupported inputs.

## Scope

- implement explicit Zero Hour/base Generals roots, language selection, XDG configuration, deterministic loose/BIG precedence, case folding/collision reporting, and hardened BIG traversal;
- implement `--verify-data` so it never creates a window, GPU device, or audio device;
- parse enough INI, CSF, W3D, DDS/TGA, audio, video, font, compression, and WWShade metadata to inventory the supplied corpus safely; and
- check in only logical names, format/effect expectations, archive precedence, and synthetic regressions—never retail bytes or private paths.

## Explicit Exclusions

Conversion, extraction and automatic installation scanning are excluded.

Only Zero Hour in GeneralsMD is ported. Base Generals executable, authoring tools, DRM, obsolete online services, unrelated engine rewrites, non-x86-64 support and retail redistribution remain excluded.

## Source Requirements

- [Authoritative port plan](../../../docs/zero-hour-linux-port-plan.md), §8 “M4 — VFS, retail data, and corpus characterization”.
- Governing sections: §6 Filesystem, data roots, and XDG paths; §9 Unexpected required asset formats at M4 or consuming milestone; PRE-008–009.
- Shared §1 scope, §2 preservation rules, §3 selected stack, §7 dependency/check contract and §10 validation matrix apply. Source revision and commit are recorded in status.yaml; this contract cannot supersede that authority.

## Preconditions

M1, M3, and PRE-008. If retail data is temporarily unavailable, synthetic VFS work may proceed but the milestone cannot close.

Direct implementation dependencies: [M1](M1-portable-foundations.md), [M3](M3-headless-startup.md).

If a precondition fails, retain completed evidence and stop only this milestone and dependants; do not claim acceptance from a partial demonstration.

- `PRE-005` — portable ABI, codecs and support libraries; provider M1; require completed acceptance evidence before entry (currently pending implementation).
- `PRE-007` — controlled headless execution and null devices; provider M3; require completed acceptance evidence before entry (currently pending implementation).
- `PRE-008` — user-owned Zero Hour/base Generals roots and English locale remain readable; local symlink verified, formal corpus verification is this milestone's output.

## Readiness checks

- Inspect status.yaml and linked acceptance evidence for M1, M3 — each direct provider must have completed its contract; pending implementation is not completed evidence.
- After M0 exists, run `cmake --list-presets` — all four source-defined presets are listed. Inspect provider test/evidence records and use their documented checks if freshness is in doubt; do not require this milestone's unfinished features at entry.
- `test -d original_game_symlink && test -d original_game_symlink/ZH_Generals && git check-ignore original_game_symlink` — supplied local roots exist and link remains ignored. Set the three local retail variables to the supplied roots and English; inspect readability without writing or hashing retail bytes.

Produces `PRE-009` (verified VFS and logical English corpus manifest) as part of this milestone's acceptance, not as entry requirements.

Readiness audit: [prerequisite manifest](../../readiness/cnc-generals-zero-hour/prerequisite-manifest.md). Unmet providers stop this milestone and dependants only; all implementation statuses remain pending.

## Functional Requirements

Implement the scoped work above using the concrete interface contract below.

Complete every scoped work item and its source exit together; preserve existing simulation, formats and engine-facing behavior except the explicitly replaced platform edges.

## Architecture / Security Constraints

Read-only roots; normalize slashes, reject absolute/traversing logical paths and ASCII-case-fold. Priority: ZH loose > selected -mod BIGs > ZH BIGs > Generals loose > Generals BIGs; case-insensitive FilenameList order, first-loaded wins. Duplicate resources across archives resolve normally; case-only loose/archive-name collisions fail with both sources.

Treat retail files as read-only, including the ignored original-game symlink. Commit only project-owned source, synthetic fixtures and permitted logical/derived evidence. Ordinary tests must run without private media, interactive devices or a network fetch.

## Interfaces and Compatibility

Apply the concrete [shared writable-path contract](index.md#shared-writable-path-contract), including every XDG fallback and read-only retail boundary.

Implement --zh-data, --generals-data, --language, --verify-data and ZeroHourDataPath/GeneralsDataPath/Language; CLI overrides config. Allow identical roots; choose locale automatically only if exactly one valid locale exists.

Use the canonical x86-64 presets `linux-gcc-debug`, `linux-clang-debug`, `linux-gcc-release` and `linux-clang-release`. Shared primitive codecs and public engine interfaces remain compatibility boundaries. Changes outside this contract require source-authorized evidence, not incidental cleanup.

## Acceptance Criteria

- [ ] PRE-009 is complete. Verification works from any CWD, names each resolved root and locale, produces a stable corpus manifest, accepts the selected owned installation, and gives actionable failures for missing, corrupt, colliding, traversing, oversized, or unsupported inputs.
- [ ] Every scoped behavior and interface above has positive evidence; the listed negative/boundary cases fail with actionable diagnostics.
- [ ] The required validation below passes; deferred work is not used to mask an unfinished path.
- [ ] Evidence records compiler/build or device/corpus context as applicable without private media or absolute private paths.

## Required Validation

Run `cmake --build --preset <preset>` and `ctest --preset <preset> -L 'data' --output-on-failure` for applicable supported presets.

Test both roots, arbitrary CWD, every precedence layer, stable mount order and locale ambiguity; invalid BIG identifiers/counts/NUL paths/offsets/overflow/size bounds fail before exposure. Verify owned English corpus then report formats/fonts/effects/required logical assets; verification does not hash, upload, copy or rewrite retail data.

Retail checks use a separate build directory with `ZH_ENABLE_RETAIL_TESTS=ON` and local `ZH_RETAIL_ZH_DATA`, `ZH_RETAIL_GENERALS_DATA`, `ZH_RETAIL_LANGUAGE`. Hardware jobs additionally enable `ZH_ENABLE_GPU_TESTS=ON`; defaults stay OFF. Do not claim future commands have run when compiling this packet.

## Known Risks / Deferred Work

PRE-009 stores logical metadata only. Unexpected required format follows §9: supported narrow adapter, maintained licensed in-process decoder, or named consuming-milestone blocker; no content skipping/conversion helpers. Optional Windows recordings are not required.

Source §9 conditional gates remain evidence-driven; newly demonstrated blockers are recorded at the consuming milestone.
