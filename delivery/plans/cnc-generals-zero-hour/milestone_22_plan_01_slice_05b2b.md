# M22 plan 01 slice 05B2B: original texture/material/shader physical entry

## Outcome and dependencies

Requires accepted 05B2A. Original `TextureClass::Init`/`Apply` and reached
`TextureLoader` state complete source-owned texture selection/decode/mip/
filter/stage and missing-resource behavior. Original DX8 texture, material
and shader physical calls translate to public `GpuDevice` resources/state
in authored order. A scoped edge may hold original source objects and
translate their outputs; it may not select substitute geometry, images,
materials, shaders or passes. Full interleaved pass scheduling remains 06.

## Source closure

Preserve original thumbnail/background/foreground routes reached by owned
and read-only retail encounters. Connect original DDS/Targa provider bytes
and original bitmap/format/mip decisions to public GPU texture create/
upload; map original filter and material/shader state at physical calls.
No adapter-created pixels, checkerboard fallback, external loader, fake
material, generic shader or silent skip. Prove any GpuDevice/backend
capability gap, and resolve it through the public contract without private
Vulkan/Direct3D calls or reordering original source.

## Tests and failure controls

Owned image/W3D fixtures prove exact original loaded bytes, mip/filter,
material and shader identities followed by source-issued recording
resource/state. Missing/malformed required texture, unsupported format/
stage, injected load/create/upload failures and retry reject before success,
release every original/device owner and preserve authored optional missing
semantics. Read-only retail family aggregate, provider-removal, ABI/link
identity, GCC/Clang full suites, focused sanitizers and ledger freshness.
No complete frame or scene success until 06–09.

## Commit boundary

One independently validated commit: `delivery: M22 slice 05B2B translate original texture entry`.
