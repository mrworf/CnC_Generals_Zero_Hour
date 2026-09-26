# M22 plan 01 slice 08P0C2B0B: resident selected-stage atomic transaction

## Goal, dependency and boundary

After accepted C2A, D0 aggregate and dependency-first B0A source-reference
retirement, establish
one complete bounded idle-preparation transaction across original delayed
DX8 texture/stage/transform state and OriginalGpuEdge filter/sampler state.
C2B then supplies exact shroud semantics only. No frame/pass/view/draw/present,
C1 advancement, scene/factory/retail admission or generic source-state rewrite.
Generated local runtime work has no authorization change.

## Confirmed owner boundary

Native setShroudTex performs a multi-call source stage/transform sequence.
CPU Set_Texture changes retained refs/dirty bits before a fallible marker;
stage maps/transforms allocate before source recording. The edge increments
revision before marker emission, and delayed filter application can create a
new sampler and retire the accepted one. Existing SourceStateSnapshot excludes
texture refs/pending edge state and requires already-applied source state.
RenderStateStruct omits texture-stage maps/texture transforms, and restoring it
uses ordinary fallible setters. Neither is a complete rollback checkpoint.

The complete shared atomic boundary is the selected source stage and its
edge/device admission, not a map-reset shortcut or a shader-family owner.
Retain exact prior texture identities/refcounts, stage-map values and key
presence, texture transform, delayed/dirty state and shader-dirty baseline;
preserve unrelated stages, world/view/material/light/buffer state. Support
already-pending source state without forcing application as a baseline.
Preserve edge generation/revision, pending texture identity, filter values,
sampler identity and owned sampler resources. Pin old refs/resources until the
candidate is complete; never release the accepted sampler/ref during staging.

## Resolved resident-provider and selected-stage audit

D0 is accepted at `1507d9d5174f84a26c655ce42048aea983f5f99f`; its unchanged
B2B executable baseline and all required final-source gates precede this owner.
The shipping CPU shroud ReAcquireResources creates/publishes the texture, marks
it initialized with Apply_Gpu_Texture and sets its filter before setShroudTex.
B0B therefore admits only an immutable declared selection of resident/ready
providers (or explicit null). Exact active-edge map membership and generation
must be checked before dereferencing a provider; then require supported regular
TextureClass, initialized state, valid complete filter tuple and existing device
texture identity. Preflight every declared stage/provider/filter before reference
increments, source allocation/publication or opening the idle device journal.
Repeated declared stages, stage8/bound+1, stale/nonresident/uninitialized/unsupported
providers and malformed budgets reject without mutation. Pin all prior/current
and declared candidate source identities needed by the selected stage boundary.

Fresh TextureClass::Init is not stage admission: it changes inactivation fields,
runs synchronous foreground loading, publishes edge/native texture ownership and
changes initialization/format/extents before later filter effects can fail.
Such providers reject here; ordinary lazy loading remains unchanged. B0B does not
silently perform a load, initialize a provider or consume LastAccessed before
complete tuple admission. No new texture-loader/factory prerequisite is required
for the audited resident shroud route. Device-owned candidate/resource/upload
faults remain covered through the composed idle capability, not a fresh source
load hidden inside application.

TextureClass::Apply still updates LastAccessed before selection/marker/filter
effects. Snapshot that exact field for each admitted source identity and restore
it on any later failure, alongside retained refs/maps/transform/dirty state and
edge pending filters/samplers/revision. No new serialized member or platform ABI
change; any narrow read/restore boundary stays in the consistently CPU-selected
Linux declarations. Preserve filter defaults and the provider's authored filter
fields; B0B owns pending application, not changing the global filter profile.

Add selected-stage-only application. It invokes the original texture/null and
filter order for the admitted stages and clears only their texture dirty bits.
Do not call Apply_Render_State_Changes as a shortcut: it applies ShaderClass
and material plus every unrelated dirty stage. ShaderDirty/CurrentShader,
material/render/world/view/light/buffer state and unselected stage refs/maps/
transforms/dirty bits remain untouched, including an initially pending baseline.
Reject any attempted out-of-scope mutation while a stage owner is active before
that mutation; do not broaden the checkpoint to a general render-state snapshot.

Bound declared stages to eight, native stage keys to twelve per stage and
texture-transform keys to eight. Bound pinned source identities and all edge/
device resources explicitly; preserve absent versus present map keys. Candidate
copies and old reference/resource retention complete before publication. Commit/
abort use bounded allocation-free no-throw restoration/publication, not ordinary
fallible setters. Retained source teardown cannot run a fallible marker/resource
callback after the accepted device commit; stage retirement must be prepared
under the exact owner, then release reference units once in safe order. Reset,
source/edge removal and destruction cancel while the original generation is
still valid. Foreign finish cannot consume the live original owner.

## Candidate, device composition and commit

Use explicit finite stage/key/resource budgets and one nonnested exact-owner
transaction identity. Preflight active source/edge/device generation and D0's
idle capability before mutation; SDL/unsupported/mismatched/active-frame modes
fail closed. Stage map/transform allocations, retained refs, filter/sampler
candidates and marker/resource effects inside the admitted idle device journal.
No pass/view/viewport/clear/draw/present operation is allowed in this phase.

