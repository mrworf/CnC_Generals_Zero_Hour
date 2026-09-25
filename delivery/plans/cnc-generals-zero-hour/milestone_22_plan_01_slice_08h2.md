# M22 plan 01 slice 08H2: authored terrain source tile sets

## Outcome and source boundary

Requires accepted slice 08H1. Generalize the canonical `readTexClass` owner
from one uncompressed 64x64 image to the original bounded base/edge texture
class contract: a class declares a contiguous tile span and square width, and
the authored TGA supplies the matching source tiles in source order. Preserve
terrain-name lookup, filesystem ownership and `TileData` mip generation.

## Implementation and tests

Decode only source-supported true-color TGA forms and orientation rules needed
by the canonical owner, including bounded uncompressed/RLE input. Validate
dimensions, depth, packet lengths, class span/width, cumulative capacity and
short reads before publishing any tile. Roll back all tiles opened for the
current class and all prior classes when map construction fails. Do not pack
an atlas, synthesize absent classes or retain retail-derived bytes.

Generated tests cover multi-tile base and edge classes, ordering/orientation,
mips, RLE packets, two-generation teardown, and missing/malformed/oversized or
overlapping inputs. Run the full proportional M22 gates and commit once as
`delivery: M22 08H2 own authored terrain tile sets`.
