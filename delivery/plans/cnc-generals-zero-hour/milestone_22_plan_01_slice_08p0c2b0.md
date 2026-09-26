# M22 plan 01 slice 08P0C2B0: complete delayed-source stage transaction

## Goal, dependency and boundary

After accepted C2A and both D0A/D0B capabilities plus D0 aggregate, establish
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

Persist exact focused commands before production. Run GCC/Clang native and
sanitizer focused witnesses; six complete builds and canonical nonretail suites
(`-LE 'gpu|lan|retail'`, sanitizer `ASAN_OPTIONS=detect_leaks=0`); exact serial
host strict LSan (`ASAN_OPTIONS=detect_leaks=1`, no UBSan override); generated and
established physical Vulkan with host/validation contract; serial LAN 4/4 all
six; ledger/diff and exact staged review on final source. Commit one slice:
`delivery: M22 08P0C2B0 own delayed source stage transaction`.
