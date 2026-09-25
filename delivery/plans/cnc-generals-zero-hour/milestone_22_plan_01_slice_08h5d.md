# M22 plan 01 slice 08H5D: extra-blend terrain submission

## Outcome and source boundary

Requires accepted slices 08H5A through 08H5C. Restore the bounded source
`HeightMapRenderObjClass::renderExtraBlendTiles` consumer and schedule it
after the accepted base-terrain shader reset. It builds only inventoried
third-layer cells and submits them with the accepted road-base alpha material.

## Implementation and tests

Build exact source-order XYZNDUV2 vertices and 16-bit indices from authored
extra UV/alpha data, including source cliff-height diagonal flip selection.
Bind the alpha atlas, issue the exact bounded draw, reset shader/dynamic
bindings on every exit and update the visible count only after successful
submission. Hidden, disabled, empty, unsupported global mode and missing
owner paths produce no draw or fail before mutation as appropriate. Generated
tests cover multiple entries, both topologies, cliff override, draw failure,
retry, teardown and two generations; they also assert base-pass then
extra-pass order.

Cloud/light-map/noise, dynamic lighting and partial-map mutation remain
guarded. Run the full proportional M22 gates and commit once as
`delivery: M22 08H5D submit extra terrain blends`.
