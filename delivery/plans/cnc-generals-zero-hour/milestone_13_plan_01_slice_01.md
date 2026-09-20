# M13 slice 01: bounded FFmpeg custom-I/O decode

## Goal and observable outcome

Open a logical VFS movie through FFmpeg custom I/O, discover Bink video/audio streams, decode and convert bounded frames/audio with stable timestamps, and report logical-path diagnostics for malformed or unsupported input.

## Scope

- Add a video API and FFmpeg implementation backed only by `VirtualFileSystem` bytes and `AVIOContext` read/seek callbacks.
- Discover Bink video plus optional Bink audio and expose metadata without physical paths.
- Decode video to tightly bounded RGBA8 and audio to interleaved stereo float.
- Enforce limits on source bytes, packets, dimensions, pixel count, frame bytes, audio samples, and queued output.
- Normalize missing/broken timestamps monotonically while retaining valid media timestamps.
- Reject missing decoder, corrupt/truncated input, excessive dimensions, unsupported streams, and queue overflow with subsystem/logical-path errors.

## Non-scope

Renderer commands, playback policy, localized fallback, physical output, and full retail corpus acceptance are excluded.

## Dependencies and ordering constraints

This first slice depends on the completed M4 VFS and distribution FFmpeg dependency probe. Slice 02 consumes its decoded frame/audio structures.

## Entry point and end-to-end behavior

`FfmpegVideoDecoder` receives a mounted VFS and logical path, reads bounded bytes into custom I/O, validates streams/dimensions/codecs, and yields ordered decoded video/audio batches until EOS.

## Data or state transitions

Decoder state transitions closed -> opened -> decoding -> end/error -> closed. Output queues remain within configured capacities; any terminal decode error stops admission and remains inspectable.

## Authorization and permissions

Not applicable beyond the already-authorized read-only VFS. The implementation performs no writes, extraction, network access, or device access.

## Validation, errors, and recovery

Positive structural tests exercise bounded queues, timestamp normalization, and custom-I/O seek semantics. Negative tests cover missing resources, bad signature/truncation, excessive declared bytes/dimensions, absent codec, queue overflow, and idempotent close. Diagnostics name `video` and the logical asset only.

## Expected implementation surfaces

`include/zh/video/decoder.h`, `src/video/decoder.cpp`, CMake wiring, and `tests/video/test_video_decoder.cpp`.

## Positive tests

- VFS custom read/seek preserves byte position and EOS semantics.
- Bounded packet/frame/audio queues preserve FIFO ordering and media timestamps.
- Pixel/audio conversion sizing and monotonic timestamp repair are deterministic.

## Negative tests

- Missing, corrupt, truncated, oversized, decoder-disabled, and capacity-exhausted inputs fail without partial output or unbounded allocation.

## Required validation commands

```sh
cmake --build --preset linux-gcc-debug --target video_decoder_tests
ctest --preset linux-gcc-debug -R '^video_decoder$' --output-on-failure
```

## Acceptance criteria

The custom-I/O decoder API is bounded, Bink-specific at its product boundary, safe on malformed input, and testable without private media.

## Delivered evidence

- `cmake --preset linux-gcc-debug` configured against distribution FFmpeg 8.0.1 libraries without a network fetch.
- `cmake --build --preset linux-gcc-debug --target video_decoder_tests` passed.
- `ctest --preset linux-gcc-debug -R '^video_decoder$' --output-on-failure` passed (1/1).
- Focused tests cover FIFO/capacity rejection, dimension/pixel/conversion bounds, timestamp repair, missing/corrupt/truncated resources, decoder-unavailable injection, source-byte limits, and logical-path diagnostics.
- The production decoder uses `AVIOContext` read/seek callbacks over bounded VFS bytes, accepts only FFmpeg's Bink demuxer/Bink codecs, and bounds packet, RGBA, audio conversion, and queued-audio storage.

## Commit boundary

Commit plan, decoder API/implementation, CMake wiring, and focused tests as `delivery: M13 slice 01 decode bounded Bink video`.
