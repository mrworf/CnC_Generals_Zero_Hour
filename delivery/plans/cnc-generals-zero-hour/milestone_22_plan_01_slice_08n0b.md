# M22 plan 01 slice 08N0B: render-ready modeled volume geometry

## Goal and boundary

Depends on accepted 08N0A. A newly admitted modeled volume caster becomes
render-ready only after its exact source render object is scene-linked and
the map-owned volume buffer provider is available. Keep shadow admission,
scene publication and volume-geometry acquisition distinguishable: do not
move raw GPU allocation into an unvalidated constructor or treat a manager
entry as a drawable stencil shadow. Existing pre-map `BaseHeightMap` resource
reacquisition cannot ready casters created later in the map-object loop.

Provide a narrow per-caster readiness transition at the post-publication
owner boundary, using the accepted volume geometry slot builder and
transactional release. On allocation/upload/finalization failure, unwind
new geometry and the just-created caster/model/scene state without affecting
earlier accepted casters, terrain, bridge layers or world list heads. Preserve
release-before-scene-removal on replacement and teardown; a released or
foreign provider must reject instead of silently drawing an empty shadow.
No raw Direct3D fallback, synthetic retail surrogate, or broader shadow
type admission is authorized.

## Validation and commit boundary

A generated map/frame fixture proves ordered original terrain, tracks,
volume stencil and water Recording commands with nonzero geometry ranges;
two generations, model replacement and reset/re-entry end at zero owner and
device residual. Inject every relevant allocation/upload/draw failure,
missing/foreign buffer provider, detached render object and duplicate
finalization; assert immediate bounded rollback and clean retry. Preserve
existing decal and standalone volume source fixtures. Run the six complete
builds/suites, both focused strict host LSan groups, physical Vulkan, serial
LAN, ledger and diff checks. Commit one independent slice:
`delivery: M22 08N0B ready modeled volume geometry`.
