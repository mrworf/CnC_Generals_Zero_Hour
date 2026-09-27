# M22 plan 01 slice 07FD: active original shadow route

## Approved slice08 modeled-shadow correction

The redacted continuation reaches an actual modeled `SHADOW_DECAL` while volume
shadows are enabled. The existing source already orders decals in its non-stencil
pass before volumes in the later stencil pass. Remove only the obsolete global
decal-versus-volume exclusivity; each template still owns one exact shadow type.
Defer initial modeled decal admission until scene/user-data/module publication,
and replacement admission until its new render is scene-linked/user-data-bound,
using the existing 08N callback/lifecycle boundary. Preserve old-shadow release
before old-scene removal and constructor/replacement cleanup on rejection.
Direct/prepublication, duplicate, foreign, detached, missing texture/size/resource
and malformed requests remain closed. Generated actual-factory constructor,
replacement, fault/retry and mixed decal-before-volume frame controls must prove
exact surviving identities and total-zero teardown in two generations. Deliver
this correction with active slice08, not a new milestone or shadow family.

## Outcome and dependency

Requires 07FD0 and the accepted original scene mesh route. Implement the
smallest source-requested shadow type: bounded `SHADOW_DECAL` requests, which
`W3DDebrisDraw` and `W3DModelDraw` submit through
`W3DShadowManager::addShadow`. `W3DDefaultDraw` is the contrasting source
volume caller, not a decal caller. Preserve volume and projection-derived
routes as typed pending. The disabled-shadow owner is not an accepting
substitute once a source drawable requests the admitted decal.

## Boundaries and acceptance

Use the 07FD0 generated `LogicFixture` and its actual `W3DModelDraw` render
object, with an owned loaded map. Under the existing CPU-only compilation
branch, admit exactly `SHADOW_DECAL` when shadow decals are enabled, the
manager is the published primary-display owner, and the request render object
is the currently published `W3DModelDraw` object in that primary RTS scene.
The source `allocateShadows` call owns admission; a direct manager request is
only a negative control.

Implement a bounded manager-owned decal quad through the original DX8 buffer
and source-state path. It has one four-vertex/six-index source allocation per
accepted caster, queues only after map-terrain traversal, and uses the source
`DoShadows` position before the accepted tracks/water static route. The CPU
scene must tolerate that registered caster only as a non-rendered source owner;
arbitrary, duplicate, detached or foreign render objects remain rejected.

The generated-map Recording probe must cover exact owner metadata, manager
publication, source buffers/range, terrain-to-shadow-to-water ordering,
disabled decals, volume/projection/default/none, duplicate, foreign and
post-detach requests. It must inject create/upload/draw failure, demonstrate
rollback/retry, source `releaseShadows` removal, two generations and process
re-entry, and prove zero Recording resources at teardown. Tracks/water stay
active compatibility siblings; particles remain inactive. Run the full
six-configuration acceptance matrix, host leak/Vulkan proportional checks,
and serial LAN before the independent 07FD commit.

Physical pixels, retail input, volume/projected/alpha/additive shadow variants,
trees, and particles remain outside this child.
