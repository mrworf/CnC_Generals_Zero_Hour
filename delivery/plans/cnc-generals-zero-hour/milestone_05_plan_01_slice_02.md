# M5 slice 02: replay commands and checkpoint snapshots

## Goal and observable outcome

A caller can write and read a versioned Linux replay command stream with fixed-width checkpoint snapshots and CRC fields; malformed streams are rejected without partially replacing an existing replay.

## Scope

- Define replay header/version/endian fields, scenario/config identifiers, bounded commands, and bounded checkpoint records.
- Encode commands using explicit tick/type/player/payload fields.
- Encode snapshots as canonical fixed-width state fields and compute/store/verify CRC-32 over canonical snapshot bytes.
- Decode transactionally with exact ordering and boundary validation.

## Non-scope

Executing the replay across presets (slice 03), networking, Windows replay import, and full legacy engine object integration.

## Dependencies and ordering

Depends on slice 01 format/error conventions and M1 codecs. Slice 03 consumes the replay API.

## Entry point and behavior

`encode_replay` produces a self-contained byte stream. `decode_replay(bytes, destination)` validates the entire stream, including every stored snapshot CRC, then atomically replaces `destination`. `snapshot_crc32` is defined over canonical snapshot bytes rather than host memory.

## State transitions

Replay encoding is side-effect free. Replay decode retains the destination on every error and publishes the decoded value only when no bytes remain. CRC disagreement names its checkpoint.

## Authorization and permissions

Not applicable: all operations are deterministic in-memory transforms.

## Validation and recovery

Reject wrong version/endian, unsupported command kinds, decreasing command/checkpoint ticks, counts/payloads/text beyond limits, malformed UTF-16, truncated fields, trailing bytes, and CRC mismatches. Callers may discard the bad replay and report the diagnostic.

## Implementation surfaces

- `include/zh/persistence/replay.h`
- `src/persistence/replay.cpp`
- `tests/persistence/test_replay.cpp`
- `CMakeLists.txt`
- this slice plan

## Tests and commands

Positive: multi-command replay round trip, canonical snapshot byte fixture, stable CRC, and non-ASCII text. Negative: version/endian/count/length/type/order/UTF-16/truncation/trailing-data/CRC failures plus destination preservation.

```sh
cmake --build --preset linux-gcc-debug
ctest --preset linux-gcc-debug -R persistence_replay --output-on-failure
```

## Acceptance criteria

- Replay commands, snapshots, and CRCs have canonical fixed-width encodings.
- Equivalent state round-trips and CRC corruption identifies the checkpoint.
- Bounds are checked before payload allocation and failure never publishes partial state.

## Commit boundary

Commit replay codec, tests, build wiring, and this plan as `delivery: M5 slice 02 add Linux replays`.
