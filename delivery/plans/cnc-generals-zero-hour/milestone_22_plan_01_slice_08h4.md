# M22 plan 01 slice 08H4: authored terrain blend and cliff queries

## Outcome and source boundary

Requires accepted slice 08H3. Restore the source CPU decisions that consume
the accepted authored metadata: base tile quadrant selection, blend and extra
blend UV/alpha orientation, custom edge-class ranges, cliff UVs/flip state and
texture-class lookup. These queries feed the already accepted terrain geometry
and public material path; they do not replace the original producer.

## Implementation and tests

Port the canonical bounded query semantics from the source branch while
retaining explicit range and missing-owner failures. Validate every cell and
record reference before indexing fixed arrays. Generated fixtures exercise
horizontal, vertical, both diagonal, inverted/flipped, extra blend, custom
edge and cliff cases plus malformed indices, missing atlases, retry and two
generations. Require exact aggregate Recording state/draw categories without
retaining authored identifiers or bytes.

Run focused and cumulative M22 gates and commit once as
`delivery: M22 08H4 restore terrain blend queries`.
