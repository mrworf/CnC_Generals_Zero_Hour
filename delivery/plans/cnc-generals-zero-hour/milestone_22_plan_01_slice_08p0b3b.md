# M22 plan 01 slice 08P0B3B: atomic tree geometry and Recording resources

## Goal and boundary

After 08P0B3A, compose accepted model references and bounded atlas pixels
with native tree mesh geometry. Preserve the source XYZNDUV1 bytes: position
with model offset, random scale and rotation; UV clamp/atlas transform;
normal-driven diffuse lighting; and normal slots carrying sway, darkening
and base height. Preserve triangle winding, per-tree index identity and
relocation. Reject missing UVs, malformed polygons/nonfinite transforms and
oversized geometry before publication. Enforce 30000-vertex/60000-index
source budgets and the 16-bit index range; do not use proxy geometry.

Create and upload source wrapper-backed Recording vertex, index and atlas
resources in source order and exact active device generation. Stage a full
replacement bundle while existing registry/type/instance/atlas/GPU owners
remain accepted. Use 08P0B3A's local client RNG preview for candidate random
scale/sway; discard it on every failure and commit it only after all fallible
work and registry storage are ready. Geometry/capacity, missing provider,
wrapper create/upload, atlas upload, reset and teardown failures unwind the
candidate in reverse order with immediate pre-teardown residual zero. Then
publish atomically; repeated update/removal preserves identity and frees
obsolete resources exactly once. The physical factory remains closed;
frame/shroud/shadow behavior remains 08P0C.

## Validation and commit

Generated textured mesh/TGA fixtures assert vertex/index/UV and Recording
resource bytes/counts; existing no-UV mesh is a negative control. Inject
each source/device boundary, compare failed admission followed by retry
against a fresh equivalent client RNG generation, inspect immediate
pre-teardown residuals, and retry in the same/fresh device generation. Move
08P0A's registry capacity/identity controls to generated ready assets
without weakening their negatives. Run six complete builds and canonical
nonretail suites, focused strict host LSan, physical Vulkan
display/bootstrap/map controls, serial LAN 4/4 all six, ledger and diff
checks on final source. Commit one slice:
`delivery: M22 08P0B3B publish tree resources atomically`.
