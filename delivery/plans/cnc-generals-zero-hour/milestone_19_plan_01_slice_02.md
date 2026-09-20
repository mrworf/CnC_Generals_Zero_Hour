# M19 slice 02: original compression runtime boundary

Status: pending

## Goal and observable outcome

The production compression target compiles actual original Zero Hour compression translation units and a harness observes a source-owned encode/decode result with compile/link/runtime identity evidence.

## Scope

Port the required original RefPack/EAC boundary with narrow allocator and bounds adaptations. Link original translation units into `zh_compression`, remove its bootstrap provider, and test valid round trips, size/header boundaries, malformed/truncated input rejection, and provider removal.

## Non-scope

No retail compressed input is used. Unused codecs may remain classified as deferred only when their consumers are not part of the M19 support boundary and their provider/rationale is explicit. Game data loading remains later work.

## Dependencies and ordering

Depends on slice 01's identity contract.

## End-to-end behavior and state

An asset-free harness sends owned bytes through original compression code and receives the original decoded payload plus a runtime provider witness. Invalid lengths and headers fail without out-of-bounds access or partial-success claims. No persistent state is written.

## Authorization and permissions

No privileged operation, network, retail data, display, or GPU access applies.

## Validation and recovery

Positive round-trip and boundary vectors; negative malformed/truncated buffers and identity-provider removal. Sanitizers cover the original source objects. Errors return bounded explicit failures; callers may retry only with corrected input.

## Expected implementation surfaces

`GeneralsMD/Code/Libraries/Source/Compression/`, portability adapter headers/sources, `CMakeLists.txt`, and `tests/original_support/`.

## Commands

- GCC/Clang focused builds and original-compression CTests.
- ASan/UBSan focused original-compression tests.
- Link/runtime identity validator.

## Acceptance criteria

- Actual original `.cpp` files appear in compiler commands and final link.
- Runtime output proves original encode/decode behavior.
- Malformed input is bounded and fails explicitly.
- Removing the original provider makes the identity gate fail.

## Commit boundary

One commit containing original-source portability changes, target wiring, harness/tests, evidence, and this slice's completion record.
