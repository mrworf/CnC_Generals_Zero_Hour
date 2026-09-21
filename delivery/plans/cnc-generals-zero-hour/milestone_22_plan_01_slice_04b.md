# M22 plan 01 slice 04B: original CPU buffers and mesh registration

## Outcome and dependencies

Requires 04A. Canonical original `DX8VertexBufferClass`,
`DX8IndexBufferClass`, `DX8MeshRendererClass::Register_Mesh_Type`,
`DX8FVFCategoryContainer`, and original polygon/category owners preserve
source allocation, lock, append, grouping, lifetime and source-derived
vertex/index/UV/material identity before physical drawing. No completed
pass or successful frame is claimed.

## Source and test boundary

Inspect only reached canonical original `dx8vertexbuffer.cpp`,
`dx8indexbuffer.cpp`, `dx8renderer.cpp`, `dx8polygonrenderer.cpp`,
`meshmdl.cpp`, material and texture owners, adding actual reached link
dependencies. Separate CPU buffer storage from original physical D3D calls
within the original source or shared canonical implementation, without
proxy geometry, no-op replacements, fake D3D headers or a second manager.
Tests drive authored rigid and reached skinned/UV variants through original
registration, compare original vertex/index/layout and category decisions,
reject invalid FVF, stale locks, missing material/texture and required
unsupported state, and exercise reset/retry and exact ownership. Gate on
provider-removal, ABI/link-map exclusivity, GCC/Clang full suites, focused
sanitizers and ledger freshness. Remaining physical operations fail typed.

## Commit boundary

One independently validated commit: `delivery: M22 slice 04B preserve original mesh buffer ownership`.
