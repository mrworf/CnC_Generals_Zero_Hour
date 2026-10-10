# N2 deferred model-state effects

Parent `bd43e3469f90f96dbb632cce0e5e85c38df94dd1`; same N2 plan01/slice03.
Work began from a clean committed visibility checkpoint. Supporting deferred
state lifecycle, not complete model tick/startup or N2 acceptance.

## Coupled source correction

prepareState has initialization, direct replacement and pending-target callers.
Idle reselection instead clones current state and selects another animation.
The pending-target boolean previously conflated skipping another transition
lookup with skipping effect replacement. Source doDrawModule invokes
setModelState(nextState), so that target must replace particles and apply its
visibility rules plus retained explicit overrides. The boolean is now named
skipTransition; all actual state admissions replace effects. Idle selection
still keeps them. No framework, library, retail content or recovery changes.

Generated actual Drawable, parsed particle manager/master-slave templates and
source display clock exercise a waiting state through completion. The condition
root retains old tracker ownership through tentative target publication and
restores exact current identity, selected/pending pointers, tracker ID, visibility,
asset caches, client RNG, system cursor/list and pooled used units on rejection.
Locally caught bad_alloc deliberately commits the outer root. Every standard
prefix has a corrected same-owner retry; accepted publication proves source
deferred old-system destruction before fixture restoration. New empty state
does not retain or recreate old authored effects.

## Fixture investigation retained

Initial builds91889/68960/72221 stopped terminal1 before CTest: the new test used
probeClientCursor before its declaration. Only the fixture declaration order was
corrected; logs `/tmp/n2-deferred-effects-{gcc,gccsan,clangsan}-build.log` retain
the failure. Retry builds succeeded. Initial GCC focused session15375 terminal8:
all65 publication failure/retry prefixes passed, but the first whole-fixture
retirement baseline had omitted this family's actual particle/model warm cohort.
Existing producer-particles families warm that complete parent before baselines;
the differently named new family did not. The identical complete new cohort is
now warmed before measured repeats, with no ownership-count exemptions or
timeout changes. Initial log `/tmp/n2-deferred-effects-gcc-tests.log` retained.
GCC warm retry passed, followed by the final focused4 below. Sanitizer/whole-owner
gates remain mandatory; warming alone is not a leak-check acceptance claim.

## Frozen source and execution

Canonical735 SHA256:
`ff3065ff7f5b4a1c0be3dc6525d274066863828f016ff2996493d1fe6b670fc3`.
Inclusive742 SHA256:
`8aa7ff6b9f634407d99aff5bfed7a93c2410b7e75ac1abf9ff5968e5ba124bc6`.
Full configured498 **unrun**. All final fixture builds passed and retired0.

Focused selector:
`^original_drawable_queries_(producer-deferred-effects|producer-subobjects|producer-transitions|producer-particles-lifecycle)$`.

GCC focused4 PASS23.15s, terminal0.13 normalized manifest rows SHA256
`b6468a4a9e2fa7f6062d95a8d2910511d4ef7a666635f61fec03200a1f226b24`.
Deferred admission proves terminal65, all prefixes and terminal with every retry,
both warm execution and three measured complete repeats. Related selector:
`^original_drawable_queries_`,42 configured; all condition/query/clock/pose/
particle ownership paths, not just the new family.

Final focused4 PASS: GCC23.15s, GCC sanitizer92.03s, Clang sanitizer53.86s.
All three have the same13 normalized manifest rows and focused SHA256 above.
Final related42 PASS: GCC210.98s, GCC sanitizer843.31s, Clang sanitizer497.19s.
All three have76 normalized manifest rows SHA256
`497254120e767d64281bb7fdf629ce2fc0ef8e78cfcde87b5e9c99de8b4cb2b3`.
GCC related19170 and combined sanitizer focused/related37173/58150 all retired
terminal0; no live jobs remain. Source fingerprints rechecked unchanged.
ASan/UBSan/LSan ran on the normal host with all leak checks enabled, no
suppression. Upstream verification passed for pristine bgfx
`cca91681c953d2de9531197b0f580c866ffaa775`, bx
`d86e4ea9d9da6e832a3ff41398587d82b772c69b` and bimg
`101b5b5fd4670f82cfdec8e98aa1abf9ff5968e5ba124bc6`.
All logs:
`/tmp/n2-deferred-effects-final-{gcc,gccsan,clangsan}-{build,tests}.log` and
`/tmp/n2-deferred-effects-related-{gcc,gccsan,clangsan}-tests.log`.

## Pending outcomes

This verified supporting checkpoint is located non-self-referentially with
`git log -1 --format=%H -- evidence/qa/N2-deferred-model-state-effects.md`.
The user-requested worktree review found exactly seven modified files and this
one new QA file, all owned by this coherent correction. No unrelated changes,
retail assets, generated outputs or recovery content were staged or removed.
Continue the existing
coherent ordered model tick: actual physics/AI/geometry composition, power/scale/
duration/attachment/construction/turret ordering, particle sampling before recoil,
lazy pose/clock PrevFrame/event admission, source transfer and concrete19. Actual
original startup/loaded deterministic simulation, full498, slice03 and N2 remain
incomplete. The earlier visibility checkpoint is committed; it is not current
acceptance for this changed producer source.
