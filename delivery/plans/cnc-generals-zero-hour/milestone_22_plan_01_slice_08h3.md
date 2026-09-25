# M22 plan 01 slice 08H3: authored terrain atlas packing

## Outcome and source boundary

Requires accepted slice 08H2. Replace the one-class atlas guard with the
canonical bounded placement of all source base and edge tiles. Preserve class
contiguity, source order, four-pixel wrapped borders, public GPU texture
ownership and transactional publication of base, alpha alias and edge atlas.

## Implementation and tests

Compute placements without overlap within the fixed atlas width and bounded
power-of-two heights; reject capacity overflow, invalid class spans, missing
tiles and inconsistent edge ownership before publishing textures. On any
create/upload failure release every atlas handle and restore unpublished map
state. Generated tests cover multiple class widths, edge classes, exact
placements/uploads, failure injection, retry and two generations. This slice
does not implement blend/cliff UV selection or claim a complete terrain draw.

Run focused and cumulative M22 gates and commit once as
`delivery: M22 08H3 pack authored terrain atlases`.

## Result

Complete. Base and edge texture classes are packed independently into bounded
fixed-width grids, largest class first while retaining source class and tile
order. Exact authored base/edge bytes, placements and power-of-two atlas
heights are covered across failure injection, retry and two generations;
missing owners and capacity exhaustion fail before publication. Active
blend/cliff UV selection remains slice 08H4. See the
[08H3 evidence](../../evidence/cnc-generals-zero-hour/milestone_22_slice_08h3.md).
