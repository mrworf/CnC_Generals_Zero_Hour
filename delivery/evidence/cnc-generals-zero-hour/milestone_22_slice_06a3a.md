# M22 slice 06A3A: source light environment expansion

## Source identity and bounded snapshot

The original `DX8Wrapper::Set_Light_Environment` function body is extracted to one canonical include consumed by the mutually exclusive native DX8 and Linux CPU configurations. Original equivalent ambient selection, four original directional/point slots, negated light direction, source diffuse/ambient/first-light specular, original Generals point attenuation (1, 0.1/inner, 8/outer²), clearing unused slots and the authored null-environment retention remain in one source method. Linux records exact selected `D3DLIGHT8` values and enabled slots in the immutable source-state snapshot; it does not invent scene lights. Validation rejects invalid count, ambient, point radius/attenuation, nonfinite fields and invalid direct slot before mutation. Source category still calls that original method before its mesh world/draw selection. Lit physical draw remains typed unavailable until 06A3B.

## Witnesses and failure control

- Original `LightEnvironmentClass::Reset`/`Add_Light`/`Pre_Render_Update` drive zero, directional, mixed point and four-light original `Set_Light_Environment` states; Recording source markers and snapshots prove ambient, slot enable/disable, specular and authored point attenuation. An original `RenderInfoClass` environment reaches `MeshClass::Render` and original texture category, whose light marker precedes the mesh world and unlit draw. This is not a claim of a lit category pixel.
- Invalid ambient, direct invalid range/slot, and missing translator reject without publishing a replacement snapshot or a draw. Original null environment retains global lights, while a new device generation begins with all light slots disabled. No fallback ambient or adapter-generated light was introduced.
- The legacy renderer API inventory now categorizes the newly reached `D3DCOLORVALUE`; dependency ledger records shared source identity and owner obligations. Original provider-removal, ABI/link and read-only retail scenario checks remain passing. Retail aggregate remains FVF 274 (normal+UV1), count three; scene dynamic-light families remain 07/08 obligations.

## Validation

- GCC and Clang Debug full builds; **146/146 non-LAN and 4/4 LAN** each (the first concurrent GCC LAN run shared a port with Clang; isolated rerun passed).
- GCC/Clang focused ASan/UBSan/LSan original full draw, source graph, category edge, shader and GPU edge **5/5 each**.
- GPU Vulkan/validation suite **4/4**, with explicit Khronos validation and zero Validation Error/VUID. `git diff --check` clean; original-game symlink untouched. Physical lit shader and original lit category remain 06A3B/C.
