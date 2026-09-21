# M22 plan 01 slice 05B2B2B3A: canonical FVF physical layout

## Outcome and dependencies

Requires accepted 05B2B2B2. Preserve the original `DX8FVFCategoryContainer::Define_FVF` and `FVFInfoClass` decisions, vertex buffer bytes, index width/first index/base vertex and source ordering. Represent their actual position/normal/diffuse/UV offsets and stride through a narrow public `GpuDevice` vertex-layout contract shared by Recording and SDL_GPU. Do not reinterpret every buffer as the existing fixed `world_mesh`, repack/rebase geometry, add a private Vulkan route, or claim an original material pass before 06. B3B consumes the layout and adds shader/pipeline lowering.

## Source and public device boundary

Inventory the selected original FVF families against owned W3D fixtures and read-only retail family aggregates. Translate source offsets/stride to public semantic/format/location descriptors only at the device edge; source `FVFInfoClass` remains authoritative. Strictly reject unsupported position encodings, UV dimensionality/count, duplicate or overlapping attributes, out-of-stride offsets and device attribute/stride limits before resource creation, without silently dropping a required semantic. Preserve defaults of existing fixed layouts for earlier milestones. Recording validates the same descriptor and indexed draw bounds; SDL_GPU constructs `SDL_GPUVertexBufferDescription` and `SDL_GPUVertexAttribute` from the public descriptor. Reject mismatched index width and stale device generation. Compile/link checks prove one original FVF provider and no host ABI-mismatched inline WW3D layout.

## Acceptance and negative controls

Owned original W3D mesh/FVF fixtures establish at least the reached normal+UV, diffuse+UV and source-selected zero-UV families (where present), with exact original offsets/stride and unchanged vertex/index bytes and counts. Recording observes each valid layout, explicit first-index/base-vertex semantics and rejects unsupported/truncated/overlapping/mismatched layouts. On an actual SDL_GPU Vulkan device, execute an indexed layout probe under `VK_INSTANCE_LAYERS=VK_LAYER_KHRONOS_validation`, inspect output and assert zero Validation Error/VUID; use test-only shader/output solely to prove the physical layout, not as a replacement for original state/render decisions. Exercise resize/recreation, invalidated resources and bounded teardown. Tests retain original shader-source and texture/provider identity controls, GCC/Clang full suites, focused sanitizers, `git diff --check`, and the operation ledger. Neither retail scene frames nor complete shaders or authored category pass scheduling are accepted here.

## Commit boundary

One independently validated commit: `delivery: M22 slice 05B2B2B3A preserve original FVF layouts`.
