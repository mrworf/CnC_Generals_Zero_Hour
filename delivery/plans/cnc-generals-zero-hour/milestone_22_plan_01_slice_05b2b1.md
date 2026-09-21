# M22 plan 01 slice 05B2B1: original mip upload and texture lifetime

## Outcome and dependencies

Requires accepted 05B2A. Original `TextureClass::Init` enters original
`TextureLoader` task metadata and original `TextureLoadTaskClass` compressed
or uncompressed Begin, Lock, Load, and Unlock methods in authored order.
Original DDS/Targa and `BitmapHandlerClass` write exact mip bytes into
physical-edge staging storage, then public `GpuDevice` receives source-
issued texture create and per-mip upload. Source-owned initialized texture
identity and teardown persist through repeated load/use/release. No
material/shader binding or complete draw/frame is claimed.

## Source and translation boundary

Keep one canonical implementation of the reached original methods in
`textureloader.cpp` shared by mutually exclusive native and Linux object
configurations. The device edge may allocate locked staging bytes and map
original WW3D format/dimension/mip to `TextureDesc`/`TextureUploadDesc`, but
may not decode, synthesize, resample, choose fallback pixels or reorder
source calls. Preserve original DDS/TGA-first fallback and original
`MissingTexture` producer/pink optional fallback; the Linux adapter may not
generate or substitute those pixels. A missing or malformed **required
scenario** resource fails at the owning scenario boundary in slice 08 before
scene success, unwinds the authored optional loader's intermediate state and
supports reset/retry. The Linux original task ownership and source `TextureClass`
publication must match the authored sequence, apart from device-handle
translation. The public device API and Recording/SDL implementations must
validate exact pitches, block alignment, format and mip bounds; unsupported
create/upload fails closed with resource cleanup and retry. No D3D proxy or
private Vulkan call. Thumbnail/background routes reached in production are
preserved; unported Win32 mix routes remain typed negatives if unreachable.

## Acceptance and negatives

Owned TGA/DDS/W3D fixtures prove source-issued create and exact per-mip
source-decoded bytes, dimensions, reduction, DDS DXT1/3/5 families, 24/32-bit
Targa conversions, original mip rounding and original optional missing
texture bytes. Test unsupported format and dimension, malformed/truncated
load failures, injected create and upload failures, partial-task cleanup,
reset/retry, original and device
reference counts, and unsupported thumbnail route. Read-only retail family
aggregates classify reached source formats; do not retain retail names,
paths, bytes or hashes. Enforce canonical provider-removal, link-map ABI,
ledger/hash/source classification, GCC/Clang full suites and focused
ASan+UBSan/LeakSanitizer. The downstream 05B2B2 and 06–09 gates remain.

## Commit boundary

One independently validated commit: `delivery: M22 slice 05B2B1 upload original texture mips`.
