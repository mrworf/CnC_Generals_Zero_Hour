# M22 plan 01 slice 06C3C2B3B: mixed source categories and depth visibility

## Outcome and boundary

Requires B3A. Extend the same original source scene/frame path to skin, decal, translucent sorting and two static levels through public bgfx. Preserve original category, WWShade and Flush order; do not replace source objects with adapter triangles. C4 remains the broader WWShade retail-family audit, while B3C owns the fault/full-suite acceptance matrix.

## Physical witness and recovery

Use owned W3D fixtures and controlled source positions/materials to make rigid/skin/decal/translucent and front/back depth or stencil interactions distinguishable in RGBA readback at two extents/generations. The public device has no direct depth/stencil readback: source-issued visible/occluded color outcomes are required, correlated with accepted M30 independent depth/stencil device probes. Recording asserts exact source category/static/sort command order and refcounts. A negative absent category or deliberately inverted depth setup must change the expected pixels, proving the fixture can detect omission. Validation output must remain clean.

## Tests and commit

Run GCC/Clang focused source/renderer suites, ABI/provider removal, dependency ledger, both sanitizer toolchains and validation-enabled RTX Vulkan. Evidence `delivery/evidence/cnc-generals-zero-hour/milestone_22_slice_06c3c2b3b.md`. One commit: `delivery: M22 slice 06C3C2B3B prove mixed source categories`.
