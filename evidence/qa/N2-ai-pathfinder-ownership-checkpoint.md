# N2 actual AI/pathfinder ownership checkpoint

Packet `linux-upstream-v2`, existing N2 plan01/slice03; transaction parent
`7176f853623b56f38b535d17128c91e3e1ddd53d`. This is supporting original-runtime
ownership evidence, not milestone or full startup acceptance.

## Frozen source and validation

Canonical622 source SHA256:
`6f61efe126b833f504210c247b2b7be185c0c8738daea769c98e2b1dd8c993f8`.
Toolchain-inclusive629:
`5e82472b49f1e9ed2b5db9db1a52b15f819913e845daf649eb95f1716d8d6e63`.
Specification remains `0316702c3fb75979f3ef5cdafb3adcf3a4c1bfa3dc84e94f883aceb6ce49bf90`.
All three full configured builds pass. Configured suite: 300 tests; full300 has
not run at this incomplete milestone checkpoint.

| Configuration | Focused5 | Related87 |
| --- | --- | --- |
| GCC | PASS, 2.09s | PASS, 47.87s |
| GCC ASan/UBSan/LSan | PASS, 9.78s | PASS, 234.19s |
| Clang ASan/UBSan/LSan | PASS, 5.73s | PASS, 150.89s |

Commands: `cmake --build build/original-core-{gcc,gcc-sanitize,clang-sanitize} -j2`
(each configuration separately). Focused CTest regex:
`^original_ai_pathfinder_|^original_logical_world_terrain$`.
Related regex:
`original_(ai_pathfinder|waypoint_graph|native_terrain|logical_world|native_water|header_paths|map|chunk|runtime|core|enum|source_boundaries)`.
CTest executes on the normal host with `--output-on-failure`. Sanitizers retain
`ASAN_OPTIONS=detect_leaks=1:halt_on_error=1` and
`UBSAN_OPTIONS=halt_on_error=1:print_stacktrace=1`; no suppression or fixture
isolation is used for this CPU-only batch.

## Proven behavior

`tests/original/ai_pathfinder.cpp` directly constructs the actual AI, Pathfinder,
PathfindCell, TAiData and original pooled side/build/group nodes. Three complete
owners repeat every family and retire heap units, original used pool units,
cell-info owner/unit counts and whole memory-manager ownership exactly.

- Concurrent Pathfinder owners preserve already-acquired cell units. Last-owner
  release retains backing until the last acquired unit retires; reacquisition
  does not evict that unit. Acquisition without an owner is rejected.
- All 30,000 source cell-info units can be acquired, exhaustion returns null,
  released units are reused, and actual cell repeated reset retires its unit.
- Null, foreign and duplicate releases reject before ownership mutation.
- Nonempty side/build override clones publish atomically, preserve source list
  prepend reversal/build-node append order, and retain independent pooled nodes.
  Empty build lists remain valid. Optional unconfigured skill sets have defined
  zero-active defaults rather than indeterminate source counts.
- Actual group registration, explicit deletion, reset and whole-owner destruction
  preserve original IDs. Failed list insertion retires the group and restores
  its ID cursor. Legitimate TheAI publication remains live through destruction.
- Native TerrainLogic dispatches an unclipped wall query through the real
  Pathfinder wall height (40), and a clipped query with no wall pieces returns
  actual terrain ground. Missing AI still rejects; no ground-only substitute.

Complete standard-allocation manifests have exact terminals cold AI3, warm AI2,
override1 and group1. Every prefix fails, preserves accepted ownership, and is
followed by a successful corrected retry in the same process. Separately exhaust
actual AISideInfo, AISideBuildList, BuildListInfo and AIGroupPool pools; verify
rollback and a corrected retry for each. These tests do not substitute standard
allocation injection for original typed-pool exhaustion.

## Failure classification and limits

An initial pre-correction fixture binary used the nonexistent pool name
`AIGroup` instead of source-selected `AIGroupPool`: all four fixture families
failed their registry/exhaustion checks. Source inspection corrected the test
name, followed by rebuilding and passing all focused families. This was a
fixture defect; no production behavior was changed to satisfy it.

No retail assets, recovery archive or dependency sources were modified. No GPU
evidence or whole-game parity is claimed. Classified map/layer/zone backing,
nonempty wall geometry/group members, model-derived bridge bounds and actual
registered bridge objects remain pending together with concrete root admission,
partition/ghost ordering, original startup and matching original simulation.
Source anchors and partial-allocation retry hazards are recorded in
`docs/original-engine-formats.md`. Slice03 and N2 remain incomplete.
