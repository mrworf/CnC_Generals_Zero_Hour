# M22 slice 05B2B1: original texture task and mip upload

The mutually exclusive M22 Linux `zh_w3d` target now compiles the original
`TextureClass::Init` → `TextureLoader::Request_Foreground_Loading` →
`TextureLoadTaskClass::Finish_Load` → `Begin_Load` → `Load` → `End_Load` →
`Apply` chain. Original task methods in `textureloader.cpp` have shared
canonical bodies in `textureloader_begin.inc`, `textureloader_mips.inc`, and
`textureloader_task.inc`; the native configuration includes those same
bodies. The original DDS/Targa/BitmapHandler methods choose formats and
write compressed/uncompressed mip bytes. Linux-only lock staging and
`OriginalGpuEdge` translate the original physical create, upload and owner
publication calls into public `GpuDevice`; they do not synthesize image
pixels, alter source fallback order, or bind a material/shader/pass.

The original `MissingTexture` producer supplies its authored optional pink
fallback (0x7FFF00FF) with one session-owned shared texture; adapter code
does not generate substitute pixels. Owned fixtures distinguish this optional
missing-source success from malformed/truncated mip failure and injected
GPU create/upload failure, all with cleanup and retry. Required scenario
resource rejection/unwind/retry remains slice 08, not falsely claimed here.
Source `TextureClass` lifetime invalidates the GPU handle; session teardown
invalidates outstanding original owners and a new device generation cannot
reuse a stale source handle. Win32 thumbnail/background mix routes remain
typed unavailable; production display disables thumbnail loading.

Positive fixture witnesses: DDS DXT1/3/5 exact reduced compressed block
bytes; DDS DXT1 unsupported → original DXT2 alpha fill → original uncompressed
fallback; DDS probe failure → Targa; Targa 24-bit BGR→BGRA and 32-bit BGRA
exact pixels, original 2×2→1×1 mip rounding, one-mip clamp, shared authored
missing texture. Negative witnesses: zero/oversize source dimensions,
unsupported format family, missing/truncated mip bytes, failure injection,
thumbnail path and stale generation. Recording rejects invalid pitch, block,
format, extent and mip level. SDL_GPU uses its public BC1/2/3/BGRA texture
formats and per-mip upload region; an opt-in real Vulkan BC1/BGRA mip upload
test passed on the graphical host with explicit
`VK_INSTANCE_LAYERS=VK_LAYER_KHRONOS_validation`, and the validation-clean
wrapper observed zero Validation Error/VUID. This is an owned physical-edge
test, **not** a retail scene or complete draw acceptance.

GCC and Clang Debug full builds and 144/144 non-LAN tests per compiler
passed; two local-UDP LAN tests per compiler passed outside the socket
sandbox. Focused original texture and renderer recording tests passed under
GCC and Clang ASan+UBSan+LeakSanitizer outside the ptrace sandbox. Original
texture identity and provider-removal including `missingtexture.cpp` passed
on both compilers; source classification covers 3293 paths exactly once,
and dependency-ledger hashes pass. The earlier first-GPU-edge witness gained
the same linker section collection already used by adjacent original-WW3D
witnesses to avoid pulling unreachable original Win32 link dependencies.

The read-only retail format-family aggregate from 05B1/05B2A remains the
only retail observation here; no private retail filenames, paths, bytes or
hashes are recorded. Source material/filter/shader binding is still 05B2B2;
original interleaved draw scheduling, scene consumers and validation-layer
retail frames remain mandatory 06–09.
