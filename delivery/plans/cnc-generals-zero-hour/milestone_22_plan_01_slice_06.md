# M22 plan 01 slice 06: original interleaved pass graph

## Outcome and dependencies

Requires slices 04C–05B2B2B3B2. Original HLOD/mesh, DX8 renderer, WW3D and enabled
WWShade owners perform every reached interleaved CPU decision and translated
physical operation in their authored source order. The original
`DX8TextureCategoryClass::Render` body must execute its source-ordered
texture/material/shader, mesh transform, alpha override and draw calls;
its reached original light-environment, world identity/transform and normal
normalization decisions must execute here rather than being inferred from
independent B3B method probes;
pending 05B2B2A/B1/B2/B3A/B3B1/B3B2 state binds only at this actual pass/draw, including
pass-time bind failure negatives. Full fixture frames and
sorting/static-sort/decal/material/skin flushes record through the same
source path. No GameClient display or retail-frame claim yet.

## Reached source closure

Restore source-owned `WW3D::Render`/`Flush`, rigid and skin FVF category
passes, original texture/material/shader combinations, alpha/shadow and
additional-pass overrides, decal, procedural material, static sort and
sorting renderer. Preserve actual `USE_WWSHADE` configuration: when enabled,
register and render reached original SHDMESH/legacy shader-family routes;
when disabled, prove authored no-op macro and required material/effect
semantics through regular original scheduler. Read-only retail aggregate
classification informs enabled family coverage without storing private
names or bytes. Translate only at physical device edges; never reorder
source calls or inject adapter-owned pass authority.

## Test and failure contracts

Owned multi-pass W3D fixtures demonstrate exact source order, shader,
texture, material, transforms, visibility/alpha/shadow, skin/decal and
static/sorting flushes on recording `GpuDevice`; compare branch selection
and negative controls for supported/unsupported WWShade configuration.
Reject missing required shader/material, unsupported state, wrong ordering,
partial frame and injected device failures. Teardown/reset/retry releases
original and device owners. Check unmixed ABI, provider-removal, GCC/Clang
full suites, focused sanitizers and ledger. Derived shadow/terrain/effects
belong to 07 before retail acceptance.

## Commit boundary

One independently validated commit: `delivery: M22 slice 06 restore original interleaved passes`.
