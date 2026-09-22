# M22 plan 01 slice 06C3C2B3B: mixed source categories and depth visibility

## Outcome and boundary

Requires B3A and B3B0–B3B3 prerequisites. Extend the same original source scene/frame path to skin, decal, translucent sorting and two static levels through public bgfx. Preserve original category, WWShade and Flush order; do not replace source objects with adapter triangles. C4 remains the broader WWShade retail-family audit, while B3C owns the fault/full-suite acceptance matrix.

No retail files, runtime defaults, UI, gameplay, GameClient integration, or new device abstraction are in scope. The existing original W3D fixture builder and `WW3DAssetManager`, `SimpleSceneClass`, camera, category/decal/sorting owners and `OriginalGpuEdge` are the only scene producers. Test-only source positions/materials may make coverage distinguishable but may not generate adapter-side geometry or bypass original source scheduling. No user authorization applies; owned W3D bytes are in-memory test data and device targets are caller-owned.

## Entry, state and errors

The fixture loads the expanded owned W3D packet, constructs original render objects, attaches them to one original scene, and invokes the same `WW3D::Begin_Render` → `Render(scene,camera)` → `End_Render` entry point as B3A. The original objects/decals retain refs through category/static/sort queues until source flush; the edge translates reached state to public bgfx commands. On absent required model/material/texture, unsupported category state, stale target, or failed physical command, fail the source frame and release queued refs; a new device generation must not reuse stale handles. Normal teardown removes objects, unlocks/deletes decal ownership, shuts down WW3D, then destroys public targets. No persistent state changes.

Investigation gate before implementation: prove which existing `TEST.SKINHLOD`/skin mesh, authored decal material and sorted translucent mesh variants are reachable from `SimpleSceneClass::Render` in a single source frame; identify any source-owned render-order or depth/stencil state that prevents the planned pixel oracle. Reuse the accepted direct original category/decal/sorting tests as regression witnesses, not as substitutes for the new source-scene entry point. If a required category cannot be reached without inventing a new source mechanism, replan narrowly before production changes.

## Physical witness and recovery

Use owned W3D fixtures and controlled source positions/materials to make rigid/skin/decal/translucent and front/back depth interactions on a D24S8 target distinguishable in RGBA readback at two extents/generations. The reached source pipeline has depth testing but does not translate original stencil compare state; this slice does not invent it, and existing independent stencil device/viewport-clear probes remain separate. The public device has no direct depth/stencil readback: source-issued visible/occluded color outcomes are required, correlated with accepted M30 independent depth/stencil device probes. Recording asserts exact source category/static/sort command order and refcounts. A negative absent category or deliberately inverted depth setup must change the expected pixels, proving the fixture can detect omission. Validation output must remain clean.

Positive tests compare per-region RGBA pixels and source Recording command order with each required category present. Negative tests remove each category in turn and invert at least one front/back depth relationship; affected regions must change while outer clear and unrelated regions remain stable. Include malformed/unsupported and failed-draw recovery only where newly reached, with zero outstanding source and public resources after each generation. Commands: focused GCC/Clang source CPU tests, `renderer_bgfx_device_contract`, validation-scanned bgfx source test with `VK_INSTANCE_LAYERS=VK_LAYER_KHRONOS_validation`, ASan/UBSan source and device controls, ABI/provider-removal, dependency-ledger and four preset full asset-free suites as required by M22. Retail/GPU remains opt-in so ordinary presets stay display- and asset-independent.

## Tests and commit

Run GCC/Clang focused source/renderer suites, ABI/provider removal, dependency ledger, both sanitizer toolchains and validation-enabled RTX Vulkan. Evidence `delivery/evidence/cnc-generals-zero-hour/milestone_22_slice_06c3c2b3b.md`. One commit: `delivery: M22 slice 06C3C2B3B prove mixed source categories`.
