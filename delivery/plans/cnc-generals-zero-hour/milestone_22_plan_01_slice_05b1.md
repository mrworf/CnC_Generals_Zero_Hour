# M22 plan 01 slice 05B1: original image providers

## Outcome and dependencies

Requires accepted 05A. The original WWLib `TARGA.CPP` and WW3D2
`ddsfile.cpp` own image parsing, compressed-block bytes, mip/reduction,
dimensions, format and unsupported/missing decisions required by subsequent
`TextureLoader` calls. Build the reached non-device methods in the full M22
target from those exact canonical sources. No `TextureClass::Init` success,
texture resource, material state, or frame is claimed in this CPU slice.

## Source closure and boundaries

Use project-owned generated image fixtures solely to verify the original
provider behavior, and classify read-only retail family/format aggregates
without recording private filenames, contents, paths or hashes. Preserve
original file-factory semantics and image orientation. Native Win32/D3D
surface-copy methods may remain typed physical edges until 05B2; do not
substitute a generic renderer loader or fork ownership of pixels, mip policy
or missing-image decisions. Keep M20 schema/full configuration mutually
exclusive and source identity intact.

## Positive and negative gates

Original Targa and DDS witnesses check exact bytes, dimensions, mip layout,
orientation and the reached DXT1/DXT3/DXT5 format families on owned inputs;
negative cases include missing, malformed, truncated and unsupported images,
bounded parse sizes, and clean re-entry after failure. Verify no original
provider omission passes the source-identity/provider-removal controls.
Run GCC/Clang focused and full asset-free suites, focused ASan+UBSan,
dependency-ledger freshness and original allocation/owner teardown. This
slice cannot claim 05B2's texture/resource/state acceptance.

## Commit boundary

One independently validated commit: `delivery: M22 slice 05B1 restore original image providers`.
