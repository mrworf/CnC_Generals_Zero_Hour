# N2 displayed-terrain segment query checkpoint

Status: frozen required matrix PASS all3. Not slice03/N2
completion, concrete view/root integration or physical rendering acceptance.

Transaction parent: `10b43a84977f750dee0f4fc66ceacd7cc0b7c831`; initial tree clean.
Existing governing plan01/slice03 was refined before production changes. No
framework/vendor/asset/recovery content changed. Plan/spec revision remains
`0316702c3fb75979f3ef5cdafb3adcf3a4c1bfa3dc84e94f883aceb6ce49bf90`.
Both handoff source/planning commits remain ancestors of this parent.

Frozen original source (same canonical/inclusive path recipe as text checkpoint):

- canonical640 SHA256 `f52556b3fc5a8a6afc9514b725b4faa80bf518aec465dbfc346ca5beab3af652`
- inclusive647 SHA256 `473cda148e8e687750242c192d45065264e25a37673bef5db3b3d9bdeb059f46`
- configured suite362; full suite not run (milestone completion gate).

## Delivered behavior and test scope

NativeTerrainHeightMap supplies an allocation-free real displayed-height mesh
segment query; NativeTerrainVisual exposes it through original TerrainVisual's
virtual intersectTerrain. Uses source diagonals, clamped edge vertices/overhang,
two-sided intersections, source start/end/coplanar rules and nearest-hit admission.
Corrects original short-circuit first-hit bug. Promoted intermediates and plane-Z
recovery make hostile/huge inputs defined. Output is optional, supports aliasing
and remains unchanged on miss/rejection. No contact cache or renderer internals.

New `rays`, `rays-negative`, `provider-rays` families repeat3 with actual original
memory manager, standard-allocation/whole-manager retirement and exact query
allocation terminal0. Cover independent source plane oracle for both triangles
in every generated cell, diagonal/edges, reversal/multiple hits, overhang,
surface start/exact end, parallel/coplanar/zero-length, unloaded/one-sample,
huge Z/hostile XY/nonfinite values, corrected retries, real virtual admission,
construction lowering/reset/replacement/exchange and separate water backing.
Existing terrain malformed/admission/fault tests remain required.

Source/ownership findings and limits: `docs/original-engine-formats.md`.
Development GCC initial new3 PASS2.94s. Proactive extreme-Z review then added
plane-Z recovery and explicit full-range tests before the frozen matrix; this
was not a failed-test/individual-assert/full-build cycle.

## Required frozen matrix

Each command is the full configured incremental build, then focused15, then
related172 on that exact frozen source. Individual tests retain timeout60;
GCC/Clang ASan/UBSan/LSan retain unsuppressed leak/error checks.

```sh
cmake --build build/original-core-<configuration> -j 4
ctest --test-dir build/original-core-<configuration> -R '^original_native_terrain_' --output-on-failure
ctest --test-dir build/original-core-<configuration> -R '^original_(native_font|text_ownership|script_camera|object_ownership|team_ownership|player_ownership|ai_pathfinder|waypoint_graph|native_terrain|logical_world|native_water|native_client|diplomacy|statistics|header_paths|map|chunk|runtime|core|enum|source_boundaries)' --output-on-failure
```

| Configuration | Exact terminal handle | Full configured build | Focused15 | Related172 |
|---|---:|---|---|---|
| GCC | 44939 terminal0 | PASS | PASS21.55s | PASS168.10s |
| GCC sanitizer | 22250 terminal0 | PASS | PASS112.07s | PASS727.05s |
| Clang sanitizer | 48724 terminal0 | PASS | PASS68.68s | PASS442.76s |

Logs: `/tmp/n2-terrain-rays-{gcc,gcc-sanitize,clang-sanitize}-{frozen-build,focused,related}.log`.
All three handles returned exit0. There are no live validation jobs and no
required frozen failures. Canonical640/inclusive647 digests were rechecked
unchanged before closeout. Full362 and actual root/view integration remain pending.

## Remaining scope

Terrain picking is separate from bridge/drawable selection, logical height/LOS
and water. This does not establish camera projection/transform/lock, source
drawable-before-view phase, actual native UI/headless initialization/frame,
popup/snapshot/version handling, dependency-aware root shutdown, CPU model
queries, full original startup/GameLogic/RNG/script journal or read-only retail
acceptance. Continue those in the same N2 plan01/slice03 after this checkpoint;
N3–N7 are still pending. No milestone acceptance was relaxed.
