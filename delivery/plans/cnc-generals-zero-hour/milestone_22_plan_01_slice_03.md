# M22 plan 01 slice 03: original DX8 and WWShade translation to GpuDevice

## Goal and observable outcome

The original render-object and GameClient producers from slices 01–02 submit their reached DX8Wrapper/WWShade resource, state, pass, and draw operations through the public `renderer::GpuDevice` contract. A recording device witnesses actual original producer identity and complete state; the adapter translates but owns no authoritative scene content.

## Scope

- Translate reached original vertex/index/texture resources, uploads, transforms/camera, material/blend/depth/cull/color/alpha/fog/light/shadow state, targets, multipass ordering, and draws at the DX8/OS device edge.
- Bind the existing Linux display/view traversal to the translation service while preserving original GameClient traversal and lifecycle.
- Reconcile each reached RC-012 deferred ledger operation; unsupported required state fails before a successful frame marker.

## Validation and error handling

- Recording integration begins at the production original GameClient entry, traverses actual concrete modules and original render objects, and ends in balanced explicit commands.
- Tests reject missing/malformed assets, unsupported state, absent adapter/provider, stale handles after reset, nested/incomplete passes, and injected allocation/upload/draw failures.
- Markers identify original translation units/classes and source-derived model/material/`ShaderClass` state, not just adapter activity.
- Source identity and provider removal cover original GameClient, WW3D, WWShade/DX8 translation, and `GpuDevice` providers.

## Acceptance criteria

- Every reached translated value is derived from original producer objects after their state decisions execute.
- Recording has balanced passes/resources and zero retained ownership after normal, reset, re-entry, and every failure path.
- No Direct3D library, private Vulkan API, adapter-owned geometry/material, generated scene, or silent null-success path is linked or accepted.

## Commit boundary

Commit original device translation, recording tests, ledger update, slice status, and evidence as `delivery: M22 slice 03 translate original rendering`.
