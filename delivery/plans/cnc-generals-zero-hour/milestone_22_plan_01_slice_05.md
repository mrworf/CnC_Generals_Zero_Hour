# M22 plan 01 slice 05: original DX8/WWShade outputs to GpuDevice

## Outcome and dependencies

Requires slice 04. Original WW3D/WWShade pass, shader, texture, geometry and
state outputs cross the narrow physical edge into public `renderer::GpuDevice`.
The translator owns resource handles and uploads, not authoritative mesh,
scene, pass, shader/material, or draw-order decisions. A recording device
witnesses actual original producer markers.

## Device closure

Translate reached original DX8 wrapper vertex/index/texture resources,
uploads/locks, camera transforms, original shader/material and texture-stage
combinations, light/fog/shroud, blend/depth/cull/alpha, targets, multipass
and draws. Do not compile `dx8wrapper.cpp` with Direct3D SDK headers or
substitute a fake device. Characterize any public-GpuDevice capability gap
against the original call and M14 backend before changing the contract.
Unsupported required state rejects the frame before success marking.

## Test and failure contracts

Actual original HLOD/mesh traversal yields balanced recording passes,
buffers, textures, samplers, shaders, pipelines and draws with original
source-derived state; reject wrong shader/material/texture, nested or
incomplete passes, malformed uploads and injected create/upload/draw
failures. All handles return to zero after success, reset and each failure.
Provider removal covers original pass owners, translator and `GpuDevice`.
Only slice 06 may claim GameClient display/terrain/shadow/2D integration.

## Commit boundary

One independently validated commit: `delivery: M22 slice 05 translate original GPU edge`.
