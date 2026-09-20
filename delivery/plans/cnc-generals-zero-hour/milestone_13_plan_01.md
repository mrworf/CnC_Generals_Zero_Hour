# M13 delivery plan: headless Bink video

## Goal

Close M13 with a bounded FFmpeg/VFS video path that decodes the observed retail Bink variants headlessly, synchronizes presentation to media/audio clocks, records valid renderer commands, and preserves localized fallback and movie lifecycle behavior.

## Authority and constraints

- Milestone contract: `delivery/milestones/cnc-generals-zero-hour/M13-video.md`.
- Product authority: `docs/zero-hour-linux-port-plan.md`, especially Video, dependency policy, and M13.
- Direct providers M4, M7, and M12 are complete at transaction start `8b6ad202d31117e0aa78806a75749012364cc219`.
- Use only distribution FFmpeg shared libraries and custom VFS I/O. Do not add nonfree FFmpeg configuration, physical presentation, capture, or recording.
- Retail roots and `original_game_symlink` remain read-only. Commit no retail media, retail hashes, host paths, or other private-path evidence.
- Packet, decoded-frame, audio, dimensions, timestamps, and conversion allocations are bounded before use.

## Delivery slices

1. [Slice 01: bounded FFmpeg custom-I/O decode](milestone_13_plan_01_slice_01.md)
2. [Slice 02: synchronized presentation and lifecycle](milestone_13_plan_01_slice_02.md)
3. [Slice 03: localized fallback and retail variant acceptance](milestone_13_plan_01_slice_03.md)

The slices are dependency ordered. Decode and resource limits precede presentation; fallback and opt-in corpus evidence then close the complete user-visible path.

## Milestone validation

For all four canonical presets:

```sh
cmake --preset <preset>
cmake --build --preset <preset>
ctest --preset <preset> -L 'audio|video|renderer-contract' --output-on-failure
```

Acceptance additionally runs the gated, read-only retail corpus test in a separate build. It selects one logical movie for each observed combination of video dimensions and audio sample-rate presence, decodes bounded leading frames, and compares only legal derived metadata/frame-count expectations. The observed English corpus variants at planning time are Bink video/YUV420P at 96x120, 400x324, 640x480, 720x486, and 800x600, with silent, stereo 44.1 kHz Bink audio, and stereo 48 kHz Bink audio cases.

## Non-scope

Physical SDL_GPU presentation and pixel capture (M14), playback recording, retail redistribution, non-Bink product formats, and broad legacy movie-call-site rewrites are excluded.

## Commit record

- Slice 01: `0f280767e56d55baa795de4caa69090d3af83967`
- Slice 02: `67f01a42e1f72c3e85ba230c5fe40b090e0ef91e`
- Slice 03: `c316f1392b5045aa6bdec11ea4b96d00ff8527a7`
