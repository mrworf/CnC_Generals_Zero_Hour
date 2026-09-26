# M22 plan 01 slice 08P0C2A: exact tree program semantics

## Goal, dependency and boundary

After accepted C1 and [C2A0](milestone_22_plan_01_slice_08p0c2a0.md)'s exact
reflected source-block origin prerequisite, translate the canonical `Shaders/Trees.nvv` vertex program
through the existing public GPU contract. Accepted XYZNDUV1 normal slots are
not geometric normals: x is sway index, y diffuse darkening and z tree base
height. Generic world-mesh or lighting shaders cannot substitute. C2B owns
source shroud binding, C2C prepared-frame lifetime, C2D scene draw; decals,
physical tree factory and retail admission remain closed. Generated local
runtime work requires no additional authorization.

## Source program and state

Preserve exact declared position/packed diffuse/UV input, source c4–7 matrix,
c8 no-sway and c9–18 sampled sway, c32 origin offset and c33 shroud scale.
Displace position by height above the base times the selected wave; scale
diffuse RGB by the encoded darkening while preserving source alpha; atlas UV
passes through; shroud UV comes from the original unswayed position, not the
displaced position. Encoded index 0 selects c8's zero vector; indices 1–10
select sampled waves. Native load/update retains the encoded sway type across
topple, so do not invent a topple-specific zero selection. Native `Trees.nvp`
is explicitly unused and the pixel
shader install is under `#if 0`; retain the original fixed-function fragment
combiner/alpha behavior rather than activating that unused program.

The source owner supplies all program constants and selection. The edge only
validates/transports/lowers them; no adapter-generated geometry or material
authority. Investigate the minimal explicit source-program selector and
uniform ABI within existing `OriginalGpuEdge`/shader registry conventions
before edits. Keep existing generic program ABI and behavior unchanged.
Bound finite constants, source FVF, encoded sway index and texture/resource generation
before creating resources. Candidate shader/pipeline/uniform create/upload
must unwind in reverse and leave the prior accepted identity intact; retry
must work without reset. No C1 state or RNG/FX consumption occurs here.

## Surfaces and validation

Expected surfaces: canonical tree program source semantics, exact source-owned
constant snapshot/selection, `OriginalGpuEdge`, project shader registry/build
inputs and focused generated program witness. Preserve native Windows source
and class layout; ledger records reached providers/hashes. Do not wire scene
rendering or create a surrogate tree registry.

Positives prove height-relative sway, no-sway, darkening, packed diffuse/alpha,
matrix orientation, unchanged atlas UV and unswayed shroud UV on generated
source tree buffers/constants. Negatives cover nonfinite/out-of-range input,
wrong FVF, omitted program, stale generation, resource creation/upload failure,
owner removal, rollback and retry. Direct shader/Recording and physical pixel
controls prove the exact program, not a complete terrain frame. Add no generic
fallback. Identify/persist exact focused commands before implementation.
Run GCC/Clang focused and sanitizer witnesses; six complete builds and
canonical `ctest --test-dir build/<preset> -LE 'gpu|lan|retail' --output-on-failure`;
sanitizers with `ASAN_OPTIONS=detect_leaks=0`; exact serial host strict LSan
`ASAN_OPTIONS=detect_leaks=1` without UBSan override; established physical
Vulkan and serial LAN 4/4 all six; ledger and diff checks on final source.
Commit one coherent slice: `delivery: M22 08P0C2A translate exact tree program`.

## Implementation admission checkpoint

Use a separate std140 tree vertex constant record and explicit tree-program
preparation, retaining the existing fixed-function fragment combiner and
generic uniform ABI. The terrain source snapshots accepted sampled sway,
canonical composite matrix and actual shroud offset/scale without advancing
C1. Validate the source XYZNDUV1 bytes and finite/bounded encoded indices
before resources; candidate creation/upload preserves prior accepted physical
identity until complete publication. No scene wiring is admitted.

Focused final-source commands: `ctest --test-dir build/<preset> -R
'^(original_w3d_tree_program|original_w3d_shader_source|original_w3d_first_gpu_edge|original_w3d_terrain_scene_attachment|original_headless_update)$'
--output-on-failure`; physical exact-program control
`original_w3d_tree_program_bgfx`, with established host graphical environment
and `VK_INSTANCE_LAYERS=VK_LAYER_KHRONOS_validation`. Both sanitizer focused
runs use detect_leaks=0; strict host LSan selects tree program and terrain
attachment with detect_leaks=1 and no UBSan override.

The standalone generated physical witness must initialize original process
services before bgfx workers and retire every bgfx device before service
shutdown. Native memory-pool critical sections are intentionally absent in
single-threaded tool mode; exercising omitted synchronization concurrently
would be undefined behavior, so no unsafe omission control is admitted.

The first uncommitted A trial exposed the independently owned reflected-block
defect recorded in C2A0. Remove those production/test trial edits after this
plan-only checkpoint; accept A0 independently, then reimplement A against its
corrected metadata owner. Trial Recording/physical results are localization,
not final A acceptance. Retain exact generated-UV1 tree-only admission with
stage0/output0 and stage1/output1, disabled transforms and unchanged generic
absent-UV rejection. Prove a nontrivial atlas quadrant and position-varying
unswayed shroud pixels, plus native nonzero toppled sway-index coverage.
