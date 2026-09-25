# M22 plan 01 slice 08P0C1B3: committed tree FX dispatch

## Goal and boundary

After C1B2, dispatch optional native topple-start and bounce FX at most once
for each accepted interaction transition. `FXList::doFXPos` is immediate and its
nuggets can create audio, drawable, light and other external effects; it is
not a reversible frame or GPU operation. Preserve the source shroud gate and
topple/bounce event order. No C2 draw, C3 decals or C4 physical factory.
Authorization is not applicable to generated local simulation.

## State, failure and surfaces

Audit the complete reached `FXList` nugget/provider graph before production.
Stage event intent with the interaction candidate, validate providers and
geometry/Recording before committing state, and put dispatch after the last
fallible state/GPU operation. Use an explicit owner-scoped deferred event
queue or equivalent commit marker so failed admission cannot dispatch and a
retry cannot duplicate already-dispatched FX. Decide and document handling
for a dispatch-time nugget failure, which cannot be reversed after a prior
nugget has acted; do not claim atomic external FX if only at-most-once
delivery is possible. Do not mutate GameLogic or audio RNG in failed
pre-dispatch paths; preserve native GameClient FX randomness only on accepted
dispatch. Removal/reset discards undispatched owner events without replay.
Expected source/test/ledger surfaces include `BaseHeightMap` header/source,
`FXList` only if a narrow provider contract is essential, and generated
terrain/FX fixture. Split any genuine independent FX provider prerequisite
before admitting it.

## Acceptance and commit

Generated positives: topple and bounce with no FX, one FX and multi-nugget
FX, shroud gate, order, pause, retry and two generations. Negatives:
provider disappearance, state/geometry/upload failure before dispatch,
owner removal and injected queue/publication failure. Prove no duplicate
dispatch on retry and immediate residuals. Run focused GCC/Clang and
sanitizer witnesses, six complete builds/canonical nonretail suites, strict
host LSan, physical Vulkan, serial LAN 4/4 all six, ledger and diff checks.
Commit one slice: `delivery: M22 08P0C1B3 dispatch committed tree FX`.
