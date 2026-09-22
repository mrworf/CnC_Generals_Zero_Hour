# M30 slice 03: physical shader, pipeline and ordered draw behavior

## Goal and outcome

Project-owned bgfx shaders and source-defined state produce ordered indexed/base-vertex and point/UI/world/effects draws through the engine device with correct bindings, format/alpha/depth/stencil semantics and explicit unsupported-state errors.

## Scope / non-scope

Own shader binary/manifests loading, program/pipeline cache, vertex layouts, buffers, stage textures/uniforms, sampler state, draw order, diagnostics and physical pixel tests. Presentation and full suite are slice 04. No new material or original scene behavior.

## Dependencies / order

Slices 01–02 resources and view/target sequencing; M29 shader family/descriptor manifests; M7–M10/M14 accepted CPU contracts.

## End-to-end behavior and state

Caller creates stage shaders and pipeline → pipeline validates source state/layout/target formats → stage bindings resolve to public bgfx uniforms/textures → indexed draw submits to the current ordered view after prior clear/draw commands → readback shows expected pixels. Destroy retires resources and cache entries. Unsupported required state fails before submit; no draw is silently dropped.

## Authorization / validation

No permission surface. Validate shader file provenance, stage/manifest/slot bounds, texture format and sampler, uniform range, index/base vertex bounds, point-size and stencil state. Keep owned engine handles opaque; no private backend API escapes.

## Implementation surfaces

Expected: bgfx device implementation, shader registry/manifest integration as needed, focused bgfx draw tests, renderer docs/ledger.

## Tests and commands

- Positive: shader families, UI/world/effects representative pixels, indexed/base-vertex, point size, alpha/depth/stencil and four-block equivalent binding.
- Negative: missing/wrong shader, stale resource, invalid index/bindings, unsupported topology/state/format reject without proxy behavior.
- Run focused renderer shader/producer tests, physical draw/readback tests and relevant M14 comparison tests with explicit Vulkan validation on RTX.

## Acceptance / commit boundary

One commit includes exercised draw/pipeline/binding behavior, positive/negative tests and evidence; no untested skeletal pipeline.
