# N2 slice 01 — original core-owner process

## Goal, scope and entry point

Build a Linux executable using original common services. It initializes native
core owners, exercises original strings/geometry/RNG/CRC and explicit memory
pools, retires them, and repeats. This is partial original-source foundation,
not GameLogic acceptance. Depends on N0; precedes rooted data/full runtime.
No assets, renderer, registry, SDK shim, or replacement gameplay implementation.

## Planned implementation and state

Port retained BaseType/PreRTS/common headers to fixed-width types and native
standard includes. Correct case-sensitive include names on retained paths.
Replace x86 inline assembly with defined C++ preserving valid source behavior;
test finite conversion boundaries, bit patterns, unsigned RNG wrap and CRC bytes.
Inspect every parallel RNG family for coupled overflow/range errors.

Remove GameMemory's global allocation override and legacy duplicate placement
new declarations. Standard ordinary/nothrow/aligned allocation share their
standard owner. Preserve explicit class/DMA pools with correctly aligned backing,
checked size arithmetic, guarded construction, clear retirement and no cached
class-pool pointers surviving factory shutdown. Strings use independent standard
storage, preserving copy/mutation/value behavior and initialized backing. Inspect
ASCII/Unicode refcount and allocation together, not just compiler diagnostics.
Replace native critical sections with standard recursive mutex ownership.

Retained pool implementation will use checked native-aligned blobs with explicit
live-slot/free-slot metadata, instead of pointer arithmetic into legacy hidden
four-byte headers. DMA allocations carry an owner-specific live-address ledger,
so shared named subpools do not authorize another allocator's frees. Reserve
free-slot backing before publication; return partial blob/pool/DMA acquisitions
on failure. Keep the existing original pool/class-glue entry points, but replace
their internal allocation algorithm. This is Zero Hour-owned memory code, not
an adopted framework modification. Standard containers no longer recurse into
global game allocation. Test same-name DMA aliases and canceled/failed ownership
independently, with exact live counts and factory teardown.

Add explicit original-source CMake targets and a generated core-owner fixture.
Record verified source/lifecycle facts in original-engine-formats.md. Keep the
original source graph and explicit exclusions visible; no archived providers.

## Validation, recovery and acceptance

Positive: original RNG/CRC known checkpoints, independent logic/client/audio
streams, copy-on-write strings or equivalent independent value copies,
concatenation/aliasing, native alignment, pool growth and repeated factory use.
Negative/boundary: invalid sizes/overflow, constructor failure after acquisition,
pool owner mistakes before effects, zero-size/free-null, string capacity limits,
finite conversions and malformed-range admission. Validate ordinary, nothrow,
array and over-aligned C++ allocation outside pools under both sanitizers.
Keep each injected failure, rollback and retry in the same bounded test family.

Commands: CMake configure/build/CTest for original core target in normal,
GCC-sanitizer and Clang-sanitizer configurations, retaining N1 regression tests
when common build wiring changes. No retail permissions apply: generated inputs
only; no external writes/services. No sanitizer errors or unsettled core-owner
resources after repeated cycles. No gameplay/file representation change.

Commit the complete portable core behavior, plans, tests and evidence once these
gates pass. Do not mark N2 complete; slice 03 must reach actual GameLogic.

## Acceptance checkpoint

Completed 2026-10-07. Five core families pass normally and under GCC/Clang
ASan/UBSan with leak checks. Frozen combined renderer/core matrices pass 23/23
each. Evidence: `evidence/qa/N2-original-core.md`. Matching slice commit subject:
`delivery: port original core ownership to Linux`. Continue slice 02 in this same
N2 transaction; do not create a replacement governing plan.
