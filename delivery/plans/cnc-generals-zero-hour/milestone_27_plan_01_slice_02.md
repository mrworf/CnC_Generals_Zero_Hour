# M27 plan 01 slice 02: transfer, chunks and random streams

## Goal and observable outcome

Owned buffers round-trip original fixed-width transfer and chunk primitives, and the actual original random provider yields characterized deterministic sequences whose client/audio activity cannot perturb logic checkpoints.

## Scope

- Port the independently usable Xfer primitive methods and data-chunk stream core as source-owned extracted units shared with later integration.
- Encode Bool, Int, Int64, Real, ASCII, and `WideChar`/Unicode data with explicit little-endian widths.
- Preserve chunk label/version/size framing, nested bounds, unknown-chunk skipping, and callback dispatch.
- Compile and execute actual `RandomValue.cpp` after only narrow include/portability corrections.
- Characterize six-word logic-seed CRC and independent logic/client/audio streams.

## Non-scope

Snapshots, full save/replay compatibility, runtime object IDs/registries, filesystem save/load classes, Windows format compatibility, or M24 persistence acceptance.

## Dependencies and ordering

Depends on slice 01 provider and M26 strings/allocator. It supplies primitive codecs consumed by slice 03 map metadata.

## Entry point and behavior

The focused executable serializes typed records and nested chunks to owned memory, reopens them through the original-source reader, validates callback dispatch and values, then seeds the original random provider and compares deterministic checkpoints before and after independent client/audio draws.

## Data/state transitions

Typed state -> fixed-width bytes/chunks -> validated temporary decoded state -> committed output. Each random stream transitions only its own six-word seed, and reinitialization restores its documented seed state.

## Authorization and permissions

Not applicable; all operations are process-local owned buffers with no files, devices, or services.

## Validation and error handling

Positive tests cover boundary values, two-byte UTF-16 including surrogates, nested/unknown chunks, deterministic sequences, inclusive ranges, and logic CRC checkpoints. Negative tests cover truncated scalars/strings/chunks, impossible lengths, nesting/size overflow, wrong versions, invalid Bool encoding, and no output mutation on failed decode.

## Expected implementation surfaces

`CMakeLists.txt`; `include/zh/original_data.h`; original-tree Xfer/chunk extracted sources; actual `RandomValue.cpp` plus narrow portable includes; focused tests; classification/ledger updates.

## Required commands

- Focused build/tests in all four presets using the `original-data` label.
- Source identity and provider-removal checks for Xfer/chunk/random providers.
- No full suite until slice 03.

## Acceptance criteria

- Fixed-width runtime output is identical across all presets and `sizeof(WideChar) == 2`.
- Actual original random symbols execute; logic CRC/sequence is unchanged by client/audio activity.
- All malformed input is bounded and deterministic under Debug and Release.
- No persistence or Windows compatibility claim is made.

## Commit boundary

One commit containing codec/chunk/random behavior, directly required portability, tests, ledger/classification changes, and plan result. Slice 03 is excluded.
