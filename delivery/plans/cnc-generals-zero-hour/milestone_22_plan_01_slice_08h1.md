# M22 plan 01 slice 08H1: authored visual terrain metadata

## Outcome and source boundary

Requires accepted slice 08G2. The redacted retail continuation has crossed
map-local INI loading and reaches the canonical visual-map reader, where the
current Linux branch accepts only one bitmap tile, one blend sentinel, one
cliff sentinel and one texture class. Restore the bounded source version-8
metadata owner for multiple base and edge texture classes, blend records,
cliff records, cell indices and cliff-state bits. This slice parses and owns
metadata only; texture decoding, atlas placement and render submission remain
later dependencies.

## Implementation and tests

Use the original `WorldHeightMap` record order and fixed-capacity arrays.
Validate every count, class span, tile/blend/cliff index, record flag and chunk
tail before publication; overflow, overlap, truncation, bad flags and invalid
references fail closed and retire every partially allocated array. Preserve
the accepted generated one-class map and logical-only route.

Generated fixtures cover multiple unresolved base/edge classes and nonzero
blend/cliff records, boundary counts, malformed permutations, two generations
and provider removal. Retail roots are not needed for implementation; a later
redacted probe may retain only fixed stage/category counts. Run focused
GCC/Clang, sanitizer/LSan, canonical nonretail suites, physical Vulkan, serial
LAN, ledger validation and `git diff --check`.

One commit: `delivery: M22 08H1 own authored terrain metadata`.

## Result

Complete. The canonical CPU branch now owns bounded version-8 multi-class
base/edge metadata, active blend and cliff records, cliff-state bits and all
cell references. Generated two-generation coverage proves accepted metadata
and rejects invalid counts, gaps, overlap, references, flags, directions,
nonfinite values and tails without retained allocations. Image tile-set
decoding, atlas placement and blend/cliff query semantics remain 08H2–08H4.
See the [08H1 evidence](../../evidence/cnc-generals-zero-hour/milestone_22_slice_08h1.md).
