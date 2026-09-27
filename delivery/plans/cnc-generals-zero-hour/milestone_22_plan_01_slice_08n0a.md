# M22 plan 01 slice 08N0A: constructor-order modeled volume-shadow admission

## Approved slice08 shared modeled-shadow publication correction

Use this same post-scene/user-data/module callback boundary for initial modeled
decals, and the existing post-scene/user-data replacement boundary for decals as
well as volumes. Do not admit in-flight manager identities or change Windows
behavior/layout. The manager's single-type/duplicate/published-model checks stay
strict; only obsolete global decal-volume exclusivity is removed under 07FD.
Failed candidate allocation/upload/admission withdraws caster/render/scene state,
preserves accepted siblings, and permits deterministic clean retry. Existing
volume readiness and explicitly empty authored state semantics remain unchanged.

## Goal and boundary

After accepted 08M0, a generated modeled Drawable with an enabled volume
shadow may complete its original constructor and publish one exact scene-
linked render object and one bounded shadow manager entry. Preserve the
ordinary Drawable/ThingFactory atomic publication contract. This slice owns
only source ordering, identity and rollback; it does not claim volume
geometry or a rendered shadow frame.

Approved slice08 callback correction: an explicitly empty authored
ModelConditionInfo is a valid no-render-object/no-caster state. The post-create
callback must skip volume readiness only for that exact empty state with no
shadow. Nonempty model data with a missing render resource remains fail-closed;
every actual scene-linked model still requires strict caster admission/readiness.
Do not pre-create placeholder geometry or weaken the manager's NULL rejection.
This is a bounded correction within the existing owner, delivered with slice08.

Audit `W3DModelDraw::setModelState` from render creation/validation through
scene add, user-data binding and module publication. Its initial constructor
call occurs before the module pointer is stored, so the existing published-
module shadow guard correctly rejects an immediate request. Defer that
request to the source `onObjectCreated` callback, after the module array and
render scene link are published; state replacement requests its shadow only
after the new render object joins the scene. Move `DrawableInfo`'s back-
pointer assignment before module creation, so scene user-data never points
to an unbound identity. Keep `W3DShadowManager`'s existing published-model
guard intact, including its foreign/stale model and duplicate negatives;
do not use an in-flight token, selector bypass or unconditional acceptance.

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
