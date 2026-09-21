# M22 slice 05B2B2A: source-owned texture stage and filter decisions

The original `TextureClass::Apply` body is shared by the native and Linux
configurations of original `texture.cpp`; it initializes on demand, updates
the original access time, selects source-enabled or source-disabled texture
for the requested stage, then calls the original `TextureFilterClass::Apply`.
`texturefilter.cpp` now owns the Linux and native implementations instead of
the former CPU stand-in in `texture_cpu.inc`. Its Linux branch issues the
source's five distinct min, mag, mip, U and V state decisions in that order
to the scoped physical edge. Original `WW3D::Set_Texture_Filter` and
`Enable_Texturing` bodies are shared in original `ww3d.cpp` across the
mutually exclusive native/Linux ABI configurations.

The GPU edge retains a generation-scoped pending stage of original texture
handle plus public GPU sampler. It creates physical samplers from the
source-selected filter profile, bounds the original eight-stage range,
invalidates pending stage and sampler on original texture-owner teardown,
and releases all resources on session teardown. No render pass, draw,
shader, material or speculative binding occurs: original category pass
entry in 06 remains required. Public sampler `maximum_lod=0` preserves the
original no-mipmap choice, while the default finite maximum permits mips;
Recording validates and exposes the exact sampler descriptor and SDL_GPU
maps it to the public sampler API. The original bilinear, trilinear and
stage-zero anisotropic requests are represented; later anisotropic stages
follow the original linear downgrade. Actual SDL_GPU sampler creation is
the device capability check; unsupported requests fail closed rather than
inventing a driver maximum.

Owned original DDS/Targa file fixtures and an owned W3D texture chunk drive
`TextureClass::Apply` and `TextureFilterClass::Apply`: the W3D NO_LOD and
CLAMP_U fields reach the staged GPU sampler through `Load_Texture` and the
original asset manager. Stage zero/one sampler, exact five-state ordering,
texturing-disabled null selection, original authored optional missing
texture, no owner/stale handle, invalid stage/address/filter profile,
injected sampler failure/retry and owner release are verified on Recording.
Source-backed optional missing success is not a required-retail-resource
acceptance; scenario-level rejection/retry remains slice 08.

The opt-in real RTX Vulkan BC1/BGRA mip and bilinear/no-mip/anisotropic
sampler test, including device destruction/recreation, passed under
explicit `VK_INSTANCE_LAYERS=VK_LAYER_KHRONOS_validation` with the wrapper
rejecting any Validation Error/VUID. This owned physical sampler check
does not assert retail scene binding. GCC/Clang Debug full builds and
144/144 non-LAN tests each passed, with 2/2 LAN tests each on sequential
isolated socket runs. GCC/Clang focused ASan+UBSan+LeakSanitizer tests
passed; original provider identity and provider removal including
`texturefilter.cpp`, source classification (3293 paths) and dependency
ledger hashes passed. Full material/shader state and actual pass-time
binding, failure and frames remain 05B2B2B/06–09.
