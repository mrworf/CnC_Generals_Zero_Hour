# M7 slice 02: DDS/TGA parsing and BC fallback

## Goal and observable outcome

Renderer asset input can parse bounded DDS and TGA byte streams into explicit texture data, preserve DXT2/DXT4 premultiplied-alpha semantics, and decode BC1/BC2/BC3 to deterministic RGBA8 whenever the backend reports no compressed-format support.

## Scope

Add fixed-width little-endian DDS parsing, uncompressed true-color TGA parsing (including origin normalization), checked dimension/payload arithmetic, BC1/2/3 block decoders, and a load decision based on public format support. Add synthetic positive and malformed/truncated/unsupported fixtures.

## Non-scope

Filesystem/VFS access, retail byte fixtures, paletted or RLE TGA, legacy D3DX APIs, GPU uploads, and visual pixel acceptance.

## Dependencies and ordering

Depends on M7 slice 01 types and M1 bounded/fixed-width conventions. It does not modify recorder lifetime semantics.

## Entry point and behavior

`parse_texture(bytes, backend_supports_bc)` returns either validated `TextureData` or an actionable error. DXT1/2/3/4/5 map to BC formats and DXT2/4 set premultiplied alpha. Unsupported BC formats are decoded to RGBA8; supported ones retain their block payload. TGA BGR/BGRA becomes top-left RGBA8.

## Data/state transitions

Input is immutable. Output owns bounded bytes and an explicit descriptor. Parsing is atomic: failures return no partial texture. No global state exists.

Authorization is not applicable because parsing operates only on caller-provided memory.

## Validation and recovery

Reject bad magic/header sizes, unsupported compression/pixel formats, zero or excessive dimensions, overflow, incomplete mip/block data, and truncated pixels. Synthetic fixtures prove exact pixels, BC transparency/interpolation, DXT premultiplication metadata, TGA channel/origin conversion, and supported-versus-fallback selection.

## Expected surfaces

`include/zh/renderer/texture_loader.h`, `src/renderer/texture_loader.cpp`, `tests/renderer/test_texture_loader.cpp`, and `CMakeLists.txt`.

## Validation commands

Build and run `renderer_texture_loader`, then the related `renderer-contract` label in one debug preset. Four-preset validation follows slice 03.

## Acceptance and commit boundary

All valid synthetic formats and all named malformed inputs behave deterministically without retail data. Commit as `delivery: M7 slice 02 add texture parsing fallback`.
