# N2 retained visual-owned CPU terrain checkpoint

Parent: `029730a6cb11c2175103fd46c8cbb1b0d1e3fb14`.
Packet: `linux-upstream-v2`; existing N2 plan01/slice03 remains incomplete.
No adopted-library source, retail asset, recovery archive or proprietary
derivative changed. Tests use generated in-memory map contents.

## Delivered behavior

NativeTerrainHeightMap now admits the full retained CPU height/blend/lighting
phase as one transaction, using actual DataChunkInput and actual GlobalData
configuration. The loader skips logical world owners, preserves omitted lighting
and optional shadow color, initializes old-version absent lights as the source
does, and publishes only after the whole stream succeeds. Source terrain-name
selection and lower-only construction deformation are included, without changing
authored cliff flags or normal height/LOS semantics. Terrain-name division and
ground-query reciprocal multiplication retain their distinct source arithmetic.

Nine terrain families include the previous six and three new independent
visual/visual-negative/visual-faults families, each across three whole owners:

- All lighting versions1–3 against all blend versions1–8, both lighting orders,
  source raw directions/colors, row0 retention, missing/optional lighting/shadow.
- Packed tile selection, floor/border/clipping, first overlapping texture class,
  and skipping logical object contents instead of reparsing their owners.
- Border-adjusted raw heights, unchanged equal/raised terrain, actual lower
  mutation reaching ground queries, retained authored cliffs and invalid byte/
  coordinate admission without undefined conversions or changed backing.
- Every incomplete prefix with mandatory blend last; duplicate/invalid-version/
  invalid enum/nonfinite/trailing lighting; late blend failure after lighting.
  Every rejection retains accepted backing and all actual configuration fields,
  followed by a corrected same-owner retry.
- Complete visual-map allocation prefixes[0,61), exact success terminal61,
  every failure/retry and restoration, three complete owners and exact whole-
  manager retirement. Frozen runs confirm this census in each configuration.

Details and source evidence: `docs/original-engine-formats.md`, Retained
visual-owned terrain admission and deformation. This checkpoint is not actual
native TerrainVisual/TerrainLogic/root startup, image-backed color, renderer
acceptance or original simulation execution. Those remain required by slice03.

## Frozen source and validation

Canonical611 source SHA256:
`79af234130aced68a21a3b5731399114f400e4414f31851be99f204f08305464`.
Toolchain-inclusive618 SHA256:
`5483f9920102f5e32292339110750ee6bb50bf21c21f80fa6555d02e456e3502`.
Same changed-path/path-NUL-content-hash recipe as N2 logical-world and water
checkpoints. Documentation/evidence/workflow state do not enter these hashes.

All three configured builds use unchanged dependencies and existing Debug
GCC/GCC-sanitize/Clang-sanitize profiles. Sanitizer checks retain leak detection
and halt-on-error; no suppression, timeout change or isolated bus is involved.
Related72 includes all nine terrain families, all five logical-world families,
water and the source/header/map/chunk/runtime/core/enum checks affected by the
changed height-map owner/header. Unchanged sides/polygon/client/transfer families
retain prior evidence, not a claim of new execution here. Full configured285
tests have not run and N2/slice03 are not complete.

```sh
cmake --build build/<configuration> -j 3
ctest --test-dir build/<configuration> -R '^original_native_terrain_' --output-on-failure
ctest --test-dir build/<configuration> -R 'original_(native_terrain|logical_world|native_water|header_paths|map|chunk|runtime|core|enum|source_boundaries)' --output-on-failure
```

All three frozen configured builds pass. Frozen related72 passes GCC21.03s,
GCC ASan/UBSan/LSan108.39s and Clang ASan/UBSan/LSan71.92s. Local logs:
`/tmp/zh-visual-terrain-{gcc,gcc-sanitize,clang-sanitize}-frozen-build.log`
and `-related.log`; sanitizer CTest LastTest.log retains exact manifests.
The explicit normal GCC manifest is also retained in
`/tmp/zh-visual-terrain-gcc-frozen-manifest.log` (three terminal61 runs), because
a subsequent CTest discovery-only command replaced that temporary LastTest.log.

Earlier base focused9 passed GCC3.89s, GCC sanitizers32.76s, Clang sanitizers28.10s.
The expanded GCC focused3 passed after adding discriminating class/skip cases.
Final source review separated terrain-name division from the ground query's
reciprocal multiplication before running the frozen matrix. No assertion/fix
cycle occurred and no acceptance oracle or source behavior was weakened.
