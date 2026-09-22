# M22 plan 01 slice 07D: original W3DView camera and empty source frame

## Goal and boundary

Requires accepted 07C3. Build the canonical original `W3DView` class in the full-draw probe, preserving its header/layout and native branch. Under an initialized original `W3DDisplay` owner and active `OriginalGpuEdge`, `init()` owns original 3D and 2D `CameraClass` refs; a default, zero-terrain, no-filter `drawView()` delegates to the original `RTS3DScene::doRender(m_3DCamera)` path and uses already translated `WW3D` frame boundaries. This is a source view/camera dependency, not production `W3DDisplay::draw`, factory activation, tactical updates, terrain, shroud, tracks, particles, shadows, postprocess filters, or retail acceptance.

## Entry, state, errors and surfaces

An initialized GameClient source scenario constructs the original display, attaches a source view, and binds a public Recording/bgfx color+depth target. The view has no cameras before init, two owned cameras after init, stable re-entry, and no refs after destruction. The fixture configures the source-owned 3D camera's transform/view plane because production tactical terrain camera transform is pending; this does not substitute a different camera owner. Source frame is clear-only with no attached object; when one generated rigid W3D object is attached to the display's original scene, the same view path produces a draw and physical changed pixels. Missing display owner, edge, camera, targets, unsupported filters/wireframe/tactical modes reject before a successful frame; rebind/retry must work. No new authorization or external mutation applies. Expected implementation surfaces: original `W3DView.cpp` CPU branch, canonical `ParabolicEase.cpp` constructor dependency, full-draw target, test profile/probe, ledger, and evidence. Preserve source-owned camera choice and fail closed for methods outside this slice.

## Tests and commit

Positive/negative Recording source trace, refcount/slot lifetime, no-owner/no-edge, absent and visible rigid object, injected draw failure/retry, and stale/rebound target. Public bgfx Vulkan compares clear-only and one-object pixels in BGRA8/RGBA8 at two extents, two device generations with explicit Khronos output rejection. GCC/Clang ASan+UBSan source controls, provider/ABI/ledger identity and mandated full asset-free suites are required before one independent plan/source/tests/evidence commit. Later 07 slices own display draw/view list integration and advanced categories.
