# M12 delivery plan: VFS-backed miniaudio

## Goal

Close M12 with an offline, provenance-pinned miniaudio adapter that preserves the engine-facing audio manager behaviors, streams through the project VFS, decodes every required synthetic format, and remains playable with no audio device.

## Authority and constraints

- Milestone contract: `delivery/milestones/cnc-generals-zero-hour/M12-audio.md`.
- Product authority: `docs/zero-hour-linux-port-plan.md`, especially Audio, dependency policy, PRE-011, and M12.
- Direct provider M4 is complete at transaction start `33b9535ab1802c274e830f7e6ca74486a6b4ba17`.
- The first slice owns PRE-011: miniaudio 0.11.25, upstream commit `9634bedb5b5a2ca38c1ee7108a9358a4e233f14d`, its exact header/license hashes, chosen MIT-0 license, compile options, and an offline build proof.
- Retail roots and the ignored original-game symlink remain read-only. No retail bytes or private absolute paths may be committed.
- The audio callback may only consume preallocated command/completion queues and PCM buffers. It must not use engine allocation, simulation locks, or game-object calls.

## Delivery slices

1. [Slice 01: pin and compile miniaudio offline](milestone_12_plan_01_slice_01.md)
2. [Slice 02: VFS-backed manager, voices, spatial controls, and completions](milestone_12_plan_01_slice_02.md)
3. [Slice 03: format decoding, lifecycle failures, null sink, and corpus probes](milestone_12_plan_01_slice_03.md)

The slices are dependency ordered. The reviewed vendored decoder/device dependency is established before integration. The manager and its real-time boundary precede format/lifecycle and corpus acceptance.

## Milestone validation

For all four canonical presets:

```sh
cmake --preset <preset>
cmake --build --preset <preset>
ctest --preset <preset> -L audio --output-on-failure
```

Acceptance additionally requires a clean offline configure/build, synthetic PCM WAV, Microsoft ADPCM, IMA ADPCM, and MP3 decode probes, representative effects/speech/music scheduling, archive-backed seeking, focus/pause/device-failure behavior, race-free shutdown, one-warning null-sink playability, and logical-path diagnostics for corrupt or missing assets. Retail corpus probes are gated by the existing retail options and record only logical paths and detected encodings.

## Non-scope

Video delivery (M13), gameplay audio API redesign, redistribution of retail media, Windows audio backends, and broad legacy engine integration beyond the project-owned compatibility adapter are excluded.

## Commit record

- Slice 01: pending
- Slice 02: pending
- Slice 03: pending
