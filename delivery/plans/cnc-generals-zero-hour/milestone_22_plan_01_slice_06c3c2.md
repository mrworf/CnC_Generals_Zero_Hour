# M22 plan 01 slice 06C3C2: original WW3D scene and static-sort frame

This is the aggregate acceptance contract, delivered in dependency order by [C2A](milestone_22_plan_01_slice_06c3c2a.md) source `SceneClass`/`SimpleSceneClass` traversal and [C2B](milestone_22_plan_01_slice_06c3c2b.md) canonical WW3D Render/static-sort/full-frame integration. No aggregate completion is claimed from either child alone.

## Goal and observable outcome

Requires accepted 06C3C1. Original `WW3D::Render(SceneClass*,CameraClass*,...)` and `Render(RenderObjClass&,RenderInfoClass&)` produce complete owned scene/object frames with canonical camera, source viewport clear, fill/ambient/light environment, scene/object calls and original Flush/static/sort order on Recording and public bgfx Vulkan. This closes aggregate 06C3, not WWShade family or retail GameClient acceptance.

## Scope and ordering

Restore the canonical source bodies and any reached, source-owned scene/ambient/fill physical mapping required by them. Preserve `TheDX8MeshRenderer.Flush()` → authored `SHD_FLUSH` branch → descending `Render_And_Clear_Static_Sort_Lists` → `SortingRendererClass::Flush()` → pending-delete release, static publication/refcounts and reentrancy while draining. Use C1's source clear, never a direct adapter clear replacing the WW3D call. No adapter-owned scene, geometry, material, scheduler or placeholder. 06C4 owns WWShade enabled/disabled branch/family; 07 GameClient, 08 retail recording, 09 retail pixels.

## End-to-end state, validation and errors

Caller-owned targets and source Begin frame enter the original `Render` method. Camera update/apply, clear, fill, ambient, scene/object render and source Flush emit a balanced command stream; End presents only a complete frame. No file/network authorization applies; retail roots remain read-only. Null scene/camera, inactive frame, unsupported fill/ambient/scene variant, stale resource and injected clear/category/static/sort failures reject and unwind/requeue original owners; reset/retry yields no duplicate/stale draws. Test a two-camera, two-static-level mixed rigid/skin/decal/translucent fixture at two extents/device generations, source-identity and provider-removal controls, exact Recording order and visible bgfx color/depth/stencil pixels. Run focused GCC/Clang original-rendering tests, sanitizers, explicit validation-layer GPU with combined output scanned for `Validation Error`/`VUID-`, full/LAN suites, ABI/source/ledger checks; record commands and grade in `delivery/evidence/cnc-generals-zero-hour/milestone_22_slice_06c3c2.md`.

## Commit boundary

One independently reviewable commit for canonical source scene/object bodies, tests, documentation/ledger and evidence: `delivery: M22 slice 06C3C2 render original WW3D scenes`. Mark 06C3C and 06C3 aggregates accepted only after every condition above passes.
