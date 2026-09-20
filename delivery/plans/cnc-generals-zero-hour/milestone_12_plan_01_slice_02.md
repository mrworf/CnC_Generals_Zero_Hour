# M12 slice 02: VFS-backed manager, voices, spatial controls, and completions

## Goal and observable outcome

Provide the existing-manager-compatible audio adapter: callers schedule 2D/3D one-shots or streams by logical VFS path, control groups and spatial properties, enforce voice priorities, and receive completion only when the game thread drains a bounded queue.

## Scope

- Expose engine-facing audio manager, voice, listener, group, and playback descriptors.
- Adapt the project VFS to miniaudio read/seek/tell/info callbacks without extracting archives.
- Implement one-shot/streaming, loops, group/voice volume, pan, pitch, priority/limit eviction, delay, min/max distance, attenuation, occlusion/low-pass controls, pause/focus, and listener transforms.
- Use fixed-capacity command/completion queues; the callback consumes prevalidated/preallocated state and never calls game objects or locks simulation state.
- Queue completion identifiers for explicit game-thread dispatch.

## Non-scope

Format corpus breadth, real hardware acceptance, video audio routing, and gameplay call-site migration are excluded.

## Dependencies and ordering

Depends on slice 01 PRE-011 and M4 VFS. Slice 03 exercises the manager with real decoder formats and lifecycle faults.

## Entry point and end-to-end behavior

The caller constructs `AudioManager` with a `data::VirtualFileSystem`, schedules a logical path and descriptor, submits controls, renders deterministic frames into a provided buffer or null sink, and drains completion events on the game thread.

## Data or state transitions

Voice lifecycle is queued -> delayed/playing/paused -> completed/stopped/evicted -> completion queued -> completion drained. Invalid handles and queue overflow fail without partial mutation. Focus loss pauses active output and focus gain resumes it.

## Authorization and permissions

Not applicable. Reads use the already-authorized read-only VFS; no writable retail access or privileged device operation occurs.

## Validation, errors, and recovery

Missing/corrupt logical paths name the asset. Invalid parameters/handles and full queues return actionable errors. Priority eviction, looping, completion ordering, and archive-backed seek behavior are deterministic.

## Expected implementation surfaces

`include/zh/audio/manager.h`, `src/audio/manager.cpp`, `include/zh/audio/vfs.h`, `src/audio/vfs.cpp`, CMake declarations, and `tests/audio/test_audio_manager.cpp`.

## Positive tests

- Effects, speech, and music schedule through loose and archive-backed VFS data.
- 2D/3D controls, grouping, priority, loops, delays, focus pause, and queued completions behave deterministically.

## Negative tests

- Missing/corrupt assets, invalid handles/parameters, queue saturation, and lower-priority admission fail predictably without callback-side allocation or engine calls.

## Required commands

```sh
cmake --build --preset linux-gcc-debug --target audio_manager_tests
ctest --preset linux-gcc-debug -R '^audio_manager$' --output-on-failure
```

## Acceptance criteria

Every required manager control is observable in focused tests, VFS streams seek without extraction, and completion dispatch is demonstrably game-thread queued.

## Delivered evidence

- `cmake --preset linux-gcc-debug` passed with the VFS-backed manager target.
- `cmake --build --preset linux-gcc-debug --target audio_manager_tests` passed.
- `ctest --preset linux-gcc-debug -R '^audio_manager$' --output-on-failure` passed (1/1).
- The test covers effect/speech/music classes, loose and BIG-backed reads, streaming mode, 2D/3D distance, listener/group/voice controls, pan, pitch configuration, loop, delay, occlusion/low-pass, focus/pause, priority eviction/rejection, stop, queued completion drain, invalid parameters, and logical missing-path diagnostics.

## Commit boundary

Commit manager/VFS APIs and implementation, CMake wiring, focused tests, documentation, and updated plan evidence as `delivery: M12 slice 02 add VFS audio manager`.
