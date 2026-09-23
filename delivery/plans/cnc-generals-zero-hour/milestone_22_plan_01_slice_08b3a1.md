# M22 plan 01 slice 08B3A1: source buffer-slot CPU closure

## Goal

Build and exercise the smallest portable source-compatible
`W3DBufferManager` route that a later original volume geometry owner can use.
It is an allocator/Recording ownership slice; it does not construct a volume
or admit `SHADOW_VOLUME`.

## Scope and constraints

Requires 08B2.  Link the original buffer manager into the full-draw target
with CPU FVF layout identifiers and portable source bounds.  It may use only
the existing public renderer edge and CPU-owned DX8 wrapper buffers.  Raw
Direct3D, private Vulkan interfaces, volume/decal/projected owner admission,
retail data, and pixel assertions are prohibited.

## Behavior, validation, and commit boundary

A generated Recording probe allocates supported position/index slots, writes
and uploads a bounded triangle, retries create/upload failure, rejects
invalid/overflow format or size and foreign/duplicate releases, recreates
resources, removes the provider, and completes two generations at zero
ownership.  Run focused plus proportional six-build/host acceptance, update
the ledger/evidence/index, and commit this slice independently.
