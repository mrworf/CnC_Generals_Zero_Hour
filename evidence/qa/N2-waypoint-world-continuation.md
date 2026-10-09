# N2 original waypoint and world-continuation checkpoint

Parent: `253c2e46353c2061ebd6148b275948f2d1d2c789`.
Packet: `linux-upstream-v2`; existing N2 plan01/slice03 remains incomplete.
No dependency/vendor, read-only retail asset, proprietary derivative or recovery
archive changes. All tests use generated worlds and disposable fixture roots.

## Behavior

NativeWaypointGraph owns the actual pooled Waypoint nodes and shares source
constructor/prepend/link/read/delete operations with original TerrainLogic.
This is not a second substitute waypoint implementation or simulation.
The original eight-slot accessor now rejects index8. Graph admission stages the
entire candidate; links retain reverse file traversal, first-file duplicate-ID
selection, missing/self/duplicate suppression, source-only bidirectionality and
independently admitted reverse links after forward saturation. Unsigned high-bit
IDs preserve their signed wire bit patterns. Resnap separates initial absent/
prior retained grid from the later newly retained terrain, and validates all
arithmetic before any accepted node changes. Nonallocating head exchange supports
the next actual TerrainLogic publication boundary.

NativeLogicalWorld's synchronous CPU continuation holds its existing object/
Dict/weather/trigger/side/read/name/script-ID rollback through later admission.
It runs after source side validation, before all accepted-world swaps/commits.
The accepted side parent's script identity and accepted height backing remain
unchanged while candidate sides/world owners are legitimately published. Caller
graph/terrain/root owners must stay staged; this hook does not roll back arbitrary
external effects and must not run on renderer/audio threads.

Four graph families, each over three whole owners, cover functional semantics,
every incomplete link-file prefix, duplicate/negative/impossible/trailing link
input, complete standard allocation prefixes[0,4) and exact terminal4, and actual
Waypoint-pool exhaustion with corrected retry. Heap and actual used pool units
retire exactly. The source node strings share AsciiString backing and explicit
pools own node allocations; the small standard census is not proof of pooled
coverage, hence the separate pool test.

The sixth logical-world family calls the continuation with actual source
MapObjects and constructs actual pooled Waypoints. Explicit later failure restores
all accepted identities/cursors/backing. Fresh WorldInfo namespace admission also
rolls back after this later phase; retry commits the identical key ordinal and
world dictionary. Combined allocation prefixes[0,175) and exact terminal175 keep
every failure/retry pair across three owners, including failures inside graph
admission after the logical parse. Original ten pool/world families remain in the
related matrix.

## Frozen source and validation

Canonical614 SHA256:
`cbbeff2d1ee2d8adbdb8f7c82ce451166952a0c8ecc98388f1b19117cd8e126b`.
Toolchain-inclusive621 SHA256:
`b2f07b54df9e5860ad3d6e498aa20fc902bb4ea8ecf909ce3d0844fe5413c674`.
Use the existing changed-path/path-NUL-content-hash recipe from prior N2
checkpoints. Evidence/docs/state are excluded. All3 configured builds pass.

Focused5 PASS GCC2.23s, GCC ASan/UBSan/LSan11.34s,
Clang ASan/UBSan/LSan6.73s. Leak checks remain enabled with no suppression,
timeout change or isolated bus. Frozen related77 PASS GCC23.30s,
GCC ASan/UBSan/LSan119.95s and Clang ASan/UBSan/LSan78.78s. Each run retains
exact terminals4/175 and the actual Waypoint-pool failure/retry across three
whole owners. Full configured290-test suite has not run. Neither slice03,
N2, native provider/root startup nor original simulation parity is complete.

```sh
cmake --build build/<configuration> -j 3
ctest --test-dir build/<configuration> -R '^(original_waypoint_graph_|original_logical_world_continuation$)' --output-on-failure
ctest --test-dir build/<configuration> -R 'original_(waypoint_graph|native_terrain|logical_world|native_water|header_paths|map|chunk|runtime|core|enum|source_boundaries)' --output-on-failure
```

Local logs: `/tmp/zh-waypoint-{gcc,gcc-sanitize,clang-sanitize}-frozen-build.log`
and `-related.log`; focused/retry/build logs retain iteration history. CTest
LastTest.log captures exact terminal4/175 and actual pooled failure/retry results.

Initial GCC graph focused4 passed three families; negative incorrectly treated
a bare TOC as an opened source file. Actual DataChunkTableOfContents::read sets
opened only when payload remains. Corrected omission uses another complete chunk,
and now all incomplete prefixes reject; no production reader was weakened.
Initial world continuation completed its175 failure/retry manifest but the outer
fixture baseline hadn't warmed the newly used Waypoint pool metadata. The common
warm-up now exercises that owner before baselines; whole-manager teardown still
proves exact metadata retirement. Corrected normal and sanitizer cohorts pass.

Next: actual native TerrainVisual/TerrainLogic binding, keeping logical-world
rollback open through waypoint/provider/later root failure; retained full terrain
and CPU water, then layer/ghost/partition/presentation/root and original simulation.
Source common loadMap's legacy catch-all remains unsuitable for native ownership
failure handling. Detailed findings: `docs/original-engine-formats.md`.
