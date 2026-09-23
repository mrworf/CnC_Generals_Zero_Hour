# M22 plan 01 slice 07FG: production original map-route publication

## Outcome and dependency

Requires 07FG0. Promote the already accepted opt-in original factory path from
the empty/generated-rigid diagnostic to the bounded generated map profile,
without changing default headless behavior. The real GameClient factory,
display, tactical view and terrain visual must publish atomically and drive
the same `W3DTerrainVisual::load` map boundary used by the accepted 07FF
source-owned frame; a map-loaded guard or a parallel Linux renderer is a
failure.

07FG0 supplies only the public bgfx sampled 2D mip allocation/upload/recreate
transport required by the generated atlas. This slice remains responsible for
the production factory boundary; it must not replace physical device evidence
with a Recording-backed shortcut.

## Acceptance

Use a generated profile to prove factory identity, map load/update/draw,
startup rollback, reset/recreation hooks and clean shutdown. The required
coupled acceptance is the accepted 07FF Recording marker for exact full source
family ordering (`terrain → tracks → decal shadow → water → particle → smudge`)
plus this slice's physical factory-map lifecycle run on the shared original
terrain-map boundary. The factory has no scenario drawables, caster, or smudge
producer; adding them just to reproduce that marker would invent a second
production path. This composition therefore proves family coverage without a
Recording-backed factory shortcut. Keep private retail selection exclusively in slice 08 and
physical Vulkan/resize/visual acceptance exclusively in slice 09.

## Scope and boundary

The sole new opt-in is `ZH_M22_FACTORY_MAP`, honored only with
`ZH_M22_ORIGINAL_FACTORY_PROFILE` after the real `W3DDisplay`, `W3DView`,
`W3DTerrainVisual`, public bgfx device edge, and 07FG0 mip transport are live.
It loads one generated, read-only map through `W3DTerrainVisual::load`, then
lets the existing GameEngine/GameClient/W3DDisplay update and draw path render
the frame.  The ordinary absent-environment factory stays mapless and the
device-only control stays ownerless.  No Recording device, parallel renderer,
retail map selection, resizing, or visual-quality assertion is added.

## State, failure, and teardown contract

The map route sets the map-required partition scale only inside the explicit
profile and only before `load`; it must not leak into ordinary startup.  A
missing, malformed, or failed map load returns a startup error before frame
publication, and the established main rollback confirms no published owners or
allocation residual.  Map-loaded reset may retire an explicitly known-empty
factory bib boundary, but continues to reject a foreign terrain visual, global
alias mismatch, or any unimplemented/nonempty bib producer.  The normal
reverse GameEngine reset releases the map, scene attachment, terrain tracks,
shadow/water/smudge siblings, device-edge aliases, and all resources; two
fresh processes demonstrate re-entry/recreation.

No authorization changes apply.  Generated files stay under the test temporary
directory and source fixture roots remain read-only; the original-game symlink
is neither selected nor modified.

## Implementation and test surfaces

- `src/original_runtime/linux_game_engine.cpp`: the guarded factory-map load
  boundary and no-map-compatible reset ownership.
- `GeneralsMD/.../W3DTerrainVisual.cpp`: narrow map-loaded no-bib cleanup
  permit while retaining foreign and unimplemented bib rejection.
- `tests/original_rendering/test_w3d_factory_map.py` and `CMakeLists.txt`:
  generated physical factory integration fixture and GPU test registration.
- Governing index, this plan, and 07FG evidence.

## Positive and negative proof

The fixture uses the existing bounded generated `Flat` terrain class/TGA/map
and calls the real executable with the original factory profile.  It asserts
one original display/view/terrain factory publication, a nonempty factory
frame marker, no surviving aliases/resources, and two independent
generations.  It separately proves no `ZH_M22_FACTORY_MAP` preserves the
mapless control; missing and malformed packets fail before frame publication;
a forced factory terrain failure and a forced device failure retain existing
rollback; and a map-loaded bib cleanup accepts only the known empty owner while
foreign aliases/nonempty producer routes fail closed.

## Validation and commit boundary

Run focused factory-map and coupled source controls, GCC/Clang sanitizer
source-owner leak checks, the proportional six configured non-GPU/non-LAN
CTest suites (sanitizer broad CTest may use `detect_leaks=0` only for ptrace),
serial LAN 4/4 in every build, and direct Khronos Vulkan with output scan. Run
identity/provider/ledger and `git diff --check`, write evidence, mark the index
complete, stage only 07FG paths, and commit one independent slice. Preserve the
pre-existing `tests/renderer/test_bgfx_device.cpp` diagnostic unstaged.
