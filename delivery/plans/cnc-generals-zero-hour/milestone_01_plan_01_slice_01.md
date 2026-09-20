# M1 slice 01: portable ABI and codecs

## Goal and observable outcome

Engine-facing code can express fixed-width values, convert well-formed UTF-8/UTF-16, encode/decode bounded endian fields, format UTF-16 safely, and perform specified numeric conversions without compiler extensions or host-layout assumptions.

## Scope

- Fixed-width aliases, `char16_t` `WideChar`, layout assertions, byte spans, and explicit ASCII conversion.
- Strict UTF-8/UTF-16 conversion and UTF-16LE string codecs.
- Bounded reader/writer for integer widths, IEEE-754 floats, byte arrays, and length-prefixed collections.
- Bounded portable formatting helpers for engine diagnostics and UTF-16 substitution.
- Checked truncation/floor/ceiling/nearest conversion and local `FE_TONEAREST` assertion.

## Non-scope

Legacy save/replay/packet call-site migration, gameplay types, Windows fixture compatibility, and wholesale literal conversion.

## Dependencies and ordering

M0 only. This establishes contracts consumed by slices 02 and 03.

## Entry point and end-to-end behavior

The `foundation_abi_codec_tests` executable builds a representative primitive record, verifies its exact stable bytes, decodes it back, converts Unicode at UTF-8 boundaries, and characterizes numeric conversions.

## Data and state transitions

Writers either append a complete value or report capacity exhaustion without a partial field. Readers advance only after a complete validated field. Unicode conversion returns a value or a position-bearing error and does not return partial text.

## Authorization and permission behavior

Not applicable; all inputs are in-memory synthetic fixtures.

## Validation and error handling

- Positive: all integer/endian widths, floats including signed zero, valid BMP/supplementary Unicode, ASCII identifiers, and bounded formatting.
- Negative: truncated reads, capacity exhaustion, invalid UTF-8, lone/reversed surrogates, lossy ASCII, oversized collection/string, NaN/infinity/out-of-range numeric narrowing.
- Boundary: zero-length values, integer extrema, exact buffer fit, half values on both sides of zero, and FE mode restoration.

## Implementation surfaces

`include/zh/foundation/{types,unicode,byte_codec,numeric,format}.h`, matching `src/foundation` sources, CMake target wiring, and `tests/foundation/test_abi_codec.cpp`.

## Required validation

- Build and run the focused `foundation_abi_codec_tests` target/test.
- Run `ctest --preset linux-gcc-debug -L foundation --output-on-failure`.
- Run `git diff --check`.

## Acceptance criteria

Static widths hold; exact fixture hex matches; all malformed/boundary cases fail without UB or partial mutation; no `wchar_t`, host `long`, raw struct copy, or compiler assembly enters the new contract.

## Commit boundary

Commit this plan, ABI/codec/numeric implementation, CMake wiring, and focused tests as `delivery: M1 slice 01 add portable codecs`.

## Completion evidence

- `foundation_abi_codec_tests` passes and verifies the stable fixture `a534125678efcdab8910203040efcdab8967452301000000800000c03f0300000041003dd842de`.
- The focused suite exercises exact-fit/overflow mutation, a synthetic BIG header with mixed endian fields, truncated read cursor stability, valid supplementary Unicode, malformed UTF-8/surrogates, ASCII loss, formatting bounds, signed zero, halves, NaN, infinity, and numeric overflow.
- GCC Debug `ctest -L foundation` passes all seven M0/M1 tests; broader compiler/build/sanitizer evidence is recorded by slice 03.
