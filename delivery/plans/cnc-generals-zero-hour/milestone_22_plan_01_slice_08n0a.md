# M22 plan 01 slice 08N0A: in-flight modeled volume-shadow admission

## Goal and boundary

After accepted 08M0, a generated modeled Drawable with an enabled volume
shadow may complete its original constructor and publish one exact scene-
linked render object and one bounded shadow manager entry. Preserve the
ordinary Drawable/ThingFactory atomic publication contract. This slice owns
only source ordering, in-flight identity and rollback; it does not claim
volume geometry or a rendered shadow frame.

Audit `W3DModelDraw::setModelState` from render creation/validation through
shadow request, scene add, user-data binding and module publication. Its
initial constructor call occurs before the module pointer is stored, so a
published-module lookup cannot authorize that call. Admit the in-flight
volume request only after exact scene registration and DrawableInfo binding,
with the explicit `Drawable*` registered under the current GameClient, the
render object's user-data identifying that same Drawable, the accepted
volume-capable template/manager and no duplicate caster. Preserve the
existing published-module guard for later `allocateShadows` calls without an
explicit Drawable. Reject foreign/stale draw, scene, model, manager and
missing/null shadow info before mutation; do not use a selector-only bypass.
`DrawableInfo` currently gains its back-pointer only after draw-module
construction; move that identity assignment before module creation so the
in-flight user-data check never reads an unbound/null owner.

On any failure before the W3DModelDraw constructor can publish its module
pointer, release the newly created shadow, detach the render object, release
its reference and leave no manager/list residue. Handle state replacement and
`nukeCurrentRender` in source order: old shadow release precedes old render
removal; failed replacement cannot leave an orphan caster or scene object,
and a subsequent clean retry is deterministic. Do not invoke gameplay
destroy hooks or reset the world to hide a partial owner.

## Validation and commit boundary

Use project-owned generated model/template assets with a volume shadow.
Positive initial constructor, replacement and removal assert exact
Drawable/model/user-data/scene/caster identities and zero residuals after
two generations. Inject scene-add, shadow admission and later model-state
failure; observe immediate pre-teardown residuals, provider removal, and
retry. Negative foreign/mismatched draw, model, scene and manager requests
remain closed, as do default and nonvolume routes. Existing generated
construction and shadow/decal controls remain green. Run six complete
GCC/Clang Debug, Release and ASan+UBSan builds and canonical nonretail suites,
focused strict host LSan, physical Vulkan 2/2, serial LAN 4/4 each six,
dependency ledger and diff checks on final source. Commit one independent
slice: `delivery: M22 08N0A admit in-flight volume shadow owner`.
