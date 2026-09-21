# M22 slice 05B2A: original texture CPU decisions

The production M22 full-ABI `zh_w3d` compiles original WW3D2
`ww3dformat.cpp`, `bitmaphandler.cpp`, `texture.cpp`, and `textureloader.cpp`.
The original loader's `Get_Texture_Information` and
`TextureLoadTaskClass::Begin_Compressed_Load/Begin_Uncompressed_Load` are
single canonical method bodies included by mutually exclusive native and
Linux configurations of **the same** `textureloader.cpp`. The source owns
DDS-first/Targa-fallback selection, compression, requested format fallback,
power-of-two/aspect normalization, reduction and mip count. Only its exact
DX8 texture-creation calls are replaced in the CPU configuration with a
typed `OriginalGpuEdge` physical boundary; no GPU texture or pixel upload is
claimed. The original `TextureClass::Init` invokes the source-owned loader.

The public renderer capability query specifies format, dimension and usage;
SDL_GPU calls its public `SDL_GPUTextureSupportsFormat`, and Recording has a
configurable negative profile. No public maximum extent query exists in the
installed SDL_GPU API. The Linux source configuration therefore preserves
the authored power-of-two/aspect decision and applies an explicit 16384
source-dimension / 64 MiB upload budget; actual backend creation remains the
hardware maximum authority in 05B2B. No fabricated hardware cap, 16-bit
display mode or generated texture pixels are used. Recording creation
rejects unsupported format/usage and requests over the bounded budget.

Owned original `FileFactoryClass` DDS and TGA fixtures drive the actual
`TextureClass::Init` through original loader methods: authored DDS two-lowest
mip removal followed by reduction one, single-mip reduction disable,
uncompressed Targa format routing, first physical edge, missing/malformed
negative, failure/reset/retry, and zero leaked file/GPU owners. The Win32
thumbnail path is a typed negative when enabled; production
`W3DDisplay::Init` disables it before loading. Separate original
`BitmapHandlerClass` tests verify BGR→BGRA bytes and authored mip rounding.
The preexisting read-only retail aggregate classified DDS DXT1/3/5 and TGA
families; actual retail frame loading is deferred to 08/09. No private
retail paths, names, bytes or hashes are recorded.

GCC and Clang Debug full builds and 144/144 non-LAN CTests each passed;
the two local-UDP LAN tests passed outside the socket sandbox for each
compiler. The focused original texture decisions, original source identity,
provider-removal of each original method owner and backend-neutral public
header tests pass on both. GCC and Clang focused AddressSanitizer,
UndefinedBehaviorSanitizer and LeakSanitizer pass outside the ptrace sandbox.
Original source classification covers 3293 paths; dependency-ledger hashes
and `git diff --check` pass. Physical texture create/upload/filter and
material/shader binding remain mandatory 05B2B; source interleaved pass
scheduling and scenes remain downstream, with no rendering acceptance here.
