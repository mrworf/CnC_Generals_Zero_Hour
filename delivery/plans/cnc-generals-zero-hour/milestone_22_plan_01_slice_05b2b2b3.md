# M22 plan 01 slice 05B2B2B3: lower original state to SDL_GPU

Superseded by [B3A](milestone_22_plan_01_slice_05b2b2b3a.md) and
[B3B](milestone_22_plan_01_slice_05b2b2b3b.md). This historical plan is not
an independently executable slice: canonical original FVF geometry must
first be represented in the public device contract.

## Outcome and dependencies

Requires accepted B2. Source-owned texture/mapper/material/shader state
lowers through the public `GpuDevice` to Recording and SDL_GPU Vulkan
shader, pipeline, sampler and uniform resources. Required original stage
combiners, light/fog/alpha and mesh vertex layouts obtain semantically
matching shader variants selected by the original pending-state values;
the edge cannot select geometry, color/material authority or reorder
passes. No dummy pass/draw; original interleaved category entry binds and
renders in 06.

## Public device and resource boundary

Use SDL_GPU public shader/pipeline/uniform APIs, compiled owned shader
sources and exact source-selected variants. Support every shader family
shown required by read-only retail family evidence; an unsupported
required combination fails with a specific contract escalation, never a
generic substitute or skipped scene. Preserve original alpha test,
blend/depth/cull/fog and UV transform/color math across Recording and
SDL_GPU. Manage resource generation, resize/recreation and bounded
teardown, while retaining original DX8Wrapper delayed ordering.

## Acceptance and negatives

Owned original material/shader/mesh fixtures drive source-selected pipeline
and uniform create state into Recording and validation-enabled SDL_GPU
Vulkan; verify positive/negative variants, wrong layout/stage limits,
unsupported capability/combiner, missing required shader/material, injected
shader/pipeline/uniform creation failure, reset/retry and zero resource
owners. Probe exact required retail variant aggregate read-only without
private filenames/paths/bytes/hashes; do not claim retail frames before
08/09. Full provider-removal/unmixed ABI, ledger, GCC/Clang suites and
sanitizers, source classification and `git diff --check` pass. Original
category Render/source-order draw and pass-time bind failure belong 06;
production GameClient/retail/visual gates remain 07–09.

## Commit boundary

One independently validated commit:
`delivery: M22 slice 05B2B2B3 lower original shader state`.
