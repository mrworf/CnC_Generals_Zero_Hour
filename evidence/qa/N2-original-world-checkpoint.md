# N2 original world ownership checkpoint

Packet `linux-upstream-v2`; N2 plan01/slice03 remains **incomplete**.
Parent: `669d65f41daba83ee7750f377679660250d76c5b`. Commit subject:
`delivery: integrate transactional original polygon worlds`.
This is actual PolygonTrigger parser/owner support, not whole-world, startup,
rendering, gameplay or milestone acceptance. No agents were used.

## Frozen source and configuration

Canonical601 SHA256:
`501c158872a3f11723a47fd178b6dbb046e0a6b77ed9d7f0529f556e309a3b1e`.
Toolchain-inclusive608 SHA256:
`288968266db177084c6481c4ef83982c3a282c4cc0e5c839bbfaca4a4445dca5`.
The existing checkpoint recipe hashes sorted baseline-different source/test/build
paths plus new nonignored paths, as path/NUL/binary file SHA256; toolchain files
are included only in the second identity. Workflow prose is not part of this
source identity. Specification hash remains
`0316702c3fb75979f3ef5cdafb3adcf3a4c1bfa3dc84e94f883aceb6ce49bf90`;
source d0483ca9 and planning f92a3e5c remain ancestors; no packet/scope change.

All configured builds passed in `build/original-core-gcc`,
`build/original-core-gcc-sanitize`, `build/original-core-clang-sanitize`.
Native execution was on the normal host. Sanitizer tests retain
`ASAN_OPTIONS=detect_leaks=1:halt_on_error=1` and
`UBSAN_OPTIONS=halt_on_error=1:print_stacktrace=1`; no suppression, weakened
coverage, timeout increase or environment bypass. Each family has timeout60.

## Validation

Focused command (each configuration):

```sh
ctest --test-dir <build> -R '^original_polygon_world_' --output-on-failure
```

All5 families PASS: functional, negative, edit, snapshots, faults. Times:
GCC7.81s, GCCsan35.78s, Clangsan20.64s. Each whole owner repeats three times;
live allocations, original pool usage and static publications retire exactly.

Related command (each configuration):

```sh
ctest --test-dir <build> -R 'original_(polygon_world|native_terrain|header_paths|map|chunk|runtime|core|enum|source_boundaries|transfer)' --output-on-failure
```

All77 PASS: GCC18.07s, GCCsan90.10s, Clangsan59.14s. These are affected original
map/terrain/chunk/transfer/core/source contracts, not the full configured263
suite. Full263 was not run; required milestone-level acceptance is still pending.

## Delivered behavior and coverage

- Actual original PolygonTrigger list and next-ID cursor stage/exchange/retire
  together. Malformed and failed candidates restore exact accepted node/point
  identity and cursor. Owner exchange has zero allocations with faults armed.
- All four source chunk versions preserve field defaults, file order, water/
  river/layer information, source ID lookup, discarded short polygons and v1
  default water rectangle/ID policy. Actual original writer/reader round trips
  pass. Empty replacement retires the list and restores source cursor1.
- All-version truncated-prefix sweeps, bad versions/counts/IDs, late trailing
  payload, impossible sizes and large forged envelopes reject. Missing v1
  configuration and unrepresentable/nonfinite extents preserve accepted state.
  Actual FileSystem and GlobalData are used with an empty generated read-only
  root; no retail input is read or modified by this fixture.
- Constructor fields initialize before point acquisition; point growth changes
  capacity only after acquisition. Failed insert growth preserves backing and
  points, and corrected retry works. Source insertion/removal/clamped queries,
  empty queries, real water handles and full-width coordinate queries pass.
- Actual original XferLoad snapshot dispatcher/adapters exercise unchanged
  version/field order, all truncated prefixes, malformed counts, atomic late
  rejection and corrected retries. Serialized counts grow/shrink/empty without
  using internal allocation capacity as file authority. Complete physical point
  reads precede growth; save/CRC does not acquire candidate arrays.

Complete discovered acquisition manifests, identical across the three builds:

| Chunk version | Four points | 96 points |
| --- | ---: | ---: |
| 1 | 15 | 25 |
| 2 | 14 | 24 |
| 3 | 14 | 24 |
| 4 | 20 | 30 |

Snapshot growth manifest6 includes its final backing allocation. Every ordinal
in each `[0,census)` has an observed failure, accepted-owner identity/cursor
checks and a distinct corrected candidate retry; ordinal `census` is the proven
exact successful terminal. Each sweep repeats three times with whole-owner
retirement. No prefix or failure/retry pair was dropped.

## Iteration findings and limits

Initial targeted compilation rejected assignment to the read-only TheGlobalData
macro; the fixture now publishes through actual TheWritableGlobalData. Initial
functional/fault fixtures lacked GlobalData's required FileSystem, and malformed
parse false was initially reported as an assertion instead of the corrupt-input
error. Those were fixture defects, corrected without changing production to
accommodate them. The complete owner batch was then frozen and validated above.
Snapshot source audit also found internal spare capacity must not define a new
serialized count limit; physical-record staging and complete growth tests were
added before the final frozen cohort. No frozen-cohort failure or timeout occurred.

Durable source findings and next-owner risks are in
`docs/original-engine-formats.md`, “Original world publication and trigger
lifecycle checkpoint”: source object/Dict/weather publication, side/build/team
copy/parser ownership, static nested script admission and actual waypoint versus
visual-ground-provider ordering. These findings are not acceptance claims.

The original v1 auto-water ID can duplicate a retained maximum ID; that source
policy and the original radius Y-sum formula are preserved, pending explicit
original-bug qualification. Whole-map atomicity still requires enclosing all
owners: a successful trigger chunk alone cannot roll back a later unrelated
chunk. Actual sides/scripts/teams/world weather, full object parsing, waypoint
provider/layer ordering, NativeTerrainLogic, ghost/partition behavior and native
GameEngine/root execution with compiler-matching original simulation remain
required. Lighting/textures and audiovisual behavior are not inferred from these
generated owner fixtures. Assets, dependencies and recovery were untouched.
