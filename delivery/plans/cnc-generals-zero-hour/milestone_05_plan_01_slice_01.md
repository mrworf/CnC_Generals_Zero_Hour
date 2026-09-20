# M5 slice 01: Linux saves and autosave metadata

## Goal and observable outcome

A caller can encode a bounded Linux save containing representative deterministic game state and autosave metadata, decode it to equivalent state, and receive actionable rejection of malformed or incompatible data without mutation of its existing state.

## Scope

- Define a Linux save format with a magic value, explicit little-endian marker, version, fixed-width fields, UTF-16LE text, and bounded collections.
- Encode/decode scenario state and optional autosave metadata through the M1 byte codec.
- Validate version, endian marker, lengths, counts, flags, UTF-16 well-formedness, trailing data, and total document size.
- Decode transactionally into a temporary value and assign only after full validation.

## Non-scope

Replay commands/checkpoint CRCs (slice 02), deterministic execution (slice 03), filesystem save placement, and Windows import (M18).

## Dependencies and ordering

M1 codecs and completed M4 are available. This is the first M5 slice and provides format/error conventions used by slice 02.

## Entry point and behavior

`zh::persistence::encode_save` returns bytes for a validated `SaveState`. `decode_save(bytes, destination)` either atomically replaces `destination` with an equivalent decoded state or throws `PersistenceError` while leaving it unchanged.

## State transitions

Encode is side-effect free. Decode transitions `destination` from its old value to the fully validated decoded value in one final assignment; every error retains the old value.

## Authorization and permissions

Not applicable: this is an in-memory codec with no filesystem, device, network, or privileged operation.

## Validation and recovery

Reject wrong magic/version/endian marker, invalid flags, excessive counts or lengths, truncation, unpaired UTF-16 surrogates, non-finite coordinates, and trailing bytes. Diagnostics identify the failed format boundary. Recovery is retrying with another byte sequence; caller state is preserved.

## Implementation surfaces

- `include/zh/persistence/save.h`
- `src/persistence/save.cpp`
- `tests/persistence/test_save.cpp`
- `CMakeLists.txt`
- this plan and governing plan

## Tests and commands

Positive: manual and autosave round trips including non-ASCII UTF-16 names and multiple entities. Negative: wrong magic/version/endian, excessive count/length, truncated UTF-16, unpaired surrogate, invalid flags/non-finite value, trailing bytes, and explicit proof that destination state remains unchanged.

```sh
cmake --preset linux-gcc-debug
cmake --build --preset linux-gcc-debug
ctest --preset linux-gcc-debug -R persistence_save --output-on-failure
```

## Acceptance criteria

- Encoded fields are fixed width and little endian through the shared codec.
- Manual/autosave state round-trips exactly.
- All malformed inputs fail before state publication and bounded counts are checked before allocation.
- No host `bool`, enum, `long`, pointer, or object-memory encoding exists.

## Commit boundary

Commit this plan, the governing plan/index, save codec, build wiring, and focused tests together as `delivery: M5 slice 01 add Linux saves`.
