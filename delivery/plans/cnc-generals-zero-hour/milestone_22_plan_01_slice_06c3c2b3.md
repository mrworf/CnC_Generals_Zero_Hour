# M22 plan 01 slice 06C3C2B3: complete original scene/static-sort public-bgfx frame

## Goal and boundary

Requires B2. Exercise the original source `Begin_Render`→`WW3D::Render(scene,camera)`→`End_Render` with owned rigid, skin, decal and translucent sorted source content plus two static levels, through the accepted public bgfx device. A source scene/static-sort trace and actual pixels—not an adapter-only replay—accept B, C2, C3C and C3 only after all checks pass. WWShade retail-family C4 and GameClient/retail later slices remain pending.

## Verification and recovery

Recording asserts source command/state/refcount order and camera clear at two viewports. Hardware test uses validation-enabled RTX Vulkan, color/depth/stencil readback at two extents/device generations, including draw/clear/draw; scan validation output. Negative injection at clear/category/static/sort and stale target aborts without partial success, leaks or stale queue, with a fresh frame retry. Run GCC/Clang focused and full original-rendering/renderer suites, ABI/provider removal, dependency ledger, both sanitizer toolchains and the milestone-relevant full CTest boundary. Evidence `delivery/evidence/cnc-generals-zero-hour/milestone_22_slice_06c3c2b3.md`. One commit: `delivery: M22 slice 06C3C2B3 prove original scene static sort on bgfx`.
