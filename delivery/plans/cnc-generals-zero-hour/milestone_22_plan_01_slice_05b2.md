# M22 plan 01 slice 05B2: original texture/material/shader physical entry

## Outcome and dependencies

Requires 05B1. Original `TextureClass::Init`/`Apply`, its reached
`TextureLoader` and canonical 05B1 image providers retain source-owned
texture selection, decode, mip/filter, stage and missing-resource decisions.
The first original DX8 texture, material and shader physical commands
translate to public `GpuDevice` resources/state in exact source order.
Remaining interleaved pass decisions remain mandatory slice 06.

## Source closure

Characterize minimal original texture loader/format/file closure reached
by owned W3D and read-only retail encounters. Replace the CPU-only typed
`TextureClass::Init`/`Apply` boundary with original loader decisions and
device-only translation; no adapter-created pixels, checkerboard fallback,
external untracked asset loader, fake material or generic shader. Translate
original `TextureFilterClass`, `VertexMaterialClass`, `ShaderClass` stage
semantics only at physical calls. A public backend capability gap must be
proven and resolved narrowly, not hidden by an unsupported-state skip.

## Test and failure contracts

Owned image/W3D fixtures prove original loaded pixels, mip/filter and
material/shader identities then source-issued recording resources/state.
Missing/malformed required texture, unsupported format/stage, injected
load/create/upload failure and retry reject before success, release all
original/device owners and preserve optional original missing semantics.
Read-only retail family aggregate classifies required format closure;
provider-removal, ABI/link identity, GCC/Clang full suites, focused
sanitizers and ledger freshness. No full frame or scene claim until 06–09.

## Commit boundary

One independently validated commit: `delivery: M22 slice 05B2 translate original texture entry`.
