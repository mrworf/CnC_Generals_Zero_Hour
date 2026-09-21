# M22 plan 01 slice 05: first original DX8 commands to GpuDevice

## Outcome and dependencies

Requires slice 04C and M14. Original DX8 wrapper physical resource and
draw-state calls reached first in original category/material traversal cross
into public `renderer::GpuDevice`. The translator owns resource handles and
uploads, not authoritative mesh, scene, pass, shader/material or ordering.
The first source-issued calls are witnessed by a recording device; later
interleaved original decisions are completed in 06, not simulated here.

## Device closure

Translate reached initial DX8 wrapper vertex/index/texture resources,
uploads/locks, camera state, first shader/material/texture-stage commands,
blend/depth/cull/alpha, targets and draw entry. Do not compile
`dx8wrapper.cpp` with Direct3D SDK headers or substitute a fake device.
Characterize public-GpuDevice capability gaps against original source and
M14 backend. Unsupported required state rejects before success marking.

## Test and failure contracts

Original W3D fixture dispatches first source physical operations into
balanced recording resources; assert source-derived state and reject wrong
shader/material/texture, malformed uploads and injected create/upload/draw
failures. All handles return to zero after success/reset/failure. Provider
removal covers original caller, translator and `GpuDevice`. Do not claim a
complete multi-pass graph or GameClient integration; both depend on 06–07.

## Commit boundary

One independently validated commit: `delivery: M22 slice 05 translate original physical entry`.
