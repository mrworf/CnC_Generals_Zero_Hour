# M22 plan 01 slice 07FD0: generated original decal-shadow request owner

## Outcome and dependency

Requires 07FA. Add the smallest generated original `ThingTemplate` and
drawable path that reaches `W3DModelDraw::allocateShadows` (or the equivalent
`W3DDebrisDraw` source owner) with `SHADOW_DECAL`. The owner must invoke the
actual `TheW3DShadowManager->addShadow(m_renderObject, &shadowInfo)` call and
later its native release/removal edge. This is a prerequisite witness only:
the manager still rejects all active shadow requests.

## Scope

Use an owned generated model/template/map fixture and an initialized original
display/terrain visual. Prove owner construction, populated source
`ShadowTypeInfo`, manager request rejection without publication, native owner
removal, and clean re-entry. Contrast the default `W3DDefaultDraw` volume
request and unsupported volume/projection/decal variants so the later 07FD
manager can admit only the witnessed decal input.

## Non-scope

No active shadow-manager resources, queue, scene pass, rendering, pixels,
retail models, particles, water changes, or production factory switch. Those
belong to 07FD and later slices.

## Validation and commit

Add a dedicated generated-fixture source probe with positive construction and
removal, negative type/manager/map controls, two generations and zero owners.
Run focused GCC/Clang, identity/provider/ledger and required acceptance before
one independent `07FD0` commit. Update the governing index and evidence.

## Delivered evidence

The dedicated `original_w3d_shadow_source_owner` probe creates the owned
read-only `LogicFixture` with the exact `W3DModelDraw` decal metadata, then
uses its real `allocateShadows` source call. The still-pending manager rejects
that call before publication, while direct volume/projection/none controls are
recorded only as negative type controls. The probe also verifies two native
manager generations, `GameClient::destroyDrawable` removal, and no Recording
resources. See
`delivery/evidence/cnc-generals-zero-hour/milestone_22_slice_07fd0.md`.
