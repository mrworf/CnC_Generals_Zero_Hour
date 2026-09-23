# M22 plan 01 slice 07FE: active original particle and smudge effects

## Outcome and dependency

Requires 07F9. Close the reached original effect route in which particle
rendering queries and renders `W3DSmudgeManager`; provide the source-owned
active-set/resources/pass behavior required by the selected scene-family
contract. The empty smudge owner and false hardware-support answer do not
close an active particle/effect route.

## Boundaries and acceptance

This follows accepted 07FEA.  The child admits exactly one generated
`SmudgeSet` with one smudge, its five source vertices and the native twelve
fixed indices; multiple sets/smudges and invalid size/opacity fail closed.

Use generated effect inputs only. Record original particle/smudge ownership,
frame count, render-to-texture/pass ordering where source-selected, absence,
malformed/unsupported effect rejection, injected failure/retry and teardown.
Do not use a generic effects recorder in place of original producers. Water
and shadows are independent siblings; pixels and retail remain deferred.
The coupled generated-map witness orders terrain, tracks, water, original
particle dispatch and the bounded smudge batch.
