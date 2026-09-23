# M22 plan 01 slice 08: original retail scenes through recording device

## Outcome and dependencies

Requires accepted slice 08B and read-only PRE-008 corpus. Original campaign/skirmish
entry, logic, GameClient, WW3D and enabled WWShade producers load selected
real retail scenes and emit complete frames through recording `GpuDevice`.
Original graph owns reached terrain, model, HLOD, animation, texture,
shroud, fog, lighting, shadow, particle, water, effects and pass decisions.
Generated/test proxy scenes cannot substitute for retail acceptance.

## Validation and error handling

Retail roots remain read-only/private; no corpus bytes, private paths,
hashes or sensitive screenshots in committed output. Record source identity,
concrete families, aggregate sizes, pass/state/draw counts and ownership.
Negative controls remove/corrupt required owned-fixture resources, inject
failure at every device operation and exercise scenario-level required-asset
failure/reset/retry with no stale track/texture/render owners. Optional
missing models retain original optional semantics. Provider removal rejects
loss of original consumer, WW3D, enabled WWShade, adapter or recorder.
GCC/Clang and cumulative suites pass.

## Commit boundary

One independently validated commit: `delivery: M22 slice 08 record original retail scenes`.

## Dependency refinement

08A is deliberately inserted before this slice after the real, output-redacted
factory campaign probe reached `W3DTerrainVisual::init` and fail-closed on a
retail-enabled terrain configuration before map/scene consumption.  08A adds
no retail scene support: it makes the factory's already published edge
selectable as Recording only under a host/test profile and reports an
enumerated aggregate configuration mask without paths, names, bytes or hashes.
08B owns the source closure for the reached bits before 08 may proceed.  08
must use that accepted closure to implement only the reached source family;
it may not turn the guard into permissive success.
