# M22 plan 01 slice 08H5B: extra-blend tile inventory

## Outcome and source boundary

Requires accepted slice 08H5A. Restore the source-owned inventory of visible
map cells whose accepted authored metadata requires a third terrain texture.
The inventory is part of height-map initialization and map-resource lifetime;
it is not a draw or shader implementation.

## Implementation and tests

Scan the bounded map in source row-major order through
`getExtraAlphaUVData`, publish positions only after the complete scan and
release them on every initialization failure, explicit free, destruction and
two-generation re-entry. Validate packed coordinate bounds and reject missing
query/atlas ownership without retaining partial state. Generated tests cover
zero, one and multiple extra blends, cliff-tagged entries, capacity bounds,
injected later initialization failure and retry.

Dynamic geometry and physical submission remain 08H5D. Run the full
proportional M22 gates and commit once as
`delivery: M22 08H5B own extra terrain blends`.

## Result

Complete. The source height-map owner scans the accepted bounded map in
row-major order and publishes an exact-capacity extra-blend inventory only
after the complete geometry transaction succeeds. Initialization failure,
explicit free, destruction and two-generation re-entry restore empty
inventory state. Generated zero-, one- and multiple-entry fixtures cover
ordering, cliff-tagged metadata and failure/retry. Dynamic extra-blend
material and submission remain 08H5C–08H5D. See the
[08H5B evidence](../../evidence/cnc-generals-zero-hour/milestone_22_slice_08h5b.md).
