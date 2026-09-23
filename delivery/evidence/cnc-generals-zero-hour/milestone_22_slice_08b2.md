# M22 slice 08B2: bounded cloud-plane source owner

08B2 closes only aggregate configuration-mask bit 4.  A source-owned water
plane may be created as a cloud layer only when the existing bounded water
owner is active, uses the supported type-0 nonzero extent, and remains in its
own primary scene.  The cloud choice is fixed at owner creation; changing the
configuration while it is live fails closed.  Volumetric shadow, decal,
reflection/grid, disabled-water, foreign-owner, and unrelated terrain
configuration paths remain rejected for later slices.

Generated Recording acceptance exercises one ordinary-water generation and
two cloud-plane generations.  It witnesses the source water marker and the
terrain/tracks/water/particle/smudge order, duplicate and foreign scene-owner
rejection, disabled/invalid extent/unsupported reflection rejection,
create/upload/draw failure rollback and retry, reset/re-entry, removal, and
zero resources after teardown.  No retail data was read, copied, named, or
recorded; this is not a pixel or retail reachability claim.

Final-tree acceptance:

- Fresh non-GPU/non-LAN suites passed **206/206** in GCC Debug/Release,
  Clang Debug/Release, GCC ASan+UBSan, and Clang ASan+UBSan.  Broad sanitizer
  CTest used `LSAN_OPTIONS=detect_leaks=0` only for the established ptrace
  limitation.
- Host `ASAN_OPTIONS=detect_leaks=1` focused water, tracks, identity,
  provider-removal, and dependency-ledger checks passed **5/5** in both
  sanitizer configurations.
- Validation-layer physical factory bootstrap, rigid, and generated-map
  checks passed **3/3**.  Serial LAN validation passed **4/4** in every one
  of the six configurations.
- The final original dependency-ledger validator, identity/provider checks,
  and whitespace diff check passed.

The existing `tests/renderer/test_bgfx_device.cpp` diagnostic remains
unstaged and is outside this slice.
