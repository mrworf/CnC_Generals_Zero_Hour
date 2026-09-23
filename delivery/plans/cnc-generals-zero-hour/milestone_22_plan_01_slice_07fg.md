# M22 plan 01 slice 07FG: production original map-route publication

## Outcome and dependency

Requires 07FG0. Promote the already accepted opt-in original factory path from
the empty/generated-rigid diagnostic to the bounded generated map profile,
without changing default headless behavior. The real GameClient factory,
display, tactical view and terrain visual must publish atomically and drive
the 07FF source-owned map frame; a map-loaded guard or a parallel Linux
renderer is a failure.

07FG0 supplies only the public bgfx sampled 2D mip allocation/upload/recreate
transport required by the generated atlas. This slice remains responsible for
the production factory boundary; it must not replace physical device evidence
with a Recording-backed shortcut.

## Acceptance

Use a generated profile to prove factory identity, map load/update/draw,
source command family coverage, startup rollback, reset/recreation hooks and
clean shutdown. Keep private retail selection exclusively in slice 08 and
physical Vulkan/resize/visual acceptance exclusively in slice 09.
