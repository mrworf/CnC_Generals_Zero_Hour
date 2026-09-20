# M12 slice 03: format decoding, lifecycle failures, null sink, and corpus probes

## Goal and observable outcome

Complete M12 with verified required decoders, deterministic no-device playability, actionable corrupt-data reporting, race-free shutdown, and gated retail corpus encoding probes.

## Scope

- Decode project-owned PCM WAV, Microsoft ADPCM WAV, IMA ADPCM WAV, and MP3 fixtures through miniaudio VFS callbacks.
- Add deterministic null/no-device startup that warns once but accepts/schedules valid audio.
- Exercise output/device failure, pause/focus, concurrent completion/shutdown, and idempotent teardown.
- Add a gated retail corpus probe that uses configured roots and records only logical paths/encodings.
- Document operator behavior and milestone evidence.

## Non-scope

Audible quality approval when no safe device is available, private corpus bytes, video synchronization, and M14 full hardware acceptance are excluded.

## Dependencies and ordering

Depends on slices 01 and 02. This is the milestone acceptance slice.

## Entry point and end-to-end behavior

Asset-free audio tests mount synthetic files, decode/schedule/render them through the adapter, simulate device absence/failure and focus changes, then shut down while completions are pending. Retail probes run only with `ZH_ENABLE_RETAIL_TESTS=ON` and configured data roots.

## Data or state transitions

Device state transitions initializing -> ready/null/failure -> stopping -> stopped. Missing device selects null output once; corrupt data remains an asset error. Shutdown closes admission, drains/stops voices, joins worker/device activity, and releases VFS handles exactly once.

## Authorization and permissions

Retail probing is opt-in and read-only. No test copies, hashes, uploads, or writes retail bytes. Device access is optional; absence is an accepted null-sink path.

## Validation, errors, and recovery

Every required encoding must decode a bounded frame count. Unsupported/corrupt content names its logical path. Device absence warns once; injected device failure degrades safely. Repeated/concurrent shutdown is race-free and idempotent.

## Expected implementation surfaces

Audio decoder/lifecycle sources, synthetic fixture generator or checked-in small fixtures, corpus probe test, CMake test declarations, `docs/building-linux.md`, and M12 plans.

## Positive tests

- PCM, MS ADPCM, IMA ADPCM, and MP3 decode and schedule.
- Null sink, pause/focus recovery, completion drain, and concurrent shutdown pass.
- Optional retail probe reports supported logical encodings without exposing physical paths.

## Negative tests

- Corrupt/truncated/unsupported data names the logical path.
- Injected device failure warns once and stays playable; post-shutdown submissions fail cleanly.

## Required commands

```sh
cmake --build --preset <preset>
ctest --preset <preset> -L audio --output-on-failure
```

Run for all four canonical presets. When local retail roots are available, configure a separate retail build and run the `audio` corpus probe.

## Acceptance criteria

All four preset audio suites pass, all required encodings have positive evidence, null/no-device mode remains playable, failures name logical assets, and shutdown is race-free.

## Commit boundary

Commit format/lifecycle implementation, generated synthetic fixtures/tests, corpus probe, docs, and final plan evidence as `delivery: M12 slice 03 complete audio acceptance`.
