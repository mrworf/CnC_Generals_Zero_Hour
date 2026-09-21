# M22 plan 01 slice 04C: original mesh and WWShade CPU pass graph

## Outcome and dependencies

Requires 04B. Original HLOD/mesh, DX8 renderer, WWShade and WW3D source
owners schedule visibility, frustum, sort, alpha/shadow overrides, base and
additional passes, skin/decal/material, texture grouping and ordered flush
before a typed physical edge. No recording frame is claimed until 05.

## Source closure and ownership

Restore reached original `WW3D::Render`/`Flush` in canonical `ww3d.cpp`,
including camera, DX8 mesh queue, `SHD_FLUSH`, static-sort and sorting
renderer; replace duplicated static defaults from `ww3d_cpu_state.cpp`
with one mutually exclusive source configuration. Retain canonical
`hlod.cpp`, `mesh.cpp`, `meshmdl.cpp`, `dx8renderer.cpp`,
`dx8polygonrenderer.cpp`, shader/material/mapper and reached original
WWShade sources. Select actual WWShade variants through owned and read-only
retail encounters, not an assumed entire legacy inventory. Physical
`dx8wrapper.cpp`/D3DX work belongs to 05; do not fake a Direct3D SDK or
duplicate pass scheduling in an adapter.

## Test and failure contracts

Drive original GameClient traversal through owned multi-pass, opaque/alpha,
hidden, shadow, skin/decal variants and sampled read-only retail family
closure. Assert original pass order and source-derived shader, texture,
material and transform identity; reject missing shader/material and
unsupported sort/override states; verify first physical command fails
typed, teardown exact once, no mixed ABI, provider removal, GCC/Clang
full suites, focused sanitizers and ledger freshness. Physical translation,
retail recording and validation-layer Vulkan remain 05–08 acceptance.

## Commit boundary

One independently validated commit: `delivery: M22 slice 04C restore original mesh pass graph`.
