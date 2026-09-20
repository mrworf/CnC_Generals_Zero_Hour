# M13 slice 02: synchronized presentation and lifecycle

## Goal and observable outcome

Drive decoded movie frames from media/audio time, route audio batches through a bounded adapter or null sink, and record texture-upload/draw commands while preserving pause, focus, skip, aspect scaling, EOS, and game-render return.

## Scope

- Add a bounded video player/controller around the decoder.
- Route decoded stereo float batches into an abstract audio adapter with a deterministic null sink and readable clock.
- Prefer the audio clock when audio exists and otherwise use monotonic media time.
- Record normalized texture upload and draw commands against `RecordingGpuDevice` with letterbox/pillarbox aspect preservation.
- Handle pause/focus, resume, skip, EOS, decoder errors, and shutdown without stale resources or callbacks.

## Non-scope

Actual texture pixels on a GPU, audible hardware acceptance, capture/recording, and localized path lookup are excluded.

## Dependencies and ordering constraints

Depends on slice 01 decoded output and the completed M7 recorder/M12 audio boundaries. Slice 03 supplies real Bink corpus inputs and fallback selection.

## Entry point and end-to-end behavior

The caller starts a movie, advances with an elapsed duration, toggles focus/pause or requests skip, and receives playing/paused/completed/skipped/error state. Due frames create ordered upload/draw markers and audio advances the master clock when present.

## Data or state transitions

idle -> playing <-> paused -> completed/skipped/error -> stopped. Shutdown is idempotent and destroys all recorder resources; focus loss pauses both clocks and audio submission.

## Authorization and permissions

Not applicable. This is headless command generation and in-memory audio routing only.

## Validation, errors, and recovery

Tests inject decoded batches to cover clock choice, bounded backpressure, timestamp ordering, aspect rectangles, pause/focus, skip, EOS, corruption propagation, and shutdown. Invalid dimensions or sink refusal transition to a clear error and release resources.

## Expected implementation surfaces

`include/zh/video/player.h`, `src/video/player.cpp`, renderer recorder support if needed, and `tests/video/test_video_player.cpp`.

## Positive tests

- Silent and audio-clock movies present frames in deterministic order.
- 4:3 and non-4:3 media preserve aspect within target dimensions.
- Pause/focus and resume do not advance clocks; EOS returns to game rendering.

## Negative tests

- Skip, sink overflow, invalid frames, decoder error, and shutdown all terminate safely with no live recorder resources.

## Required validation commands

```sh
cmake --build --preset linux-gcc-debug --target video_player_tests
ctest --preset linux-gcc-debug -R '^video_player$' --output-on-failure
```

## Acceptance criteria

Recorded commands prove bounded synchronized presentation policy and complete lifecycle recovery without a display or audio device.

## Commit boundary

Commit controller/audio-sink/recorder integration and focused tests as `delivery: M13 slice 02 synchronize video commands`.
