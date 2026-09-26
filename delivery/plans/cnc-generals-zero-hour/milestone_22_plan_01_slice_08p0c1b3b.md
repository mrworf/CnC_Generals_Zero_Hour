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

## Source-owner investigation

Native collision emits the start at the tree location; native bounce transforms
the local point `(0, 0, 21)` by the current toppling matrix. The CPU shipping
movement callback commits collision state before the later Recording frame,
so its registry instance must retain the original start position and admitted
list as pending intent. Preflight before collision publication and revalidate
the complete start-then-bounce batch before frame publication. Sequence counters
and consumed markers belong to the exact registry instance/owner epoch; the
committed frame consumes them before any external call. Dispatch a local
immutable batch without accessing the registry afterward, allowing a native
effect to remove an owner safely. Retire replaced GPU/atlas resources before
dispatch begins. Catch each dispatch failure as an accepted, consumed event;
later independent admitted events may continue, but its partially executed
nugget list is never retried. List definitions are immutable in the native
FXList contract; borrowed list pointers are revalidated against the live store.
Collision chronology uses a candidate-only monotonically increasing order,
committed with collision publication; failed publication and bounded overflow
consume neither order nor intent. Cancellation leaves surviving order gaps
unchanged; registry reset/new epoch starts at zero. Stable pre-publication
sorting orders all pending starts by callback chronology before source-order
bounces. The generated clear-shroud fixture must reveal the transformed bounce
tip as well as the base, while retaining real source query and exact logical
shroud baseline restoration (the old two-cell base reveal excludes the 21-unit tip).

GCC sanitizer localized a production `std::stable_sort` temporary-buffer
deallocation to the native nothrow/ordinary-delete allocator mismatch. Do not
change or suppress that allocator boundary in this slice. Use an explicitly
allocation-free bounded insertion sort of the local candidate batch, totally
ordered by event kind then collision chronology or bounce source ordinal.
The helper performs only stack value copies and indexed candidate writes,
with no registry, RNG or allocator entry. Add four reverse public callbacks
plus post-sort failure/retry, unchanged markers, identities, RNG and GPU;
rerun both sanitizer witnesses and the entire final-state matrix after the
correction. All first-freeze results are superseded acceptance evidence.
The 8000 limit bounds the existing heap-backed vector (start plus bounce for
the existing 4000-tree capacity), not a stack array; insertion uses exactly
one stack event value. Inject an actual 8001-element candidate, require helper
rejection before accepted marker/RNG/GPU mutation, and retain clean retry.
The reverse fixture also advances all four trees to bounce: record admitted
transformed tips in native registry order after the reversed starts, consume
suppressed tip markers exactly once, and prove pause cannot replay either.

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

## Delivered

Exact-owner pending intent, chronological bounded allocation-free ordering,
pre-publication batch admission, consumed-marker publication and post-commit
native dispatch pass the generated two-generation witness. Rejected attempts
preserve markers, geometry, GPU and both RNG streams; partial dispatch throws
are accepted/consumed and never replay. All six builds/canonical suites pass
267/267 each, focused native/sanitizer and strict host LSan pass 2/2 each,
physical Vulkan passes GCC 3/3 and Clang 2/2, serial LAN passes 4/4 all six,
and ledger/diff checks pass on unchanged final source. See
[final evidence](../../evidence/cnc-generals-zero-hour/milestone_22_slice_08p0c1b3b.md).
The general allocator-pairing lesson is recorded in `AGENTS.md`; no broader
allocator change is included. Draw, decals, factory and retail remain deferred.
