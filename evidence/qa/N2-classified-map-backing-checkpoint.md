# N2 original classified-map backing checkpoint

Active packet `linux-upstream-v2`, existing N2 plan01/slice03. Parent:
`b1f32299f741bb959826e93b8ff99f89053cce86`. This is supporting CPU ownership
and generated terrain-admission evidence, not startup or milestone acceptance.

## Frozen source and validation

Canonical622 SHA256:
`653415f3b379ba6141d140a746771b4758777cb8cc8eb4eb609a4715ce8feefd`.
Toolchain-inclusive629:
`8864c904fef64a4683baff7d249f7264865b6deef0fdc471d6a446ff580b1e0e`.
All3 full configured builds PASS. Configured suite: 304 tests; full304 not run.

| Configuration | Focused8 | Related91 |
| --- | --- | --- |
| GCC | PASS, 4.61s | PASS, 52.57s |
| GCC ASan/UBSan/LSan | PASS, 19.63s | PASS, 253.49s |
| Clang ASan/UBSan/LSan | PASS, 11.99s | PASS, 162.10s |

Build commands: `cmake --build build/original-core-gcc -j3` and separately
`cmake --build build/original-core-{gcc-sanitize,clang-sanitize} -j2`.
Normal-host CTest with `--output-on-failure`; focused regex
`^original_ai_pathfinder_|^original_logical_world_pathfinder-map`, related regex
`original_(ai_pathfinder|waypoint_graph|native_terrain|logical_world|native_water|header_paths|map|chunk|runtime|core|enum|source_boundaries)`.
ASan leak checks and UBSan remain enabled/unsuppressed as configured; no bus
isolation or graphics fixture is involved in this CPU batch.

## Direct evidence

Every family repeats three whole owners. The actual source classes execute,
not an alternative pathfinding implementation:

- `ZoneBlock` four-array growth preserves accepted backing, origin and first/count
  metadata through each allocation failure; every corrected retry succeeds.
- `PathfindZoneManager` six equivalence arrays and paired block/index backing
  execute the original mixed-cell zone algorithm on a 24×24 checkerboard,
  including nine source zone blocks with clipped edge blocks. Ground/water
  equivalence spans the blocks; complete ownership retires exactly.
- Actual `PathfindLayer` backing helper acquires contiguous cells and indexes
  together. Cells own real acquired PathfindCellInfo units; failed growth preserves
  identity and those units, successful growth retires them, and repeated backing
  retirement is safe. Extreme coordinate subtraction and unsafe dimension
  products reject or return absent cells without overflow. This tests backing
  ownership, not model-derived bridge geometry.
- `Pathfinder::newMap` executes through actual AI/GameLogic and native logical/
  visual terrain providers with a generated original nonempty logical world and
  retained visual map. It classifies 25 ground/cliff cells and source zones.
  GameLogic has no registered simulation actors in this fixture; the nonempty
  map object graph is not actor/footprint or whole-simulation acceptance.
- Same-size second admission preserves actual ground-cell identity and has an
  exact zero-allocation terminal. Complete initial-admission faults retire the
  incomplete grid, layers and zone storage. Every failed attempt is followed by
  a direct corrected newMap retry, without a reset hiding partial-state defects.
- Unadmitted ground/layer queries are safe. A wall list with no surviving
  registered objects is empty rather than using uninitialized geometry bounds.
- Source 14-bit zone packing is preserved. Cell/layer setter overflow rejects
  before truncation, completed block input cannot contain the uninitialized zero
  marker, and excessive source classification rejects before indexing its fixed
  equivalence workspace. No malformed-input fixture relies on UB.

Complete standard-allocation manifests: layer backing terminal2, source block
growth terminal4, complete mixed-zone owner terminal46, actual generated-terrain
map admission terminal14. All prefixes and corrected retries are retained under
all3 configurations, together with prior cold/warm AI, override/group manifests
and separate actual typed-pool exhaustion. Original used-pool and standard heap
counts return to warmed baselines and whole memory-manager shutdown is exact.

## Initial failure and remaining gates

The first normal focused8 run passed7 but failed the zone-negative oracle: the
fixture wrote65535 into a 14-bit field and incorrectly expected the algorithm to
observe that value. Source review corrected the fixture to representable packed
values plus the explicit uninitialized marker, and inspected assignment/classifier
limits together. The rebuilt frozen focused8 passes all3; no test was weakened
to accept packed truncation or workspace overflow.

Local newMap failure recovery withdraws candidate readiness and retires its grid;
it does not preserve a formerly accepted root for continued simulation. Outer
root-candidate publication/rollback must still enclose those phases. Live-grid
resize retains the original explicit-reset precondition. Borrowed bridge/layer
descriptors and wall registrations remain available for rebuilding, but their
nonempty participant retry is pending integration evidence.

Nonempty registered wall/object footprints, bridge source bounds/height and
model-derived geometry, bridge/tower construction rollback, concrete root and
partition/ghost ordering, original startup and matching original simulation remain
mandatory. No GPU or retail-game acceptance is claimed. Assets, recovery storage
and adopted dependencies were not modified. N2 and slice03 remain incomplete.
