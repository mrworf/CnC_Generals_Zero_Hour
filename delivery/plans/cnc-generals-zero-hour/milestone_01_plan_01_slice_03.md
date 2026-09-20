# M1 slice 03: bounded compression and validation matrix

## Goal and observable outcome

Foundation consumers can identify and safely decode bounded RefPack and tagged zlib data, create source-compatible zlib envelopes, and receive a typed actionable error for unsupported `NOX`; the entire foundation suite passes across compilers/build types and sanitizers.

## Scope

- Compression envelope classification and little-endian uncompressed-size handling.
- System-zlib `ZL1`-`ZL9` compression/decompression with preserved level tag.
- Bounded RefPack decoder adapted from the source-established command forms.
- Typed unsupported-compression error carrying archive/logical-path context for `NOX`.
- Synthetic valid/truncated/overflow/back-reference fixtures and full validation matrix evidence.

## Non-scope

LZH decoding, B-tree/Huffman modernization, retail archive use, compression call-site migration, or compatibility claims based on private fixtures.

## Dependencies and ordering

Depends on slice 01 codecs and slice 02 support being stable.

## Entry point and end-to-end behavior

`foundation_compression_tests` round-trips bytes through every zlib level, decodes synthetic literal/back-reference RefPack streams, and validates typed failures. The canonical CTest invocation then exercises all M0/M1 foundation checks.

## Data and state transitions

Decode returns a complete vector only after header, advertised size, input consumption, back-reference, and output bounds validate. Error paths expose no partially decoded buffer.

## Authorization and permission behavior

Not applicable; all fixtures are synthetic in-memory bytes and system zlib is distribution-provided.

## Validation and error handling

- Positive: zlib levels 1-9, empty/small/binary payloads, RefPack literal and overlapping back-reference commands.
- Negative: `NOX`, unknown tag, truncated header/stream, corrupt zlib, output-size mismatch, invalid RefPack type/distance, and configured allocation-limit overflow.
- Boundary: empty payload, exact allocation limit, RefPack final literal lengths 0-3, and maximum accepted advertised size.

## Implementation surfaces

`include/zh/foundation/compression.h`, `src/foundation/compression.cpp`, target/CMake test wiring, `tests/foundation/test_compression.cpp`, and completion evidence in these M1 plans.

## Required validation

- Focused compression test and complete `foundation` label.
- Four canonical preset configure/build/foundation runs.
- Emit and compare primitive fixture bytes from all four presets.
- ASan/UBSan configure/build/foundation run where available.
- `git diff --check`.

## Acceptance criteria

Compression never trusts host size/alignment, malformed streams fail deterministically, `NOX` names the logical source, four-preset fixture bytes match, and sanitizer validation is clean.

## Commit boundary

Commit this plan, compression implementation/tests, and recorded matrix evidence as `delivery: M1 slice 03 add bounded compression`.

## Completion evidence

- System-zlib envelopes round-trip synthetic empty, binary, and small payloads at levels 1 through 9 while preserving `ZL1`-`ZL9` tags.
- Bounded RefPack tests cover literal and overlapping back-reference commands; truncated/corrupt streams, invalid distances, trailing bytes, header disagreement, and allocation limits fail before returning partial output.
- `NOX` produces a typed unsupported error containing the supplied archive/logical path; unknown and not-yet-adapted legacy tags do not fall through as uncompressed bytes.
- GCC/Clang Debug/Release full builds each pass all nine `foundation` tests. Each preset emits the identical checked fixture `a534125678efcdab8910203040efcdab8967452301000000800000c03f0300000041003dd842de`.
- Combined ASan/UBSan full builds and foundation suites pass with GCC 16.2.1 and Clang 22.1.8. Leak detection alone is disabled because LeakSanitizer reports that the restricted ptrace environment is unsupported.
