# M22 plan 01 slice 08F5: map-owned no-model terrain prop dispatch

## Outcome and source boundary

Close the next missing source owner exposed by generated post-map scenario
construction. After pathfinder new-map and permanent-observer reveal,
`GameLogic::startNewGame` traverses authored map objects. A generated `PROP`
template without a draw module calls `W3DTerrainVisual::addProp`; the CPU
method is typed pending. Native `addProp` derives snow/night model conditions,
asset scale and the first draw module's best W3D model name. It calls
`m_terrainRenderObject->addProp` only when that name is nonempty. For the
generated no-draw prop, the source-equivalent result is no mutation.

## Bounded implementation and non-scope

Implement the exact CPU `W3DTerrainVisual::addProp` no-model branch with
source owner checks: published terrain visual/render-object identity, active
original GPU edge, initialized display/primary scene, attached map and the
accepted no-prop-buffer state. Require valid template and position; preserve
source model-condition and draw-module selection rather than declaring every
prop a no-op. Reject an active/nonempty model name before any prop mutation,
because the CPU terrain has no prop-buffer producer. Reject missing/foreign
owners and malformed arguments fail-closed. Do not allocate a prop buffer,
implement modeled prop rendering, alter 08F4 shroud notification, admit the
08F construction selector, replace source object traversal, or access retail
providers/content. Keep the accepted default, 08D and 08F0 boundaries intact.

## Tests, dependency and commit

Requires accepted 08F4. Use generated read-only map and template fixtures to
show no-draw/no-model success, active-model rejection, missing or foreign
owner/provider removal, malformed arguments, repeat/retry, two generations,
and zero aliases/Recording resources at teardown. Verify source identity and
the source-provider/ledger gates. After production changes, run focused
GCC/Clang, all six configured builds and canonical non-GPU/non-LAN/non-retail
test suites, canonical sanitizer `detect_leaks=0`, strict host LSan both,
physical Vulkan controls, serial LAN in all six, and `git diff --check`.
Commit one 08F5 production/test/evidence slice; only then resume 08F's
selector/water/object construction. This plan alone does not accept 08F5.

## Implementation choice before code

Reuse the accepted map/shroud owner's side-effect-free
`BaseHeightMapRenderObjClass::canNotifyShroudChanged()` as the no-prop-buffer
readiness predicate; require its exact visual/global terrain and display
aliases and live map identity first. This is an ownership check, not a shroud
notification. A generated no-draw `FixtureProp` and a separate generated
W3DModelDraw prop template distinguish the no-model and active-model paths.
Extend the existing generated terrain-visual map probe, which already covers
failed map loads, retry and two generations, rather than creating a second
map lifecycle. No new CTest registration is needed.

This is an internal original CPU owner method, not an externally authorized
entry point. Its admission boundary is the exact published display/terrain
and source-map identity. A missing global-data provider must reject before
reading weather/time model conditions.

## Result

Complete. The CPU method selects the native first-draw model result and
accepts only the exact map-owned no-model branch; a selected model remains
typed pending. Generated tests cover malformed input, provider removal,
pre-map rejection, modeled rejection, retry, two generations and teardown.
The [08F5 evidence](../../evidence/cnc-generals-zero-hour/milestone_22_slice_08f5.md)
records all final-state acceptance gates. 08F construction remains pending.
