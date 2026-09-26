# M22 plan 01 slice 08P0C1B3B: committed tree FX dispatch

## Goal and boundary

After B3A admits the complete position FX graph, dispatch optional native
topple-start and bounce FX only for an accepted tree interaction transition.
State, geometry, Recording and GameClient preview must already be committed;
no FX or downstream RNG may occur on a rejected frame. Preserve source
shroud gate, topple/bounce position and event order. C2 terrain draw, C3
decal and C4 factory remain closed.

## Event ownership and failure contract

The CPU terrain registry owns an explicit immutable event intent and
at-most-once commit marker scoped by tree owner epoch and ID. Build and
validate the complete event batch before tree publication, using B3A's
read-only graph admission and finite source positions. Publish tree state,
type/atlas/geometry and Recording resources first; only then consume each
event marker and call `FXList::doFXPos`. There is no fallible tree/GPU step
after an effect begins. Dispatch-time nugget failure cannot undo an earlier
nugget; define an explicit accepted-state/at-most-once outcome and never
replay that event on frame retry. Removal/reset discards unconsumed events;
new owner epochs cannot inherit old markers. No repeated bounce, hidden or
paused frame may duplicate dispatch. GameLogic/audio RNG remain unchanged on
pre-dispatch rejection; GameClient FX randomness advances only on accepted
dispatch, never on failed admission.

## Acceptance and commit

Generated positives: no FX, one FX and multi-nugget FX with exact order,
public crusher route, shroud clear/fog gate, topple-start versus bounce,
pause/hidden/reentry, clean retry and two generations. Negatives: missing
B3A provider/list, unsupported late nugget, queue/publication fault,
state/geometry/Recording fault before commit, owner removal/reset and a
dispatch-time nugget failure after an earlier effect. Assert no duplicate
external effects or RNG on failed admission, and explicit at-most-once
behavior after a dispatch-time failure; retain immediate pre-teardown
resource residuals. Run focused GCC/Clang and sanitizer witnesses, six
complete builds/canonical nonretail suites, strict host LSan, physical
Vulkan, serial LAN 4/4 all six, ledger and diff checks. Commit one slice:
`delivery: M22 08P0C1B3B dispatch committed tree FX`.
