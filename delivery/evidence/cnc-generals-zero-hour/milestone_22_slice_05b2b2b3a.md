# M22 slice 05B2B2B3A: canonical original FVF physical layouts

## Source ownership and boundary

- Original `DX8FVFCategoryContainer::Define_FVF` selects the format; original `FVFInfoClass` computes the actual stride and offsets. `OriginalGpuEdge::layout_for_fvf` translates only those original decisions to bounded public GPU attributes. No source buffer repacking, vertex/index rebasing or replacement draw schedule was introduced. The public `PipelineDesc` now carries an optional original FVF layout; fixed preexisting layouts keep their defaults. Pipeline hashing/equality, Recording validation and SDL_GPU pipeline input use the same descriptor.
- Position XYZ, optional normal/diffuse/specular, and up to eight source-selected UV sets have explicit locations and source offsets; unsupported positions, blend flags, unknown/unused bits, attribute overlap, duplicate semantics, out-of-stride offsets, unsupported elements and excessive attributes reject before pipeline creation. The original mixed-dimension tangent FVF currently produces overlapping offsets in `FVFInfoClass`, so the adapter rejects it instead of manufacturing packed geometry; later retail encounters must escalate if that family is required.
- Recording and SDL also validate original indexed vertex reachability using unchanged 16-/32-bit index bytes, first index, base vertex and source stride. Bounds failures precede the physical call. SDL uses public `SDL_GPUVertexBufferDescription`/`SDL_GPUVertexAttribute` and `SDL_DrawGPUIndexedPrimitives`; no private Vulkan calls. A bounded SDL_GPU diagnostic RGBA8 readback observes produced pixels but never selects source draw/material state.

## Positive and negative witnesses

- Owned original FVFInfoClass fixtures assert exact XYZ+normal+UV, XYZ+diffuse+two UV and zero-UV offsets, stride and semantic locations; Recording distinguishes pipelines and rejects invalid/overlapping/truncated layouts, unsupported formats, attribute limits and positive/negative indexed base-vertex range. Original vertex/index upload byte identity and device generation/teardown checks remain in the cumulative original GPU edge tests.
- The selected read-only retail W3D model reaches **three meshes, one FVF family**: XYZ+normal+one UV (FVF 274). Its source-owned `Define_FVF` result is checked by the same physical-layout translator before aggregate reporting. This is not a claim that every later campaign/skirmish family is covered; slice 08 must escalate an unsupported required family. No retail names, host paths, bytes or hashes appear here.
- The Vulkan test renders a four-vertex/16-bit-index owned triangle with original XYZ+diffuse+UV offsets 0/12/16 and stride 24, first index 1/base vertex 1, then downloads the RGBA8 target through public SDL_GPU and asserts the shaded center differs from its clear corner (RGB sums **320 vs 20 in each of two device generations**). Stale target readback fails after teardown. Its shader is expressly a **device layout probe**, not a stand-in for original material shading or an original WW3D frame. The GPU acceptance test explicitly enables `VK_LAYER_KHRONOS_validation`; the wrapper rejects any Validation Error/VUID. Resize/device recreation and bounded teardown are exercised by the same GPU acceptance generations. Original interleaved category passes, real scene visuals and shader semantics remain 06/08/09.

## Gates

- GCC/Clang debug full builds and 146 non-LAN tests each; four sequential LAN tests each.
- GCC/Clang focused ASan/UBSan/LSan original GPU edge and read-only retail draw tests, two each; provider-removal, ABI/link identity and dependency ledger included in full suites.
- Real RTX SDL_GPU Vulkan indexed layout and RGBA readback under explicit Khronos validation: one GPU acceptance test, zero Validation Error/VUID. `git diff --check` passes.

## Remaining contract

B3B must lower original shader/material/filter pending state with semantically matching pipeline and uniforms; 06 applies state at actual original category draw/pass in authored source order. 07–09 own production integration, retail recording and final scene/visual acceptance. The selected retail aggregate alone cannot close those gates.
