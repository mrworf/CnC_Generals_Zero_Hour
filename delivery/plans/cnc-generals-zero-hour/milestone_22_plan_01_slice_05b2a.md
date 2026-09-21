# M22 plan 01 slice 05B2A: original texture CPU decisions

## Outcome and dependencies

Requires accepted 05B1. Canonical original `ww3dformat.cpp`,
`bitmaphandler.cpp`, and `textureloader.cpp` own reached format inference,
source image conversion, requested format and compression, mip generation,
device-size normalization, thumbnail/foreground/background routing,
reduction and missing-resource decisions. An original
`TextureClass::Init`/`TextureLoader` witness reaches a typed unavailable
physical texture creation after those original decisions. No created GPU
texture, completed image upload, material/shader pass, or frame is claimed.

## Source and configuration boundary

Restore only reached non-device methods and source-owned state from
canonical TUs; preserve native branch selection/semantics and original
file-factory ordering. A device-capability query required by format
selection must use the public renderer capability contract, preserving
the authored fallback decision and explicit unsupported failures; never
replace it with a hardcoded invented format or synthetic pixels. Original
format/bitmap producers may not be duplicated into an adapter or test.
Preserve mutual exclusion of M20 schema and M22 full object ABI; the
original physical creation is a typed failure at this bounded CPU step.

## Acceptance and negatives

Owned TGA/DDS/W3D fixtures exercise original source format and mip bytes,
request/thumbnail/foreground behavior, reduction and its clamp, optional
missing and required missing/malformed/unsupported controls, transition
to the first device edge, failure cleanup/retry, and zero file/texture
owners. Read-only retail family aggregates classify any additional reached
format/capability or thumbnail closure without retaining private details.
Check original source identity, provider-removal and no dual ABI objects;
GCC/Clang focused and full asset-free suites, focused ASan+UBSan,
source classification, ledger freshness and `git diff --check`.

## Commit boundary

One independently validated commit: `delivery: M22 slice 05B2A restore original texture decisions`.
