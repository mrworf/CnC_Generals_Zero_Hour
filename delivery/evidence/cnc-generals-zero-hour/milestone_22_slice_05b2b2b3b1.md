# M22 slice 05B2B2B3B1: applied original state semantics

## Source ownership and exact boundary

Original `DX8Wrapper::Apply_Render_State_Changes` still owns the delayed shader, material, texture and filter order. A CPU-only, read-only `DX8Wrapper::Snapshot_Source_State` copies the resulting physical render/stage/material/mapper state and refuses pending dirty state. `OriginalGpuEdge::map_applied_state` translates that source result into the public original-FVF pipeline descriptor and bounded semantic inputs; it neither issues a draw nor creates a shader, pipeline, uniform or other GPU resource. It rejects absent material/render/stage state, invalid material/fog/comparison/blend/cull/UV/transform and unsafe stage sequencing. The enabled lighting bit is **preserved**, including the owned original W3D lit material; physical lit execution awaits original category light issuance in 06.

The translation includes independent color/alpha operations (disable, select first/second, modulate, add), source DIFFUSE/CURRENT/TEXTURE arguments, depth write/compare, blending, cull, fog ARGB/range, alpha test/reference, material coefficient/color selectors and source mapper UV/bump/transform values. It does not reinterpret source ARGB as adapter-selected material color. Original unsupported shader operations retain source-owned failure and replay; supported source-authored fallback is mapped *after* `ShaderClass::Apply` rather than requested pre-fallback state. Original authored `TextureClass` generation checks and physical stage binding remain B3B2.

## Witnesses and limits

- Original shader methods issue state before the source snapshot; owned fixtures check default and selected shader states, all five permitted source operations against all three arguments in color and alpha channels, two-stage color/alpha distinction, alpha/blend/depth/cull/fog/material/UV mapping, source-authored fallback and unsupported operation retry. Negative pending/absent material, bad depth/cull/blend/material/fog/UV and stale session states do not mutate Recording GPU resources.
- An owned original W3D mesh/material fixture selects its source FVF and issues its source material/shader/screen mapper. Its projected camera-space UV transform, material opacity **0.75**, and authored **enabled lighting** appear in the semantic output. The physical light environment and actual category draw remain 06, not a positive draw claim here.
- The existing read-only retail model family witness reaches three meshes with the source-selected XYZ+normal+one-UV FVF; the full retail scenario test still passes. This does not establish coverage of every campaign/skirmish shader or a retail rendered frame. No original symlink content or private retail filenames, paths, bytes or hashes were changed or recorded.

## Validation

- GCC and Clang Debug full builds and **146/146 non-LAN + 4/4 LAN tests each**, including single-original-ABI, source identity, provider-removal and dependency ledger.
- GCC and Clang ASan/UBSan/LSan focused original CPU graph, shader source and read-only retail full-draw tests **3/3 each**.
- Recording device resource count returns to zero and the session rejects state mapping after destruction. `git diff --check` passes. No B3B1 Vulkan shader/pixel or retail scene rendering claim is made; B3B2, 06–09 retain those gates.
