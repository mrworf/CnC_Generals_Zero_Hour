# M22 plan 01 slice 08U0B: transactional particle source-frame output

Status: planned; depends on separately accepted 08U0A.
Plan transaction parent: `821c0367b01f0425e269c09b105d96bc876a214f`.

## Outcome, dependencies and source order

Consume A's exact reversible CPU candidate through the accepted C2D/D0
display-owned frame journal, scene/shroud/smudge checkpoints, B0/B1 selected
texture and source-resource ownership, accepted S0 preload/cache, and R0C
prop/full-map scheduling. No frame begins until complete bounded CPU and GPU
providers, textures, shader/state, point data and journal capacity are admitted.
Native order is `updateViews` → particle CPU update → terrain/prop/tree/shadow/
water/static-sort work → one scene particle queue and post-water translucent
particle/smudge flush → one present/commit. A's candidate finalizes only with
the matching successful B frame. A failed begin, bind, draw, smudge, present or
device commit aborts GPU commands and A together; retry uses the identical
phase and neither RNG nor IDs advance twice. An earlier committed frame and
unrelated sibling producer remain byte/identity/pixel unchanged on failure.

Scope is the reached `PARTICLE` + `ALPHA` point-quad family, exact native
frustum/terrain culling and at most512 source points per system, source angle/
size/color/alpha/personality/billboard rules, regular resident texture lookup,
alpha blend/depth/stage lowering, on-screen/field counters, and existing smudge
set/reset ordering when its bounded branch is actually admitted. Other point
shaders, DRAWABLE/STREAK/VOLUME/SMUDGE-specific geometry, heat distortion,
snow/weather output and unknown/missing assets or render states fail closed
before CPU/frame mutation unless separately proven in this approved B owner.
This does not admit audio, FXList or retail selectors and never silently drops a
required source branch. The ordinary empty-system/smudge source behavior stays.

## Resource, rollback and lifetime contract

Preflight the complete candidate list and exact texture/shader/provider
identities without synthesizing asset-cache entries or initializing nonresident
resources before admission. Use accepted source refs/pins for every distinct
texture and native resource generation; duplicate references count by ownership
unit. Bound systems and their cumulative commands/bytes against A's limits and
the existing frame journal/device maxima; each source group is at most512,
matching the native buffer. Count per-system culling/truncation and all later
smudge/weather conditions before journal mutation. Reject bound+1 and invalid
format/size/finite geometry/sampler/frame state without touching accepted
targets or A. Preserve exact `queueParticleRender` coalescing and once-only
consumption; the partial Linux empty-only `doParticles` guard is removed only
for admitted work.

Own the point buffers, textures, shader/pipeline, scene queue, smudge set,
field/on-screen counts and source-frame markers within the C2D owner-specific
checkpoint. Abort restores exact count/order/identity/bytes, frame commands,
pixels and A's snapshot; release candidate GPU refs/resources in reverse. No
native worker/backend allocation is claimed reversible beyond the accepted D0
contract. Do not call `Get_Texture` as a synthesizing lookup after the frame
starts or redraw a committed system on retry. Reset/provider withdrawal and
device recreation preflight active/pending frames; stale-generation pins are
released once, never replayed into a new device. Successful frame retires A's
obsolete nodes and GPU candidate resources exactly once. Two generations and
total initialized teardown leave zero live particle/texture/point resources.

## Generated and physical acceptance

Expected surfaces: W3DParticleSys, W3DDisplay/W3DScene admitted source-order
slots, only required point-group/texture/shader adapter and existing full-probe
generated map/test/CMake/ledger rows. No parallel scene, standalone retail
fixture or generic frame/backend redesign. Register
`original_w3d_particle_frame` and use existing
`original_w3d_particle_provider`/map-frame/tree-draw controls. The generated
input includes one visible emitted source particle, hidden/shrouded and culled
siblings, multiple alpha systems and one no-particle registry case. Recording
asserts exact source order, 512/bound+1, texture identity, point bytes, shader/
blend/depth, queue/smudge/counter changes and one present. Validation-enabled
Vulkan proves bounded nonblack alpha pixels, unchanged outside pixels and
identical fail/retry output, without generated stand-in geometry. Inject every
preflight, source queue, texture ref, point upload, draw, smudge, late present
and device commit fault. Assert no accepted target touch on preflight failure,
exact frame and A rollback on late failures, no RNG/ID/list re-advancement,
and clean same-phase retry. Cover unsupported type/shader/weather/snow/heat,
missing/stale texture/provider, over-capacity, reset/removal and two generations.
No private retail material is used for generated acceptance.

Build full six configurations. Focus on GCC/Clang debug and both sanitizer
configurations with:
`ASAN_OPTIONS=detect_leaks=0 ctest --test-dir build/<configuration> -R '^(original_w3d_particle_update|original_w3d_particle_update_provider_removal|original_w3d_particle_frame|original_w3d_particle_provider|original_w3d_terrain_map_frame|original_w3d_prop_frame|original_w3d_tree_draw|original_w3d_terrain_shroud_projection|original_w3d_full_draw_identity|renderer_bgfx_transaction_resource)$' --output-on-failure -j1 -V`.
Both sanitizer builds also run this exact set under strict host
`ASAN_OPTIONS=detect_leaks=1`, no strict UBSan override. Physical four use
`ASAN_OPTIONS=detect_leaks=0 VK_INSTANCE_LAYERS=VK_LAYER_KHRONOS_validation python3 tools/run_validation_clean.py python3 tests/original_rendering/test_w3d_particle_frame.py --source-root . --executable build/<configuration>/zh_original_w3d_full_probe --gpu`,
with the established authorized host route; the wrapper is a B output and
must retain bounded timeout-failure behavior. Run renderer physical transaction
controls, minimal8 both, established Vulkan3/2, serial LAN4 all six and six
fresh serial canonical `-LE 'gpu|lan|retail'` suites. Validate exact selected
counts, clean sanitizer/validation categories, dependency ledger, frozen hashes,
ABI, ownership and staged diff. Do not reuse A's executable acceptance after B
changes source; no timeout increase without measured evidence/approval.

Only after generated focus and physical pass, run the unchanged bounded redacted
scene-once wrapper with privately recovered inputs. Record fixed public
categories/counts, no names/paths/bytes/hashes/raw logs/images; crossing the
particle checkpoint is not by itself retail-scene acceptance. A new independent
producer stops at an architecture checkpoint. One exact B-owned implementation,
test, ledger and evidence commit after required gates; then close [08U0](milestone_22_plan_01_slice_08u0.md)
as a separate evidence-only aggregate on unchanged hashes.
