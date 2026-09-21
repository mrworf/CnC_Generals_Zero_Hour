# M22 plan 01 slice 05B2B2B1: original mapper and material state

## Outcome and dependencies

Requires accepted 05B2B2A. The original `VertexMaterialClass::Apply` and
`Apply_Null` call their original mapper `Apply` methods and preserve source
diffuse/ambient/specular/emissive/power, lighting/color source and UV stage
settings. Original `DX8Wrapper::Set_Texture/Set_Material/Set_Shader`
ref-counted delayed state and its pending-change flags survive until the
actual source draw. Port only the reached nondevice portion in canonical
WW3D2/DX8 sources, no surrogate material manager or renderer claim.

## Decisions and deferred physical edge

Original mapper sources own animated UV transforms and coordinates,
including owned screen and linear-offset paths and every additional
read-only retail family actually reached. Preserve original update time,
matrix, index and transform-flag choices before a typed device boundary;
never fabricate a static UV or drop a requested mapper. Original material
source owns null/reset and light/color state. Add only device-edge pending
typed state, with reference ownership and generation invalidation matching
original delayed DX8Wrapper semantics; physical GPU uniforms/pipeline and
draw belong B3/06. Unsupported mapper/UV modes fail with family identity
and are escalated if retail-required.

## Acceptance and negatives

Original owned W3D mesh/material/texture chunks select mapper families,
opacity, lighting, color source, UV index/transform and null reset. Drive
real original update ticks and inspect ordered source state on Recording.
Read-only retail mapper/material aggregate classifies required families
without paths/names/bytes/hashes. Test unsupported modes, absent material,
missing mapper dependencies, injected edge failure, replay/reset/retry,
source refcount and bounded teardown; no dummy pass/draw. Preserve the
M20 canonical ABI and M21 contracts; provider-removal/link-map, ledger,
GCC/Clang full suites, sanitizers, classification and `git diff --check`
pass. B2/B3 and 06–09 still own shader and complete frame acceptance.

## Commit boundary

One independently validated commit:
`delivery: M22 slice 05B2B2B1 preserve original material mappers`.
