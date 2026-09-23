# M22 plan 01 slice 07FEA: original particle-provider closure

## Outcome and dependency

Requires the accepted 07F9 generated map frame and closes the prerequisite
provider edge for 07FE.  The full-draw target must compile and instantiate the
canonical `W3DParticleSystemManager`, not the generic Linux particle manager,
when an explicit test profile selects it.  It owns the original one-shot
`queueParticleRender` → `doParticles` dispatch and calls the published
original smudge manager only for the zero-particle compatible path.

## Boundaries and acceptance

The source manager has no particle point/streak/volume buffers in this child.
It clears the source ready latch and submits the empty original-smudge request
only after a loaded terrain and original smudge owner exist.  A missing owner,
missing map, populated particle/system list or an inactive edge fails closed
before an effect draw; a duplicate queue coalesces to its one source request.
Default factories retain the
Linux manager; only `ZH_M22_PARTICLE_PROFILE` selects the original provider in
the full-draw probe.

The generated-map Recording probe must prove source dynamic type, queue
coalescing and reset, manager/map/smudge negatives, failure/retry, two
generations and source teardown with zero Recording resources.  It must not
claim active particle geometry, smudge geometry, pixels, retail effects,
weather, volume particles, trees or a production factory switch.  Those are
the remaining 07FE child work.
