# M22 plan 01 slice 08K: detached active-water pre-map display continuation

## Goal and observable outcome

Close the display half of the reset-detached active-water lifecycle accepted
in 08J. During the native client-before-logic update, terrain visual update now
accepts the exact detached pre-map state, but `W3DDisplay::draw` recognizes the
same state only through a generated-route selector. Replace that selector with
one complete source-owner predicate. The exact state issues no frame and no
device command; game logic then loads the map and the normal mapped display
path remains unchanged.

## Scope and non-scope

The predicate requires an initialized published display on the active device
edge, one published tactical view and camera, complete display scene/asset
owners, the published terrain visual in its accepted 08J reset-detached state,
and the existing tracks, shadow and smudge owners. It rejects a loaded map,
foreign or missing aliases, pending water resources, wrong scene membership,
active rendering, extra views and malformed water configuration. The no-op is
derived solely from live lifecycle state; it must not consult a generated or
retail scene selector.

Do not admit retail scenes, draw an empty placeholder frame, change client-
before-logic ordering, relax mapped-frame guards, alter 08J reattachment, or
change the accepted 08F0 stop. No retail/private input is used.

## Ownership, ordering and rollback

`GameEngine::update` calls `GameClient::update` before `GameLogic::update`.
The client updates terrain, then display, then asks the display to draw. In the
exact reset-detached state the terrain update and display draw are both
idempotent no-ops. The subsequent logic update loads terrain and reattaches
water transactionally under 08J; later display draws execute the ordinary
mapped path. Because the pre-map operation allocates, binds and submits
nothing, failure leaves ownership untouched and retry remains deterministic.

## Implementation and validation

Expected production surfaces are `W3DDisplay.cpp` and the smallest terrain-
visual state query needed to share 08J's authoritative predicate. Extend the
generated terrain-water/display coverage to prove repeated no-op, zero command
delta, removal/foreign-owner rejection, pending-resource rejection, malformed
state rejection, post-load mapped drawing, failed-load rollback followed by
another no-op and retry, two device generations and teardown. Revalidate the
generated scene boundary and construction route, including the absent-selector
08F0 stop.

Run focused GCC and Clang tests; sanitizer focus; all six canonical nonretail
suites with `-LE 'gpu|lan|retail'` and `ASAN_OPTIONS=detect_leaks=0` for
sanitizer suites; strict host GCC and Clang leak checks; physical Vulkan;
serial LAN; dependency ledgers; and `git diff --check`. Then rerun the redacted
retail boundary to determine whether slice 08 is admitted or another genuine
source owner is missing.

## Acceptance and commit boundary

The pre-map display no-op is exact-state bounded, selector-independent,
mutation-free and composes with 08J load/rollback/retry. One independently
reviewable implementation commit:
`delivery: M22 08K continue detached pre-map display`.
