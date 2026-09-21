# M22 plan 01 slice 05A: original index contract and first buffer commands

## Outcome and dependencies

Requires 04C and M14. The public `GpuDevice` preserves the original W3D
16-bit index width, first index and base vertex; the recording and SDL_GPU
backends agree, without breaking existing default 32-bit clients. Original
rigid category render dispatches its first source-issued physical vertex
and index buffer commands through a narrow scoped `GpuDevice` translator.
No texture, shader, complete pass or frame is claimed until 05B–06.

## Source and device boundary

Original `MeshModelClass`/`DX8RigidFVFCategoryContainer` own geometry,
FVF, buffer bytes, indexing, category and order. The translator only
creates/uploads/destroys device handles for those exact original bytes at
their original physical call sites. No rebased/copied geometry workaround
for the missing index contract, fake D3D SDK or parallel pass scheduler.
Unsupported sorting/skin/dynamic physical paths remain typed until their
original commands are translated in 05B–06.

## Test and failure contracts

Owned W3D rigid fixture proves source-selected category emits identical
vertex/index bytes through the recording device and releases all resources
after success/reset/injected create/upload failures. Negative no-session,
unsupported kind, stale-buffer, nested-session, wrong/overflowing index
format and bounds. Recording trace proves index width/offset/base; SDL_GPU
maps to SDL's index size and draw offsets. Source identity/provider removal,
unmixed ABI, GCC/Clang full suites, focused sanitizers and ledger freshness.
The first later original texture/material command remains a typed failure;
it is not a completed pass.

## Commit boundary

One independently validated commit: `delivery: M22 slice 05A translate original buffer edge`.
