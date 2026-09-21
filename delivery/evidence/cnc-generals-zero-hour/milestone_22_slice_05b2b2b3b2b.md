# M22 slice 05B2B2B3B2B: original applied shader Vulkan execution

## Scope and ownership

The existing original `ShaderClass`, `DX8Wrapper`, `VertexMaterialClass`, `TextureClass`, `TextureFilterClass`, `Targa`, `DX8VertexBufferClass` and `DX8IndexBufferClass` issue the bounded input and delayed state. `OriginalGpuEdge` only translates the source-applied state and exact FVF layout into public GPU descriptors. Four fragment variants bind exactly the source-selected zero/one/two texture stages. Four vertex variants D1/D2/N1/N2 declare exactly the source FVF attributes; all absent/unimplemented combinations fail before shader/pipeline creation. The source-selected N1 retail and N2 fixture layouts were pipeline-created under validation, but no lit original category draw is claimed: source-issued category light environment and interleaved graph remain slice 06.

The owned 2×2 TGA fixtures are decoded by original sources, not adapter-created pixels; no retail file bytes/names/hash or private paths are recorded. The existing read-only retail full-draw scenario still exercises the original mesh/material ownership and remains distinct from this device-lowering probe. Original symlink untouched.

## Physical witnesses

- Explicit SDL_GPU Vulkan backend and `VK_INSTANCE_LAYERS=VK_LAYER_KHRONOS_validation`; the acceptance wrapper rejects any `Validation Error` or `VUID`. Exact D1/D2 indexed physical draw and N1/N2 input-layout pipeline probes pass with zero reported validation errors/VUID. Two independent device generations and 160×120 then 240×160 attachments pass, followed by bounded owner release and idle teardown.
- Original source-issued zero-stage unlit diffuse produces expected ARGB pixel; original resident source TGA one-stage modulation and two-stage additive detail produce expected pixels. The latter verifies both source mips survive sequential `SDL_UploadToGPUTexture` calls; SDL's `cycle=true` would replace the prior mip data, so the edge now uploads without cycling.
- Five permitted combiner ops × three source args pass 15 physical RGBA pixel cases for independently selected color/alpha. Separate checks cover source alpha-test discard and source alpha blend, fog, stage-zero UV transform and point-filtered/clamped texture selection. Recording contracts in B3B2A retain stage/owner/generation/descriptor/error-and-replay negatives, and source tests reject unsupported exact FVF before cache mutation. No synthetic category pass, WWShade schedule, lighting or retail scene is claimed.
- The cumulative test identified the existing positive normal+UV2 original Recording fixture; the N2 exact-input shader and Vulkan pipeline probe were added rather than weakening that acceptance.

## Validation

- GCC Debug and Clang Debug full builds, **146/146 non-LAN and 4/4 LAN per toolchain**, including original provider removal, ABI/link identity, retail full-draw and refreshed dependency ledger. The source-shader, texture-decision and texture-identity focused trio passes on both after N2 correction.
- GCC and Clang ASan/UBSan/LSan focused source-shader, texture-decision and read-only retail full-draw scenario **3/3 per toolchain** outside the ptrace-restricted sandbox.
- Vulkan GPU suite **4/4** (`original_w3d_texture_gpu_upload`, `renderer_gpu_acceptance`, `renderer_gpu_validation_layer`, `original_w3d_shader_gpu_acceptance`). The latter is run through the validation-clean wrapper with explicit Khronos layer and zero Validation Error/VUID.
- Original dependency-ledger SHA-256 refreshed. No original-game symlink modification. The original category interleaving, full GameClient scene and retail/visual/Vulkan milestones 06–09 remain pending.
