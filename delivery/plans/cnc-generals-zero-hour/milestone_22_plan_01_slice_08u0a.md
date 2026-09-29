# M22 plan 01 slice 08U0A: bounded particle CPU lifecycle and update candidate

Status: planned; production remains closed until this plan-only checkpoint commits.
Plan transaction parent: `821c0367b01f0425e269c09b105d96bc876a214f`.

## Outcome, dependencies and limit

Depends on accepted 08T0R3, 08S0, 08P0C2D0, 08L2R1 and the existing M20
GameClient/Object/Drawable/ParticleSystemManager providers. See the finite source
inventory and B integration in [08U0](milestone_22_plan_01_slice_08u0.md).
The native post-updateViews particle update becomes a bounded, reversible CPU
candidate. It preserves exact system/template/particle identity and source update
order, including stopped-system handling, while a failed later frame can restore
the pre-update state and retry without consuming GameClient RNG or system IDs.
This slice does not draw, upload, present, preload a texture, dispatch audio, or
admit retail scenes. No new selector, generic gameplay rewrite or class-layout /
serialized / Windows ABI change. Authorization is local generated validation;
the original corpus is read-only and private.

## Admission and state contract

The reached source graph is a nonempty manager registry even when its current
particle count is zero. Admission must validate exact manager/provider/generation
and registered template identity by callback-free membership before any
dereference or update. Inspect every reachable child name, slave and
per-particle attached-system edge as a bounded graph. A missing attachment ID
has native semantics: update clears it, marks the system destroyed and may
retire it that frame; this must remain candidate-rollbackable, not be rejected.
A missing optional slave/per-particle attached template is skipped as native
source does. Reject foreign/malformed provider identity, recursive cycles,
unsupported types or indeterminate graph state before mutation.
The reached family is `PARTICLE` with `ALPHA` shader. Other particle types,
shader families, external drawable effects, sound-bearing/irreversible callbacks,
and unknown template branches fail closed; no substitute output or empty-registry
shortcut. Prove the exact admitted branch has no direct audio/FXList dispatch.
Preserve ordinary headless/null no-op and unrelated nontransactional callers.

Use checked finite limits before snapshot, allocation, RNG or list mutation:
65536 reachable system/particle nodes combined, recursion depth64, 64MiB total
candidate CPU bytes and exact public ID/count arithmetic. These are transaction
admission ceilings, not claims that native gameplay has those limits. A template
burst, attached/slave expansion or ALWAYS_RENDER exemption cannot bypass them.
Bound+1 rejects owner-intact; no partial priority eviction or ID consumption.
Validate `m_maxParticleCount`/field limits and current linked-list counts against
the actual graph, not only reported counters. The fixed native render cap512
points per system is B's output bound, not a CPU truncation rule.

Build candidate state offside or snapshot every mutable owner with bounded
allocation before `ParticleSystemManager::update`: logic-frame stamp, manager
registry order/count, unique system ID, per-priority global heads/tails/counts,
per-system list heads/tails/counts, object/drawable attachment IDs, master/slave/
control edges, transforms/position/wind/delay/lifetime/burst/personality,
particle positions/velocity/keyframes/age and GameClient RNG. Respect the
source's `updateViews` then manager-update order and its same-logic-frame skip.
`createParticleSystem` increments ID before construction; its constructor can
consume RNG and create a slave before list `push_back`. Particle construction
links global and per-system intrusive lists; destruction unlinks both and can
destroy a controlled system. Guard all of these exact publication/retirement
points. Preserve accepted sibling identities and their order, not reconstruct
them from templates. Do not run destructive ordinary reset as rollback.

A candidate remains owner-scoped and reversible until B completes the matching
source frame. A generated CPU-only control may explicitly commit a candidate,
but production cannot independently finalize A before B's draw/present/commit.
Failure before or during update, or B's later abort, restores exact previous
bytes/links/IDs/counts/RNG/frame stamp and retires only candidate-created nodes.
Commit retires obsolete nodes once, preserves surviving identities and advances
the native state once. Cancel/reset/provider withdrawal rejects a live candidate
or cancels it before any owner destruction; stale-generation replay is forbidden.
Two generations, provider removal and total initialized teardown are required.
No added callback may run during rollback or a nonthrowing destructor.

## Implementation and generated acceptance

Expected owner surfaces: `ParticleSys.cpp/.h`, the existing source-frame
checkpoint/manager bridge and generated full-probe fixture/test registration;
ledger rows only for actually changed files. Do not touch the active08 trial,
renderer diagnostic, original symlink/content or generic container semantics.
The exact source owner may use private Linux-only sidecars/friend access without
fields, virtuals or Windows behavior change. If a new independent provider or
irreversible side effect is reached, stop at an architecture checkpoint rather
than adding another leaf.

Register `original_w3d_particle_update` and
`original_w3d_particle_update_provider_removal` using generated assets and the
existing full-probe/Python map lifecycle. Positive: nonempty stopped plus active
systems, real Object/Drawable attachment, missing-attachment native retirement,
missing optional child skip, shroud/terrain and source-order transform,
one-burst/zero-burst, priority eviction, finite wind/lifetime and
same-logic-frame skip. Compare fresh-equivalent generation success state and
next GameClient RNG value, exact system ID and intrusive-list order. Negative:
null/foreign/stale templates/providers, malformed attachment metadata, all unsupported families,
missing local player, child cycle/depth/node/byte/count/ID bound+1, active-frame
overlap, reset/removal and faults at every candidate allocation, constructor,
slave, particle link, eviction and list publication. Each fault proves exact
pre-attempt identities/bytes/counts/RNG, zero candidate residual and clean retry.
Repeat after reset and fresh provider/device generation; final initialized
allocation/ref residual zero. No external audio/FX dispatch is permitted.

Focused command for each of GCC/Clang debug and sanitizer configurations:
`ASAN_OPTIONS=detect_leaks=0 ctest --test-dir build/<configuration> -R '^(original_w3d_particle_update|original_w3d_particle_update_provider_removal|original_w3d_particle_provider|original_w3d_terrain_map_frame|original_w3d_prop_frame|original_w3d_tree_draw)$' --output-on-failure -j1 -V`.
Strict focused host LSan uses the identical selection on both sanitizer builds
with exactly `ASAN_OPTIONS=detect_leaks=1`. Build complete six configurations,
run established minimal/headless, Vulkan and LAN gates, then six serial
`-LE 'gpu|lan|retail'` canonical suites on frozen source as required by the
active M22 acceptance contract. Audit sanitizer categories, exact CTest counts,
dependency ledger, final hashes, ABI and staged diff; no inferred clean result.
No physical particle-output claim is made by A. The existing asset-free tests
and generated controls require no retail input or external authorization.

Acceptance: exact bounded CPU update/cancel/commit and deterministic retry are
independently witnessed; B can consume the immutable/reversible candidate
without re-advancing CPU state. One exact A-owned implementation/evidence commit
after all required gates; accepted A is B's production entry prerequisite.
