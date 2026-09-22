# M22 plan 01 slice 06C3C2B: original WW3D scene/static-sort frame

## Goal and outcome

Requires accepted C2A. Restore canonical `WW3D::Render(SceneClass*,CameraClass*,...)`, object and layer call paths in the Linux full configuration. The original source calls C1 camera clear and C2A scene traversal, then owns mesh/WWShade/static-sort/sorting Flush in the authored order. An owned mixed frame records original source provenance and produces validation-enabled public-bgfx Vulkan pixels, accepting C2 and aggregate C3C/C3 only after every fixture/error condition passes.

Delivery order: B1 restores the canonical WW3D wrappers and source-solid recording path; B2 repairs and tests exception-safe static-list disable/refcount drain; B3 proves the complete source scene/static-sort path against public-bgfx Vulkan pixels. No B1/B2 success by itself accepts this aggregate.

## Scope and state

Preserve source camera `On_Frame_Update`/`Apply`, source clear flags/color/depth/stencil, fill mode, ambient and light-environment, scene/object `Render`, and `TheDX8MeshRenderer.Flush()` → authored `SHD_FLUSH` → descending static-sort drain → sorting Flush → pending deletes. Restore the exact source methods, no adapter-owned scheduler or scene. Keep source static-list publication/refcount/reentrant disabling; make enable restoration and dequeued node ref releases exception-safe without reordering successful behavior. Wireframe/point fill and extra-pass family must either have exact physical support and tests or explicit pre-publication rejection; C4/08 audit retail reachability. No authorization beyond caller-owned device/fixture resources; retail root remains read-only.

## Tests, failures and recovery

Positive: original `Begin_Render`→two `Render(scene,camera)` calls at distinct viewports→`End_Render`, plus object entry, two static levels, rigid/skin/decal and translucent sorted categories; assert exact Recording command/state/refcount order and public-bgfx Vulkan color/depth/stencil pixels at two extents/generations. Negative: null/inactive scene/camera, unsupported fill/clear/scene variant, stale target, injected failures at clear/category/static/sort, reentrant static callback and reset/retry. No partial frame, stale queue, dangling list disable or refcount leak can count as success. Run GCC/Clang focused and full/LAN suites, source identity/provider/ABI, dependency ledger, sanitizers, explicit Khronos validation-output scan; record evidence grade and exact commands in `delivery/evidence/cnc-generals-zero-hour/milestone_22_slice_06c3c2b.md`. 06C4 WWShade family, 07 GameClient and 08/09 retail remain pending. One commit: `delivery: M22 slice 06C3C2B render original WW3D scenes`.
