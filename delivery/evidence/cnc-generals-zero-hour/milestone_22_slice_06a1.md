# M22 slice 06A1: original indexed draw method boundary

## Source behavior and device edge

The original inline `DX8PolygonRendererClass::Render` now executes its one canonical body in mutually exclusive native and CPU configurations: the original polygon chooses the source index offset/base and triangle-list/strip route. Original CPU `DX8Wrapper` methods hold buffer object and engine references, preserve original low-polygon/debug guards and delayed shader/material/texture state application, and issue a typed draw through `OriginalGpuEdge` only in a caller-owned pass. The edge copies original vertex/index buffer bytes, validates 16-bit source indices, source base/vertex range and triangle topology, binds the existing applied shader descriptors and submits one public indexed command. It invents no geometry, material, texture or pass. Original native D3D path is unchanged.

## Positive and negative evidence

- Original MeshModelClass and DX8PolygonRendererClass fixture calls `Render(1)` on source 16-bit buffers; Recording observes original index start 3, base vertex 1, list topology and authored wrapper command order. Original wrapper `Draw_Strip` selects strip topology. Low-polygon and debugger-disabled guards issue no draw. Source buffer engine refs become zero on unbind and device buffers are destroyed on scoped edge teardown.
- No active caller pass, out-of-range source index start/base, source index outside the declared vertex range and injected public draw failure reject; the same original source state replays successfully after failure. Existing GPU edge create/upload failure and wrong buffer-type negatives remain. No category source traversal, lighting, GameClient scene or retail-frame acceptance is claimed: 06A2/A3 and 06B/C remain mandatory.

## Validation

- GCC and Clang Debug full builds and **146/146 non-LAN + 4/4 LAN per toolchain**, including source/provider-removal, ABI/link, read-only retail W3D fixture and dependency ledger.
- GCC and Clang ASan/UBSan/LSan focused original full-draw scenario, CPU graph, shader source and indexed GPU edge **4/4 per toolchain**.
- Prior original applied shader GPU Vulkan validation witness remains passing with explicit Khronos validation and zero Validation Error/VUID. `git diff --check` passes, original-game symlink untouched.
