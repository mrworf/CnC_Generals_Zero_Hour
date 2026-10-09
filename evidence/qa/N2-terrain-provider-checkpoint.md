# N2 native TerrainVisual CPU binding checkpoint

Parent: `8b3cef21edc0a03a40a0695a3fa0bef54e4a3735`.
Packet: `linux-upstream-v2`; existing N2 plan01/slice03 remains incomplete.
No dependency/vendor, retail asset, proprietary derivative or recovery changes.

## Behavior and limits

NativeTerrainVisual derives from the actual source TerrainVisual and binds CPU
virtual methods to retained full terrain/topology and vertex-water ownership.
Real TerrainTypeCollection lookup preserves construction restrictions. Actual
virtual lower-only height mutation reaches ground queries and signals dirty
height state only after accepted change. Source oversize remains inert.
Profile init, water resolution/clamps/attenuation/transforms/height/velocity,
null optional matrix/resolution outputs and actual GameLogic frame0 dispatch are
covered. Reset keeps prior terrain/enabled backing and clears filename/waves.
Nonallocating state exchange supports the enclosing root candidate publication.
Filename entry uses rooted CachedFileInputStream and actual blending configuration;
this checkpoint tests empty/missing filenames and generated stream admission,
not a successful retail startup or complete file-entry fault manifest.

The class remains abstract for image-backed color, picking, presentation and
complete device snapshot. Test-only output methods are not production renderer
behavior. Original device version3 snapshot includes render-owned state after
water and height bytes; no truncated replacement snapshot is claimed. Actual
TerrainLogic/layer/bridge/pathfinder, enclosing-world provider rollback,
partition/ghost/root startup and compiler-matching simulation remain pending.
Multiple-frame water behavior is independently covered by native_water; this
checkpoint's actual GameLogic dispatch covers frame0, not whole simulation.

Three provider families repeat three whole owners and exact memory-manager
retirement. Every malformed physical prefix is followed by corrected admission;
optional lighting is placed first so tested prefixes cannot be valid omission.
Full provider standard allocation prefixes[0,61), exact terminal61 and every
corrected retry preserve accepted map/lighting/filename/dirty publication.
Virtual water enable and Y-only resize each retain their failure/retry with exact
terminal1. Late invalid profile init preserves accepted enabled water backing.
The retained source graph/pool/world tests remain in the affected matrix.

## Frozen source and validation

Canonical616 SHA256:
`2dabfa052f0aba475517696eb6f57a475436b554984e21f002b3577d8b701603`.
Toolchain-inclusive623 SHA256:
`4b9433ec3604bf0c1857ecdd0fe14da99e34877f8bf3745f3ba156bc1986b21d`.
Existing changed-path/path-NUL-content-hash recipe; docs/evidence/state excluded.
All3 full configured builds PASS. Focused3 PASS GCC6.17s,
GCC ASan/UBSan/LSan26.49s, Clang ASan/UBSan/LSan16.61s.
Related80 PASS GCC37.46s, GCC ASan/UBSan/LSan177.46s,
Clang ASan/UBSan/LSan114.70s.
No sanitizer suppression, leak disabling, timeout increase or isolated bus.
Full configured293 suite has not run; this is not slice03/N2 acceptance.

```sh
cmake --build build/<configuration> -j 3
ctest --test-dir build/<configuration> -R 'original_native_terrain_provider' --output-on-failure
ctest --test-dir build/<configuration> -R 'original_(waypoint_graph|native_terrain|logical_world|native_water|header_paths|map|chunk|runtime|core|enum|source_boundaries)' --output-on-failure
```

Local logs: `/tmp/zh-n2-provider-{gcc,gcc-sanitize,clang-sanitize}-final-build.log`,
`-final-focused.log` and `-related.log`. Earlier focused logs retain fixture
failures. Initial fixtures used a single class while asserting an unknown class,
then queried a split-class coordinate selecting OtherTerrain rather than
GeneratedTerrain. The corrected authored map/coordinate tests both actual known
and unknown types. Prefix rejection initially treated optional final lighting
omission as invalid; corrected lighting-first input preserves source omission.
No production format/lookup behavior was weakened to satisfy those fixtures.

Next: native TerrainLogic queries, actual layered/pathfinder/Bridge semantics,
then staged original world/waypoint/provider/root admission. The source bridge
buffer registers gameplay bridges after model-derived BridgeInfo; it cannot be
dropped as visual-only. Source anchors and exact pending limits are recorded in
`docs/original-engine-formats.md`.
