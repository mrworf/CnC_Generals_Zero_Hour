# M22 plan 01 slice 01: original WW3D CPU asset and render-object graph

## Goal and observable outcome

The original `W3DAssetManager`/`WW3DAssetManager` path reads a project-owned byte-exact W3D corpus through the original file/chunk interfaces and constructs the same CPU mesh, HLOD, hierarchy, animation, texture-reference, vertex-material, mapper, and `ShaderClass` state later consumed by GameClient rendering. No GPU device or adapter-owned geometry/material authority is involved.

## Scope

- Replace the bootstrap-only `zh_w3d` provider with the minimal transitive original source closure reached from `W3DAssetManager`, `WW3DAssetManager`, registered prototype loaders, and model creation.
- Preserve original `ChunkLoadClass`, mesh/HLOD/hierarchy/animation parsing, texture identity, material passes, mapper parameters, `ShaderClass` values, render-object cloning, reference counting, and reverse teardown.
- Port only OS/compiler boundaries required by those sources. Direct3D allocation/draw calls remain a typed unavailable edge until slice 04; they may not be reported as successful.
- Maintain a checked closure manifest tying each compiled original translation unit to a reached symbol or explicit runtime witness; do not bulk-add the 244-file WW3D2 directory.

## Entry point and end-to-end behavior

A focused original-source integration executable mounts owned W3D/texture bytes through the original file factory, calls the original asset manager load/create APIs, inspects the resulting original render-object/material/hierarchy/animation state, releases clones and manager-owned prototypes, and observes zero live ownership.

## Validation and error handling

- Positive fixtures cover mesh plus HLOD/hierarchy/animation and multiple material/shader/texture states using documented W3D chunks.
- Missing/truncated/oversized/unknown-required chunks and duplicate prototype names reject through the original loader's existing error contract. Preserve its authored intermediate-publication behavior; do not impose a new global transaction on `WW3DAssetManager`. Unresolved hierarchy/model/texture references and allocation failure at a required scene resource must fail the owning scenario load and unwind newly published scenario state (verified with a failure/retry witness in slice 05).
- Identity and provider-removal gates require original asset manager, loader, render-object, material, and shader translation units.
- Existing canonical schema, M21 simulation, data/VFS, renderer-component, and source-classification tests remain green.

## Acceptance criteria

- Geometry, indices, hierarchy, animation, texture references, material passes, mapper state, and `ShaderClass` values originate in original WW3D objects and are inspectable without an adapter copy becoming authoritative.
- The compiled closure is minimal and evidence-backed; unreached WW3D files remain excluded.
- Normal and every injected failure path release all original objects exactly once.

## Commit boundary

Commit original WW3D CPU closure, owned fixtures, closure/identity/provider-removal tests, ledger update, slice status, and evidence as `delivery: M22 slice 01 restore original WW3D assets`.
