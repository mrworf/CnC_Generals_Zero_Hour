# M22 slice 07FG: production original map-route factory publication

The opt-in `ZH_M22_FACTORY_MAP` profile now loads one bounded generated map
through the real original factory's `W3DDisplay`, `W3DView`, and
`W3DTerrainVisual::load` boundary after the public bgfx edge is live.  The
default factory profile remains mapless.  The map frame marker counts exact
source lifecycle publication; it deliberately makes no pixel or visual claim.

The physical fixture creates a writable generated `Flat` terrain input, seals
the source tree, and verifies two independent valid runs, absent-environment
mapless behavior, missing/malformed map rejection, forced terrain/device
rollback, one frame per valid generation, and zero aliases/residual ownership.
Map reset permits only the known-empty factory bib boundary after rejecting
foreign terrain aliases; all unavailable/nonempty bib-producer routes remain
fail-closed.

Full source family coverage is a required coupled proof, not an invented
factory producer: the accepted 07FF `original_w3d_shadow_decal_route` marker
records the exact original owner order `terrain → tracks → decal shadow → water
→ particle → smudge`.  This slice physically executes the same original
`W3DTerrainVisual::load` map boundary in the production factory.  The factory
has no scenario drawable, shadow caster, or smudge producer; adding those just
to reproduce the Recording marker would create a parallel production route.

Final-tree acceptance:

- All six rebuilt non-GPU/non-LAN suites passed fresh **206/206**: GCC
  Debug/Release, Clang Debug/Release, GCC ASan+UBSan, and Clang ASan+UBSan.
  The broad sanitizer CTest invocations used `LSAN_OPTIONS=detect_leaks=0`
  only for the established sandbox/ptrace limitation.
- Host `ASAN_OPTIONS=detect_leaks=1` source-owner focus passed **6/6** in both
  sanitizer configurations: terrain atlas, particle provider, 07FF coupled
  shadow/decal route, presentation identity, provider removal, and the
  dependency ledger.  The sandbox reproduction fails only because
  LeakSanitizer cannot operate under ptrace; the identical host rerun is clean.
- Direct host Vulkan with `VK_LAYER_KHRONOS_validation` passed the physical
  07FG factory-map fixture via `tools/run_validation_clean.py`, with no
  validation output.  The coupled 07FF marker passed **1/1**, and existing
  physical factory bootstrap/rigid controls passed **2/2**.  The retained
  original view-scene control passed **1/1** and concretely exercises repeated
  known-empty bib cleanup, a fail-closed bib creation attempt, and foreign
  published-terrain rejection; 07FG does not relax either producer or foreign
  owner condition.
- Serial local UDP/LAN validation passed **4/4** in each of all six builds.
  Final original dependency-ledger validation and `git diff --check` passed.

The pre-existing `tests/renderer/test_bgfx_device.cpp` diagnostic remains
unstaged and is not part of this slice.