On any allocation/provider/create/upload/marker/filter/publication failure,
abort device-owned commands/resources and restore all owned source/edge state,
including revision and map key presence. Release only candidate references and
samplers, exactly once in reverse; leave prior identities and unrelated state
unchanged. Fault injections stay consumed so clean retry can succeed. After
complete admission, commit the device phase and publish source/edge candidates
through a bounded nonallocating/nonthrowing boundary; retire replaced refs/
samplers only after acceptance. No fallible callback remains after that point.
If another independently required owner is discovered, persist its plan before
edits rather than weakening this complete boundary.

Reset/removal/destruction cancels a live attempt while its original owner is
still valid, before source/edge/device retirement; never restore state through
a stale pointer into another generation. Repeated abort/commit must not mutate
accepted state or release an identity twice. Ordinary nontransaction setters
and native Windows/retail class layout remain unchanged.

## Surfaces, witnesses and acceptance

Expected surfaces: narrow CPU DX8 stage transaction API/body, OriginalGpuEdge
pending stage/filter/sampler transaction integration, D0 idle admission and
generated source-route witnesses. Do not duplicate private device checkpoint
containers or activate shroud semantics inside this owner.

Two-generation Recording positives prove complete source/edge/device baseline,
successful staged texture/transform/filter/sampler publication, exact surviving
refcounts/identities and clean retry. Negatives inject each candidate allocation,
marker, sampler/create/upload and later publication boundary; assert no partial
commands/resources, revision consumption, map key insertion, dirty changes,
accepted sampler retirement or unrelated-stage mutation. Include initially
pending source state, null/stale/removed owner, bounds+1, nested/cross-mode/
active-frame entry, partial abort, duplicate finish and owner destruction.
Physical bgfx idle controls prove prior resources/pixels and no pass/view/draw
consumption on rejection, then exact retry publication and zero residuals.

Exact focused commands persisted before production:

`cmake --build build/<preset> --target original_w3d_stage_transaction_tests original_w3d_gpu_edge_tests original_w3d_texture_decision_tests renderer_recording_transaction_tests renderer_bgfx_transaction_resource_tests -j4`

`ctest --test-dir build/<preset> -R '^(original_w3d_stage_transaction|original_w3d_gpu_edge_failure|original_w3d_texture_decisions|renderer_recording_transaction|renderer_bgfx_transaction_resource)$' --output-on-failure`

The generated public source-stage witness runs two generations; its `--gpu`
route uses the established host validation-clean wrapper on GCC/Clang native
and both sanitizer configurations. Explicit fresh/lazy negatives assert no
file request, Init/inactivation/access change, retained-reference change or
device admission; selected-stage positives/late faults assert exact LastAccessed,
map key presence, pending filter/sampler identity and unrelated pending shader/
material/stage state. Strict host LSan selects the same five CPU focused tests
with exactly `ASAN_OPTIONS=detect_leaks=1` and no UBSan override.

Run GCC/Clang native and
sanitizer focused witnesses; six complete builds and canonical nonretail suites
(`-LE 'gpu|lan|retail'`, sanitizer `ASAN_OPTIONS=detect_leaks=0`); exact serial
host strict LSan (`ASAN_OPTIONS=detect_leaks=1`, no UBSan override); generated and
established physical Vulkan with host/validation contract; serial LAN 4/4 all
six; ledger/diff and exact staged review on final source. Commit one slice:
`delivery: M22 08P0C2B0B own resident selected-stage transaction`.

## Required source-reference retirement composition

[B0A](milestone_22_plan_01_slice_08p0c2b0a.md) owns every acquired pin unit and
bounded cancellation/finish retirement. Reserve its exact capacity before source
or device mutation. B0B does not directly Release_Ref a possibly sole-owned source
from no-throw commit/abort: enqueue its already-owned units under A's reservation.
Drain only at an ordinary boundary after the stage/device attempt is inactive,
while the exact edge generation/device remain valid. A prepares native cleanup
transactionally before source metadata detach and final pooled release; retry or
terminal-failure disposition is its explicit contract, not a stage-local bypass.
Include a sole-previous-stage-owner replacement/cancel/retry control so B's finish
cannot hide a fallible destructor callback. B0 aggregate follows this child.

## Approved begin-failure composition correction

A pin admission precedes idle device admission. Failed device admission must
restore the source baseline even if capacity rejection persists. Implement A's
approved grouped all-survivor fast drain with B0B: after exact phase/generation
and arithmetic checks, all identities satisfying `Num_Refs > queued multiplicity`
release units without a device journal, allocations or callbacks. Equality/mixed
or uncertain cases retain A's transactional cleanup/retry. No new slice/owner.
The proof preserves maps/native ownership/revision, exact alias units, queued and
released counters, pending fault injection and once-only finish semantics.

Focused negatives must prove failed B0B checkpoint/capacity admission cancels and
drains survivor pins immediately to baseline without a second device admission.
Retain terminal prior-stage-owner replacement/cancellation controls separately.
Include A's exact four CPU focused controls, strict host LSan and generated
physical source-reference witness on final B0B source, in addition to the existing
five-test B0B focused set and six complete canonical/host matrix. No accepted A
final-source gate is reused to justify this changed executable boundary.
