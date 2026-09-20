# M11 slice 01: bounded fixed-width LAN packet codec

## Goal and observable outcome

Provide a named, fixed-width packet format for discovery, join, acceptance/rejection, command, and disconnect traffic. Valid packets round-trip exactly; malformed external bytes are rejected before payload allocation or session mutation with field-specific diagnostics.

## Scope

- Define protocol version, message types, lobby/game port identities, packet and collection/string limits.
- Encode/decode fixed-width integer fields, sequence/tick clocks, peer/data/map identifiers, bounded UTF-8 text, and bounded command collections.
- Validate type, version, declared length, count, string length/content, and trailing bytes before returning a packet.
- Add wrap-safe 32-bit clock comparison and elapsed-time helpers.
- Add focused positive, boundary, and malformed-byte tests.

## Non-scope

No socket, session state machine, discovery scheduler, or process CLI is introduced here. Windows interoperability cannot be claimed without fixtures.

## Dependencies and ordering

Depends on M1 fixed-width primitives. Slice 02 consumes this public codec.

## Entry point and end-to-end behavior

Callers construct `lan::Packet`, call `encode_packet`, transmit the resulting bytes, and call `decode_packet` at the receive boundary. Decode either returns a fully validated value or throws `ProtocolError` without changing caller state.

## Data and state transitions

Encoding is pure and bounded. Decoding uses local temporary state and publishes a packet only after the complete datagram validates. Wrap-safe clock helpers compare modulo-2^32 tick values when intervals remain below half the clock range.

## Authorization and permissions

Not applicable: this slice is pure in-process serialization with no I/O or privileged operation.

## Validation, errors, and recovery

Reject unknown type, wrong version, header/body length mismatch, oversized datagram, invalid count, oversized/invalid string, truncated fixed fields, and trailing bytes. Diagnostics name the failed field. Corrected bytes can be decoded immediately because failures retain no state.

## Expected implementation surfaces

`include/zh/lan/protocol.h`, `src/lan/protocol.cpp`, `tests/lan/test_protocol.cpp`, explicit CMake source/test declarations, and these M11 plans.

## Positive tests

- Every message type round-trips with exact fixed-width values.
- Empty and maximum legal bounded fields are accepted.
- Wrap-safe clock comparison/elapsed behavior crosses `UINT32_MAX`.

## Negative tests

- Invalid type/version/declared length/count/string/truncation/trailing bytes fail with actionable field diagnostics.
- Encoding values beyond protocol bounds fails before output escapes.

## Required commands

```sh
cmake --preset linux-gcc-debug
cmake --build --preset linux-gcc-debug
ctest --preset linux-gcc-debug -R lan_protocol --output-on-failure
```

## Acceptance criteria

The wire format contains no host-sized fields or raw structures, all external lengths are bounded before allocation, and positive/negative codec tests pass.

## Delivered evidence

- `cmake --preset linux-gcc-debug` passed with distribution dependencies resolved locally.
- `cmake --build --preset linux-gcc-debug --target lan_protocol_tests` passed.
- `ctest --preset linux-gcc-debug -R '^lan_protocol$' --output-on-failure` passed (1/1).
- Tests cover every packet type, invalid type/version/declared length/count/UTF-8/trailing bytes, encode bounds, and wrap-around clocks.

## Commit boundary

Commit plans, protocol API/implementation, CMake wiring, and focused tests together as `delivery: M11 slice 01 bound LAN packets`.
