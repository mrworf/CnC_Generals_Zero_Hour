# N2 shared attachment owner

Parent d8eaf6127db0a0590795b3b2a0b1663df9d76bd8, clean start. Existing
N2 plan01/slice03; no new plan or agent. No asset/framework/recovery changes.

## Source and implementation contract

Original W3DModelDraw.cpp::getAttachToDrawableBoneOffset (765),
adjustTransformMtx (1115) and getProjectileLaunchOffset (2374/2484) all use
definition-lifetime attachment state. Condition/scale/Drawable lifetime changes
do not invalidate it. First named pristine supplier wins; a first miss caches0.
NativeModelDrawQueries now uses those shared ModuleData flag/value fields,
retaining its independent condition/scale-dependent pristine model cache.
NativeModelQueryTransaction captures both owners before fallible publication
and restores in reverse first-touch order. Invalid offset backing is explicitly
initialized; rollback must never read an indeterminate Vector3.

Two actually registered Drawables share identical parsed ModuleData. Generated
tests cover first supplier, different owner/state/scale, cached zero misses,
retirement of the second Drawable and continued first-owner queries. Existing
recursive cold/seeded/narrow/wide fault sweeps now assert exact shared flag/value
restoration on every failed prefix and retain every corrected retry. Generated
cold setup restores definition cache state before measurement; production
Drawable destruction does not. Logic-phase/thread guards remain unchanged.

## Retained exploratory evidence

Initial GCC focused8 terminal8: six existing query families passed, two new
shared-owner cases rejected their scale expectation. Fixture expected only one
scale; source validateCachedBones uses a scaled root in addition to scaled
prototype/tree translations. The complete scale cohort oracle was corrected
from24/40 to48/64; no production lookup/scale change. Log
`/tmp/n2-shared-attachment-gcc-tests.log` retained. Initial builds28982/16188/
30274 all retired0; first tests35156 retired8. No sanitizer test ran that oracle.

Corrected-scale GCC focused8 PASS21.50s, job82888 retired0. Builds69417/2142
retired0, but normal-host sanitizer focused8 jobs7929/72813 retired8: every case
stopped with stack overflow at main entry, before fixture/test behavior. Adding
three large automatic Fixture declarations expanded the main stack frame past
the ordinary host limit. New warm/test cohorts now use guarded heap ownership,
outside measured behavior and retired before baseline comparison. No limit,
timeout or sanitizer suppression change. Logs
`/tmp/n2-shared-attachment-corrected-{gcc,gccsan,clangsan}-{build,tests}.log`
remain scoped exploratory evidence, not final-source acceptance.

## Final source freeze and focused validation

Canonical735 SHA256
`3f55a37d3ec2381a8fdccf2e7f444c92186846bb71f71a3b40e988d10b191366`;
inclusive742 SHA256
`3bc89f51ed35a728119e8b84702faf70f5cb7ad261432f91d787d723ca220c2f`;
production/build659 SHA256
`157a53fa861365b439a5b45690cbd97a01b178ad2d89458f7db19aedbac38a4c`.
Configured full502 remains unrun. Final builds71272/10829/77343 retired0.
GCC focused8 PASS21.40s, job93720 retired0. Normal-host sanitizer focused8
jobs43078/60845 retired0: GCCsan91.85s/Clangsan51.96s. All3 emit12 normalized
fault manifest rows SHA256
`7a6aa430fbf740875eb2ce8f0ab5fcb202917125431fbc923e9c53359604dc94`.
No source changes since this final freeze.
Selector `^original_drawable_queries_(attachment-|functional$|negative$|faults$|seeded_faults$|wide_faults$|wide_seeded_faults$)`.
Logs `/tmp/n2-shared-attachment-final-{gcc,gccsan,clangsan}-{build,tests}.log`.
No acceptance/commit until focused results and the frozen related matrix pass.
Related selector `^original_(drawable_queries_|w3d_draw_data_|draw_provider_data_)`
contains62 tests, including the changed shared definition constructor's parser/
provider consumers. Related builds78592/62535/44801 retired0. Related62 jobs
54031/21425/85097 are live for GCC/GCCsan/Clangsan. Observe these exact handles;
do not duplicate. Logs `/tmp/n2-shared-attachment-related-{gcc,gccsan,clangsan}-{build,tests}.log`.
GCC related62 PASS233.57s/job54031 retired0. Sanitizer jobs21425/85097 remain
live; no complete related acceptance until all terminal results/manifests reconcile.
Clangsan related62 PASS556.13s/job85097 retired0. GCC and Clangsan each emit223
normalized manifest rows SHA256
`ce9ad11363252f71ae95bdf072e7bbabcfd21cdbb7e2b45ec94daccba748217d`.
GCCsan related62 PASS990.31s/job21425 retired0, matching the same223 manifest
rows/hash. Every build/focused/related job is now retired0. Prior live descriptions
are scoped observation history, not instructions to restart retired work.
Final canonical735/inclusive742/production659 hashes rechecked unchanged.
Official bgfx/bx/bimg pristine verification passed; no assets were accessed or
modified by these generated tests. All ASan/UBSan/LSan checks remain enabled;
normal stack limits and original60s family timeouts are unchanged.
Commit locator: `git log -1 --format=%H -- evidence/qa/N2-shared-attachment-owner.md`.
Verified supporting checkpoint only; slice03 and N2 are not accepted.

## Remaining boundary

Shared attachment owner correction is a prerequisite for the full ordered tick,
not that tick itself. Real AI/Physics/geometry/locomotion integration, client
pristine attachment staging, lazy pose/clock/PrevFrame/audio publication, source
transfer, concrete19 and original startup/loaded simulation remain required.
Retained clock completion/peek does not advance time; invalidation, progression,
pose evaluation and sound consumption are distinct source operations, documented
in docs/original-engine-formats.md. Slice03 and N2 are incomplete.
