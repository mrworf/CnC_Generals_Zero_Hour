# M22 slice 06A3B1: original lit semantics, before physical draw

## Source identity and translation boundary

The original `DX8Wrapper` CPU configuration retains the source's explicit global `D3DRS_SPECULARMATERIALSOURCE=MATERIAL` and `D3DRS_COLORVERTEX=TRUE`; its reset-device profile records the documented Direct3D defaults for `LOCALVIEWER=TRUE` and `NORMALIZENORMALS=FALSE`, distinguishing those defaults from commented-out source calls. Original `VertexMaterialClass::Apply` still selects ambient/diffuse/emissive color sources, material/power and lighting, and original `DX8Wrapper::Set_Light_Environment` owns the global ambient and four light slots. The device-edge immutable semantic record copies the source-selected material terms, all four source lights/enable flags, packed ambient, selector/normalization/specular switches and source world/view matrices. It never chooses a light, material, geometry or render pass. The exact inverse-transpose lighting and pixels are **not** implemented in this slice: 06A3B2 remains the mandatory physical gate, and 06A3C remains the original category-issued gate.

## Owned witness and controls

An original model/VertexMaterial/ShaderClass and LightEnvironment fixture drives material-source COLOR1/COLOR2 selectors and restores its original values; source commands select nonuniform world (2, 0.5), view/projection, original packed ambient, empty, directional, point and four-light environments. Recording and immutable snapshots verify state and light data in source order, accepted defaults and changed selector flags. Invalid local-viewer token and missing normal FVF reject without replacing the source-selected environment; a lit physical draw stays typed unavailable without a new GPU resource/cache mutation. Earlier original shader/texture negative retry and superseded-state tests remain passing. Read-only retail family inventory (FVF 274 normal+UV1, three occurrences) is unchanged and is **not** a retail lit draw claim.

## Validation

- GCC and Clang Debug full builds, **146/146 non-LAN + 4/4 LAN** each (full gate before the final extra absent-normal test, which passed focused GCC/Clang reruns); original provider-removal, schema/ABI and dependency-ledger checks included.
- GCC and Clang ASan/UBSan/LSan original full draw, CPU graph, shader source, first GPU edge and edge failure **5/5 each**; the final absent-normal negative also passed a focused sanitizer rerun on both compilers.
- SDL_GPU Vulkan suite **4/4**, explicit `VK_LAYER_KHRONOS_validation`, zero Validation Error/VUID. `git diff --check` clean; original-game symlink untouched.

Device-default/translation references: [Microsoft D3DRENDERSTATETYPE](https://learn.microsoft.com/en-us/windows/win32/direct3d9/d3drenderstatetype), [Light Properties](https://learn.microsoft.com/en-us/windows/win32/direct3d9/light-properties), [ambient](https://learn.microsoft.com/en-us/windows/win32/direct3d9/ambient-lighting), [diffuse](https://learn.microsoft.com/en-us/windows/win32/direct3d9/diffuse-lighting), [specular](https://learn.microsoft.com/en-us/windows/win32/direct3d9/specular-lighting), [attenuation](https://learn.microsoft.com/en-us/windows/win32/direct3d9/attenuation-and-spotlight-factor), [camera-space transforms](https://learn.microsoft.com/en-us/windows/win32/direct3d9/camera-space-transformations).
