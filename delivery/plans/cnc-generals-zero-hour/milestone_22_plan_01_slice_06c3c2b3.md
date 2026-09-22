# M22 plan 01 slice 06C3C2B3: complete original scene/static-sort public-bgfx frame

## Goal and boundary

Requires B2. Exercise the original source `Begin_Render`→`WW3D::Render(scene,camera)`→`End_Render` with owned rigid, skin, decal and translucent sorted source content plus two static levels, through the accepted public bgfx device. A source scene/static-sort trace and actual pixels—not an adapter-only replay—accept B, C2, C3C and C3 only after all checks pass. WWShade retail-family C4 and GameClient/retail later slices remain pending.

Delivery order: B3A proves the first actual source scene/two-level static bgfx pixels; B3B extends that exact source path to mixed rigid/skin/decal/translucent and depth/stencil-visible color outcomes; B3C closes clear/category/static/sort/stale-target faults and full cumulative tests. No B3A/B success alone accepts this aggregate.

## Verification and recovery

Recording asserts source command/state/refcount order and camera clear at two viewports. Hardware test uses validation-enabled RTX Vulkan and bounded RGBA readback at two extents/device generations, including source-issued draw/clear/draw. The accepted public device exposes color readback, not direct depth/stencil readback; prove depth/stencil effects through controlled source-issued color-draw visibility/occlusion and correlate with M30's independent device contract, without inventing a private readback edge. Scan validation output. Negative injection at clear/category/static/sort and stale target aborts without partial success, leaks or stale queue, with a fresh frame retry. Run GCC/Clang focused and full original-rendering/renderer suites, ABI/provider removal, dependency ledger, both sanitizer toolchains and the milestone-relevant full CTest boundary. Evidence `delivery/evidence/cnc-generals-zero-hour/milestone_22_slice_06c3c2b3.md`. One commit: `delivery: M22 slice 06C3C2B3 prove original scene static sort on bgfx`.
