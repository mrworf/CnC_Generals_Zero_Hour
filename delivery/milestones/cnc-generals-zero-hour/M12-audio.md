# Milestone M12: audio

## Objective

Deliver audio as the source-defined M12 outcome. This contract compiles the accepted native Linux migration; it does not mark implementation complete.

## User/System Outcome

`audio` tests and corpus probes pass, representative speech/music/effects schedule correctly, no-device mode is playable, and corrupt assets name their logical path.

## Scope

- implement the existing manager over miniaudio with engine VFS callbacks, 2D/3D voices, streams, groups, priorities, loops, pan, pitch, attenuation, delay, occlusion/filtering, and queued completions;
- keep the audio callback free of simulation locks, engine allocation, and game-object calls;
- validate all corpus encodings plus PCM/ADPCM/MP3 synthetic cases; and
- support deterministic null/no-device execution, focus/pause, device failure, and race-free shutdown.

## Explicit Exclusions

Video decode/presentation M13 and gameplay audio API redesign are excluded.

Only Zero Hour in GeneralsMD is ported. Base Generals executable, authoring tools, DRM, obsolete online services, unrelated engine rewrites, non-x86-64 support and retail redistribution remain excluded.

## Source Requirements

- [Authoritative port plan](../../../docs/zero-hour-linux-port-plan.md), §8 “M12 — audio”.
- Governing sections: §3 Selected replacement stack; §6 Audio / Time, threads, diagnostics, and compression; PRE-011.
- Shared §1 scope, §2 preservation rules, §3 selected stack, §7 dependency/check contract and §10 validation matrix apply. Source revision and commit are recorded in status.yaml; this contract cannot supersede that authority.

## Preconditions

M4. PRE-011 is satisfied by the initial M12 acquisition/review task before audio integration; its absence does not create a circular entry gate.

Direct implementation dependencies: [M4](M4-retail-vfs.md).

If a precondition fails, retain completed evidence and stop only this milestone and dependants; do not claim acceptance from a partial demonstration.

- `PRE-009` — verified VFS and logical English corpus manifest; provider M4; require completed acceptance evidence before entry (currently pending implementation).

## Readiness checks

- Inspect status.yaml and linked acceptance evidence for M4 — each direct provider must have completed its contract; pending implementation is not completed evidence.
- After M0 exists, run `cmake --list-presets` — all four source-defined presets are listed. Inspect provider test/evidence records and use their documented checks if freshness is in doubt; do not require this milestone's unfinished features at entry.
- Before audio integration, inspect the acquired miniaudio URL, exact version, hash, license and build options; verify its recorded source hash and offline build. Acquisition/review is the first M12 task, not a prerequisite for starting M12.

Produces `PRE-011` (reviewed, URL/hash/license-pinned miniaudio source); `PRE-023` (VFS-backed audio and null sink) as part of this milestone's acceptance, not as entry requirements.

Readiness audit: [prerequisite manifest](../../readiness/cnc-generals-zero-hour/prerequisite-manifest.md). Unmet providers stop this milestone and dependants only; all implementation statuses remain pending.

## Functional Requirements

Implement the scoped work above using the concrete interface contract below.

Complete every scoped work item and its source exit together; preserve existing simulation, formats and engine-facing behavior except the explicitly replaced platform edges.

## Architecture / Security Constraints

First M12 task acquires/reviews miniaudio and records URL, release/commit, hash, license and options; satisfy PRE-011 before compiling/integrating it. Offline builds only. Real-time callback cannot allocate via engine allocators, lock simulation or call game objects; queue completion to game thread.

Treat retail files as read-only, including the ignored original-game symlink. Commit only project-owned source, synthetic fixtures and permitted logical/derived evidence. Ordinary tests must run without private media, interactive devices or a network fetch.

## Interfaces and Compatibility

Preserve existing audio manager API with engine VFS-backed miniaudio; implement 2D/3D voices, streams, groups, priorities/limits, loops, volume/pan/pitch, listener/distance/attenuation/delay/filter/occlusion and completion scheduling.

Use the canonical x86-64 presets `linux-gcc-debug`, `linux-clang-debug`, `linux-gcc-release` and `linux-clang-release`. Shared primitive codecs and public engine interfaces remain compatibility boundaries. Changes outside this contract require source-authorized evidence, not incidental cleanup.

## UX Constraints

Preserve existing retail interaction and selected English locale behavior described in §6 and the scoped requirements above. Show actionable subsystem/path/asset errors and preserve safe shutdown/recovery. Do not redesign navigation or add service dependencies.

## Acceptance Criteria

- [ ] `audio` tests and corpus probes pass, representative speech/music/effects schedule correctly, no-device mode is playable, and corrupt assets name their logical path.
- [ ] Every scoped behavior and interface above has positive evidence; the listed negative/boundary cases fail with actionable diagnostics.
- [ ] The required validation below passes; deferred work is not used to mask an unfinished path.
- [ ] Evidence records compiler/build or device/corpus context as applicable without private media or absolute private paths.

## Required Validation

Run `cmake --build --preset <preset>` and `ctest --preset <preset> -L 'audio' --output-on-failure` for applicable supported presets.

Decode PCM WAV, Microsoft/IMA ADPCM, MP3 and every corpus encoding; verify scheduling, streaming from archives without extraction, focus/pause/device failure, race-free shutdown and null/no-device behavior. Missing device warns once and allows silent play; corrupt required asset remains named data error.

Retail checks use a separate build directory with `ZH_ENABLE_RETAIL_TESTS=ON` and local `ZH_RETAIL_ZH_DATA`, `ZH_RETAIL_GENERALS_DATA`, `ZH_RETAIL_LANGUAGE`. Hardware jobs additionally enable `ZH_ENABLE_GPU_TESTS=ON`; defaults stay OFF. Do not claim future commands have run when compiling this packet.

## Known Risks / Deferred Work

Audible checks where device exists; no-device play is required. Dependency acquisition may need network permission once; never configure-time fetching.

PRE-011 acquisition and review are owned here; version selection is an implementation choice constrained by recorded provenance, license and offline builds.
