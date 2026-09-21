# M22 plan 01 slice 05B2B2: original texture stage, material and shader state

## Outcome and dependencies

Requires accepted 05B2B1. Original `TextureClass::Apply`,
`TextureFilterClass::Apply`, reached original DX8 renderer material/shader
calls and WWShade path choices issue physical state in source order. A
scoped device-edge translator maps the exact original texture handle,
filter/addressing, stage, material and shader outputs onto public
`GpuDevice` state and Recording/SDL_GPU implementations. No substitute
material, generic shader, pass scheduler or complete frame is accepted;
original interleaved pass scheduling remains 06.

## Source and public-device boundary

Map the original source-issued texture stage/filter/material/shader calls,
including texture-disabled/null, multiple reached stages, alpha blend,
depth/cull/fog and supported WWShade decisions, to an explicit narrow
public contract with observable Recording parity and executable SDL_GPU
Vulkan semantics. If an actual consumer requires binding only within a
render pass, preserve pending source state until original 06 pass entry;
never inject a dummy draw/pass or reorder original calls. Reject unsupported
stage/combiner/shader families explicitly; no silent no-op. Keep original
method decisions canonical in mutually exclusive ABI configurations and
translate only at OS/GPU edges. Distinguish authored optional missing
texture from required-resource failure.

## Acceptance and negatives

Owned original W3D material/texture fixtures witness exact source identity,
stage and sampler state, material/shader parameters and call ordering on
Recording, plus SDL_GPU Vulkan capability-bound behavior. Test negative
missing required material/shader, unsupported stage/combiner, injected
sampler/pipeline/bind failures, reset/recreation/teardown and retry with
zero original/device owners. Read-only retail family aggregate informs
reached variants without private artifacts. Check provider-removal, no
mixed schema/full ABI objects, GCC/Clang full suites, focused
ASan+UBSan/LeakSanitizer, source classification/ledger and `git diff --check`.
06–09 remain required for complete original pass graph, scenes and Vulkan
visual acceptance.

## Commit boundary

One independently validated commit: `delivery: M22 slice 05B2B2 translate original stage state`.
