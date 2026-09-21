# M22 slice 05B1: original image-provider acceptance

Committed original provider graph: canonical WWLib `TARGA.CPP`, WW3D2
`ddsfile.cpp`, and WWLib `ffactory.cpp` are compiled into the full WW3D
target. The same original public types and file-factory ownership are used
by the direct test witness; there is no adapter-owned image decoder or
pixel substitution. `TARGA.H` retains 18/26/495-byte on-disk structures,
and `ddsfile.h` retains the 124-byte DDS header on x86_64. Native Win32
Targa write and DDS D3D-surface copying remain excluded physical edges;
no texture, material, shader, draw, or frame is accepted here.

An owned memory-backed original `FileFactoryClass`/`FileClass` supplies
Targa and DDS bytes to the original parser. Exact DXT1/3/5 blocks,
dimensions, original DDS mip reduction and original Targa Y-origin transform
pass. Missing files, wrong magic/header, unsupported DDS FourCC and Targa
type, short block/mip data, excessive mip count, zero dimensions, Targa RLE
packet overrun, and clean retry fail/succeed as specified. File owners
return to zero. The read-only retail family audit preceding this slice
classified 3,496 DDS and 375 TGA entries; the reached DDS FourCC families
were DXT1 (1,975), DXT5 (1,515), DXT3 (6). This aggregate contains no
retail names, paths, bytes or hashes. Retail image loading remains an
explicit downstream 08/09 acceptance, not a claim of this owned fixture.

Verification: GCC and Clang Debug full builds and 141/141 asset-free
non-UDP CTests each; both established local-UDP tests pass per compiler
outside the socket sandbox. Focused `original_w3d_image_providers`,
`original_w3d_image_identity` and `original_w3d_image_provider_removal`
pass on both compilers, including compile/link omission of each original
source. The focused original image witness passes GCC and Clang
ASan+UBSan with LeakSanitizer outside ptrace restrictions. Original
dependency-ledger freshness and `git diff --check` pass. The indexed
GPU source edge remains accepted 05A; original TextureLoader/TextureClass
and first physical texture/material/shader state remain mandatory 05B2.
