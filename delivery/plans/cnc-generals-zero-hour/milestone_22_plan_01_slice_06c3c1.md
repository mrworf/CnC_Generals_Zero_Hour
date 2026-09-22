# M22 plan 01 slice 06C3C1: source camera clear through public bgfx

## Goal and observable outcome

Requires accepted 06C3B3B, M29 and M30. Calling canonical `DX8Wrapper::Clear` from an active original `WW3D::Begin_Render` frame after canonical `CameraClass::Apply` produces an ordered, viewport-clipped color/depth/stencil clear through `OriginalGpuEdge` and `GpuDevice`. A caller-owned earlier draw and later draw remain ordered and outside pixels survive. This proves the physical source call, not yet full `WW3D::Render(scene)` or retail acceptance.

## Scope and dependencies

Implement the CPU-only branch of the same canonical `DX8Wrapper::Clear` method, preserving source flag selection (`clear_color`, `clear_z_stencil`, stencil only when the active depth target has stencil), `Convert_Color` packing and its default alpha/Z/stencil arguments. A narrow backend-neutral live-handle format query lets the source distinguish D24S8 from depth16/depth32, failing closed for stale/unknown handles; the public edge executes `clear_source_viewport`. Do not change native Direct3D body, original camera selection, scene scheduler, bgfx draw rules, asset providers or 06C3C2's full scene body.

## End-to-end behavior and state

Entry: original `Begin_Render` receives externally owned color/depth targets, then `CameraClass::Apply` selects the source viewport. A source `DX8Wrapper::Clear` creates no resources, clears exactly the selected flags/rectangle and returns while the source frame remains active. Original `End_Render` closes/presents; teardown has no outstanding adapter allocations. Unsupported values for selected flags, absent/inactive source frame or viewport, stale/unknown target format, and injected device-clear failure reject without a success marker, partial command or stale active pass; the caller's original abort/retry path remains valid. Unselected color/Z/stencil values do not cause rejection. No authorization beyond ownership of the bound target; retail material is untouched.

## Tests and validation

Use owned original-rendering test surfaces and direct canonical source calls. Positive: full-target source Begin clear, source camera subviewport, source `DX8Wrapper::Clear`, later draw and End; Recording trace establishes source ordering, independent CDS flags, depth-only/no-stencil flags, and target generation. Public-bgfx Vulkan test with Khronos layer checks untouched outer color and changed inset at two extents/generations, then a source depth-only clear on depth32 preserving color. M30's accepted public device test independently proves depth/stencil preservation and depth/stencil-tested pixels for the same command descriptor; the complete source scene/frame depth/stencil test remains C2. Negative: no frame/camera, invalid selected source color/Z/stencil, stale/unknown target and injected clear failure; assert no incomplete frame acceptance and successful reset/retry. Run focused original GPU-edge and WW3D CPU tests, bgfx GPU tests on the host, source identity/provider/ABI and ledger checks, GCC/Clang builds and relevant sanitizers. Full suite at M22 acceptance unless the M22 contract expressly requires it here. Record exact commands and evidence grade in `delivery/evidence/cnc-generals-zero-hour/milestone_22_slice_06c3c1.md`.

## Commit boundary

One commit for source `DX8Wrapper::Clear` translation, positive/negative tests, directly related documentation/ledger and evidence: `delivery: M22 slice 06C3C1 route original camera clear to bgfx`. C2 remains mandatory for `WW3D::Render(scene)` and 06C3 aggregate.
