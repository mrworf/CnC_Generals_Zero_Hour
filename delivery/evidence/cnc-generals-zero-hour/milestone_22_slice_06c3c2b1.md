# M22 slice 06C3C2B1 evidence — canonical WW3D scene wrapper

## Outcome and source boundary

The original `WW3D::Render(LayerListClass/LayerClass/SceneClass/RenderObjClass)` methods compile and execute in the Linux full configuration. The original source order is retained: camera update/apply, C1 viewport clear, source SOLID selection, ambient, C2A scene traversal, original `Flush`, and End. Unsupported POINT/LINE and extra passes reject before camera/scene publication. B2 still owns static-sort callback failure cleanup; B3 still owns complete source-driven bgfx pixels and aggregate acceptance.

The first wrapper run reached `DX8MeshRendererClass::Flush` with a null skin category list. GDB traced it to `WW3D::Init` omitting native `DX8Wrapper::Init`, whose `Do_Onetime_Device_Dependent_Inits` calls `TheDX8MeshRenderer.Init`. CPU WW3D Init/Shutdown now owns the matching original renderer-category Init/Shutdown lifecycle. Existing Init failure/retry and sanitizer tests pass; C1/C2A manual fixture-level initialization remains idempotent. The layer overload then exposed missing original `layer.cpp` from the CPU `zh_w3d` source list; it is now included, with canonical layer constructor/destructor/refcount behavior. The linker map contains `libzh_w3d.a(layer.cpp.o)` selected for `LayerClass::LayerClass()`.

The owned Recording test loads an original W3D mesh via `WW3DAssetManager`, places it in `SimpleSceneClass`, and runs `Begin_Render`→two `Render(scene,camera)` calls at distinct viewports→`End_Render`. It asserts camera, viewport clear, solid fill, actual original mesh draw, and second viewport order. A second frame exercises the real layer-list and direct object overloads. Negative cases cover inactive render, null layer/camera, POINT/LINE prepublication, and a hook exception that aborts the source frame with no draw, followed by fresh-frame retry. The accepted source Init test covers malformed-initialization rollback/retry. Existing C1 tests cover stale source attachments separately.

## Validation

- GCC Debug non-GPU original-rendering: 41/41 pass.
- Clang Debug focused Init/frame/C1/C2A/B1/provider-removal: 6/6 pass.
- GCC and Clang ASan/UBSan/LSan focused Init/frame/C1/C2A/B1: 5/5 each pass outside ptrace-restricted sandbox.
- `python3 tools/check_original_dependency_ledger.py --root . --ledger docs/original-runtime-dependency-ledger.tsv`: pass.
- `git diff --check`: pass.

## Residual

No static-sort exception-safety or source scene bgfx pixel claim is made. Those are B2/B3; WWShade retail C4 and GameClient/retail remain later M22 slices.
