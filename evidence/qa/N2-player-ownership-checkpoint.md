# N2 real player/cohort ownership checkpoint

This is supporting evidence inside existing N2 plan01/slice03, not N2 acceptance.
Parent: `7737aea10dcba4eb687f944064b58baf69ef5e85` (clean transaction start).
Commit boundary: `delivery: preserve original player cohort ownership`.

## Frozen scope

Canonical624 source SHA256:
`b2574cec2d7e5b16520e3f097d181d35dc01567d72d603df07378a498bb795da`.
Toolchain-inclusive631 SHA256:
`bb07b9f28e524e6a3fcc20cc8760223bee7a056ec0b8c084bf8bd98f0cd79ca7`.
Same baseline/path plus NUL plus binary file-digest recipe as prior checkpoints;
documentation/evidence/state are excluded. Configured suite308; full308 not run.
Specification hash0316702c and active packet linux-upstream-v2 unchanged.

Actual retained Player and PlayerList code runs with real NameKeyGenerator and
RankInfoStore. No replacement player/squad implementation or simulation is used.
Rank storage is legitimately empty; nonempty authored rank/science behavior and
actual GameLogic/AcademyStats startup remain pending. Optional client/partition
parents are absent in this CPU ownership fixture, following source conditional
dispatch. The fixture does not invent UI, teams or actors.

Constructor failure retires both acquired pooled relation maps and partial
selection backing. All raw owned fields initialize before fallible acquisitions.
Player init stages ten hotkey squads plus current selection as one complete
candidate before replacing old selection. Player/cohort destruction never calls
allocating init, and retires owned build/upgrade, optional AI/resource/tunnel,
production-change entries, timers, maps and selection instead. Tests exercise
nonempty two-node build chains, production-change reference counts and squad
contents; optional AI/resource/tunnel and nonempty upgrade ownership require
their real later startup participants and are not independently accepted here.
Production-change registration unwinds its pooled node if standard list insertion
fails. Neutral defaults, fixed sixteen-player capacity, initial one active player,
local-player changes and explicit reset behavior remain source-selected.

## Focused evidence

All3 configured full builds PASS (`cmake --build <configuration> -j2`).
Normal-host CTest, no suppressions; sanitizer configuration retains
ASAN_OPTIONS detect_leaks=1/halt_on_error=1 and UBSAN_OPTIONS halt_on_error=1/
print_stacktrace=1. Every family retains TIMEOUT60.

Command: `ctest --test-dir <configuration> -R '^original_player_ownership_' --output-on-failure`.

| Configuration | Focused4 | Related118 |
| --- | --- | --- |
| GCC | PASS, 0.23s | PASS, 81.35s |
| GCC ASan/UBSan/LSan | PASS, 2.43s | PASS, 369.27s |
| Clang ASan/UBSan/LSan | PASS, 1.25s | PASS, 237.44s |

Related regex:
`^original_(player_ownership|ai_pathfinder|waypoint_graph|native_terrain|logical_world|native_water|native_client|diplomacy|statistics|header_paths|map|chunk|runtime|core|enum|source_boundaries)`.

No failing focused run occurred. All configurations observed identical warmed
standard-allocation manifests; discovery retires before each fault/resource
baseline. Explicit typed pools are preconfigured to fixed, sufficient capacities,
not faulted or silently grown during these warmed heap sweeps.

| Actual operation | Failure prefixes | Proven exact terminal |
| --- | --- | --- |
| Player constructor/nonempty owners/destructor | [0,4) | 4 |
| Complete16-player cohort/nonempty owners/destructor | [0,81) | 81 |
| Player constructor/population/explicit init/destructor | [0,4) | 4 |
| Production-change pooled acquisition/list registration/removal | [0,1) | 1 |

Each failure and exact terminal is followed immediately by a corrected operation;
every pair proves both complete standard heap and original pooled used-unit
retirement. All families repeat across three owners, and whole original memory
manager shutdown returns to the process baseline. No IDs/names are advanced by
these neutral acquisition operations, so no source cursor lease is needed.

Separate actual typed-pool exhaustion covers PlayerRelationMapPool,
TeamRelationMapPool and Squad, with prior acquisition cleanup and corrected retry.
Squad replacement also tests every available-unit count0 through11: failure
prefixes[0,11), exact pooled terminal11. Every failed prefix retains original
hotkey identity/content and every acquired pool unit; every corrected retry
executes real init. Missing rank authority rejects constructor/reset cleanly;
reset preserves accepted selection before rejection. Armed ordinal0 observes
exact zero standard allocations during whole cohort destruction.

## Remaining outcome boundary

Player constructors do not create default teams. Actual team/prototype
publication and cursor rollback, Object lookup/ID/team/module/creation lifecycle,
nonempty registered wall/object footprints and model-derived bridge execution,
candidate root rollback/publication, partition/ghost/GameEngine startup and the
compiler-matching original simulation fixture remain mandatory. Do not infer
whole-game or N2 parity from this ownership checkpoint. See durable source
anchors and limits in `docs/original-engine-formats.md`.
Assets, recovery storage and upstream frameworks were not modified or consumed
by this generated ownership validation.
