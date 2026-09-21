# M22 slice 06A2: original unlit rigid/category interleaving

## Source behavior and bounded physical edge

The original `DX8RigidFVFCategoryContainer::Render` and `DX8TextureCategoryClass::Render` share their native source body with the mutually exclusive CPU/GPU build: source category/pass/task traversal, stage selection, material, shader, per-mesh light-environment selection, world/camera alignment, sorting branch, alpha/additive/material/UV override and original polygon draw remain in authored order. CPU guards only the native debugger global. Original `DX8Wrapper` captures source `Matrix3D`/world-identity and normalization states; lit state remains typed unavailable until 06A3. The original `TextureBaseClass::Apply_Null` maps its canonical null DX8 assignment to the scoped stage-disabled GPU edge. The device edge adds the exact original normal/no-UV FVF shader variant; it does not supply geometry, pixels, stages, material or a render pass. The pre-existing shader translation's original `Matrix4x4`/D3D transpose convention is corrected for nonidentity GLSL world transforms.

## Source witness and negatives

- Test-owned authentic W3D mesh/material/shader/texture chunks load via original `WW3DAssetManager`, original `MeshClass::Render`, rigid insertion and category pass. An owned TGA FileFactory supplies source-loaded pixels. Original 0/1/2-stage shader and material selections issue actual Recording indexed draws with 0, 1 and 2 fragment bindings respectively; the two-stage path chooses original detail-add, not a manufactured pass. Source texture→material→shader→world→polygon/draw order is witnessed. No original-game bytes are copied or logged.
- Source alpha override changes/restores `ALPHAREF`; authored additive, linear-UV material override, world identity and nonidentity, camera aligned/oriented and object-scale normalization branches execute. The UV transform carries the supplied original override and original mapper state is restored. Original null-texture stage disables binding. Deliberately lit category state is rejected with no draw; 06A3 must restore original ambient/light conversion before lit physical execution.
- No installed translator rejects at the original flush. Injected public buffer-upload failure produces no physical draw, and the original mesh requeues successfully in a caller-owned pass. Category reset, model refs, texture files, buffers and all GPU handles cleanly tear down; original source/provider-removal and ABI/link tests remain positive.

## Validation

- GCC and Clang Debug full builds and **146/146 non-LAN + 4/4 LAN each**; cumulative retail read-only scenario, dependency ledger, ABI/link and provider-removal checks included.
- GCC and Clang ASan/UBSan/LSan focused original full-draw scenario, CPU graph, original shader source, first category edge and indexed GPU edge **5/5 each**.
- Four GPU tests **4/4**, including original shader Vulkan pixel/layout test with explicit `VK_LAYER_KHRONOS_validation`, N0/N1/N2 input variants, true source-issued nonidentity transform, and zero Validation Error/VUID. `git diff --check` clean; retail symlink untouched. No GameClient/retail Vulkan frame is claimed; 06A3/06B/06C/07/08/09 remain mandatory.
