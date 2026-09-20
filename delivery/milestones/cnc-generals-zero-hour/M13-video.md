# Milestone M13: headless video decode and presentation commands

## Objective

Deliver headless video decode and presentation commands as the source-defined M13 outcome. This contract compiles the accepted native Linux migration; it does not mark implementation complete.

## User/System Outcome

`video` tests decode representative media headlessly with stable timing/order and valid recorded presentation commands. Actual A/V presentation and pixels remain M14 work.

## Scope

- implement FFmpeg custom I/O, Bink demux/decode, bounded frame/audio queues, pixel conversion, timestamps, and localized fallback;
- validate every observed Bink variant by decoded metadata/frame hashes or other non-retail derived expectations without displaying it;
- route audio through the audio adapter/null sink and generate texture upload/draw commands through the recorder; and
- cover skip, pause/focus, end-of-stream, corrupt/truncated packets, excessive dimensions, missing decoder, and shutdown.

## Explicit Exclusions

Physical-GPU presentation M14 and recording/frame-capture features are excluded.

Only Zero Hour in GeneralsMD is ported. Base Generals executable, authoring tools, DRM, obsolete online services, unrelated engine rewrites, non-x86-64 support and retail redistribution remain excluded.

## Source Requirements

- [Authoritative port plan](../../../docs/zero-hour-linux-port-plan.md), §8 “M13 — headless video decode and presentation commands”.
- Governing sections: §6 Video; §9 Unexpected required asset formats at M4 or consuming milestone; §10 Video.
- Shared §1 scope, §2 preservation rules, §3 selected stack, §7 dependency/check contract and §10 validation matrix apply. Source revision and commit are recorded in status.yaml; this contract cannot supersede that authority.

## Preconditions

M4, M7, and M12.

Direct implementation dependencies: [M4](M4-retail-vfs.md), [M7](M7-renderer-core.md), [M12](M12-audio.md).

If a precondition fails, retain completed evidence and stop only this milestone and dependants; do not claim acceptance from a partial demonstration.

- `PRE-009` — verified VFS and logical English corpus manifest; provider M4; require completed acceptance evidence before entry (currently pending implementation).
- `PRE-010` — validated recording device and normalized resource/command contracts; provider M7; require completed acceptance evidence before entry (currently pending implementation).
- `PRE-011` — reviewed, URL/hash/license-pinned miniaudio source; provider M12; require completed acceptance evidence before entry (currently pending implementation).
- `PRE-023` — VFS-backed audio and null sink; provider M12; require completed acceptance evidence before entry (currently pending implementation).

## Readiness checks

- Inspect status.yaml and linked acceptance evidence for M4, M7, M12 — each direct provider must have completed its contract; pending implementation is not completed evidence.
- After M0 exists, run `cmake --list-presets` — all four source-defined presets are listed. Inspect provider test/evidence records and use their documented checks if freshness is in doubt; do not require this milestone's unfinished features at entry.

Produces `PRE-024` (headless Bink decode and synchronized presentation commands) as part of this milestone's acceptance, not as entry requirements.

Readiness audit: [prerequisite manifest](../../readiness/cnc-generals-zero-hour/prerequisite-manifest.md). Unmet providers stop this milestone and dependants only; all implementation statuses remain pending.

## Functional Requirements

Implement the scoped work above using the concrete interface contract below.

Complete every scoped work item and its source exit together; preserve existing simulation, formats and engine-facing behavior except the explicitly replaced platform edges.

## Architecture / Security Constraints

Bound packet/frame dimensions/conversion buffers/queues; shared distribution avformat/avcodec/avutil/swscale/swresample only, no project nonfree configuration. Preserve aspect/scaling/localized fallback/skip/focus/pause/EOS and game-render return.

Treat retail files as read-only, including the ignored original-game symlink. Commit only project-owned source, synthetic fixtures and permitted logical/derived evidence. Ordinary tests must run without private media, interactive devices or a network fetch.

## Interfaces and Compatibility

Retain video player API through FFmpeg custom I/O over VFS, null/miniaudio audio adapter and recorder texture uploads/draws. Use media timestamps and audio clock when available.

Use the canonical x86-64 presets `linux-gcc-debug`, `linux-clang-debug`, `linux-gcc-release` and `linux-clang-release`. Shared primitive codecs and public engine interfaces remain compatibility boundaries. Changes outside this contract require source-authorized evidence, not incidental cleanup.

## UX Constraints

Preserve existing retail interaction and selected English locale behavior described in §6 and the scoped requirements above. Show actionable subsystem/path/asset errors and preserve safe shutdown/recovery. Do not redesign navigation or add service dependencies.

## Acceptance Criteria

- [ ] `video` tests decode representative media headlessly with stable timing/order and valid recorded presentation commands. Actual A/V presentation and pixels remain M14 work.
- [ ] Every scoped behavior and interface above has positive evidence; the listed negative/boundary cases fail with actionable diagnostics.
- [ ] The required validation below passes; deferred work is not used to mask an unfinished path.
- [ ] Evidence records compiler/build or device/corpus context as applicable without private media or absolute private paths.

## Required Validation

Run `cmake --build --preset <preset>` and `ctest --preset <preset> -L 'audio|video|renderer-contract' --output-on-failure` for applicable supported presets.

Headlessly decode all observed Bink variants with legal derived metadata/frame expectations; validate timestamps/order and command streams. Test truncation/corruption/missing decoder, excessive dimensions, absent fallback, skip/pause/EOS and teardown.

Retail checks use a separate build directory with `ZH_ENABLE_RETAIL_TESTS=ON` and local `ZH_RETAIL_ZH_DATA`, `ZH_RETAIL_GENERALS_DATA`, `ZH_RETAIL_LANGUAGE`. Hardware jobs additionally enable `ZH_ENABLE_GPU_TESTS=ON`; defaults stay OFF. Do not claim future commands have run when compiling this packet.

## Known Risks / Deferred Work

M14 validates actual A/V pixels/synchronization; missing decoder is reported by verification/startup. Never commit retail media.

Source §9 conditional gates remain evidence-driven; newly demonstrated blockers are recorded at the consuming milestone.
