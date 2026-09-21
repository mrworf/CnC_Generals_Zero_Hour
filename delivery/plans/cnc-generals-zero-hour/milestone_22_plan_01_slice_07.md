# M22 plan 01 slice 07: original retail scenes through recording device

## Outcome and dependencies

Requires slice 06 and read-only PRE-008 corpus. Original campaign/skirmish
entry, logic, GameClient, WW3D and WWShade producers load selected real retail
scenes and emit complete frames through recording `GpuDevice`. The production
source graph owns all terrain, model, HLOD, animation, texture, shroud, fog,
lighting, shadow, particle, water, effects and pass decisions actually reached.
Generated/test proxy scenes cannot substitute for retail acceptance.

## Validation and error handling

Retail roots remain read-only/private; no corpus bytes, private paths,
hashes or sensitive screenshots in committed output. Record source identity,
concrete families, scene sizes, pass/state/draw counts and resource handles.
Negative controls remove/corrupt required owned-fixture resources, inject
failure at every device operation and exercise scenario-level required-asset
failure/reset/retry with no stale track/texture/render owners. Optional
missing models retain original optional semantics. Provider removal rejects
loss of original consumer, WW3D, WWShade, adapter or recorder. GCC/Clang and
cumulative suites pass.

## Commit boundary

One independently validated commit: `delivery: M22 slice 07 record original retail scenes`.
