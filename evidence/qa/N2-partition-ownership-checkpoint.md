# N2 nonempty partition ownership checkpoint

Supporting work in existing plan01/slice03, not N2 acceptance.
Parent `ce04706030bffa9f511d57207c08bf43d088c81e`; clean transaction start.
Commit boundary: `delivery: preserve original partition admission ownership`.

## Frozen source and scope

Canonical626 SHA256:
`f0dcee42be5fa88b88ba5c45b9ae6369ece3bd706f627cbe938e7463f9ae90ec`.
Toolchain-inclusive633 SHA256:
`88679126a0a6e2a3b38753556d8e66955e45980108cb4e42543b4c84946c650d`.
Same baseline/path-NUL/binary-file-digest recipe as prior checkpoints.
Configured327; full327 not run. Specification and upstream sources unchanged.

Actual NativeTerrainLogic admits the existing generated original world and
retained native terrain. The source 50-by-50 extent admits25 PartitionCells with
source cell size10; source minimum size1 admits2500 cells. Init/reset repeats;
duplicate admission preserves accepted backing and nonfinite profiles reject.
All fallible cell/radius-vector prefixes unwind before corrected same-owner
retry. Normal reset may retain legitimate radius-vector capacity; whole-owner
destruction retires it exactly.

Real original actors execute constructor, scheduler, team/energy/radar callbacks
under original name/rank/script/AI/module/player/team/UI parents, then register
with this nonempty partition. Original touched-cell updates and cell-change
maintenance execute. Candidate attachment failure preserves an already accepted
actor's identity and intersections. Every corrected retry populates then retires
actual intersections. Sphere, cylinder and box families execute; source final
diagonal COI sizing remains unchanged. Nonfinite/negative/overflowing radii
reject before backing publication. Actual fixed PartitionDataPool exhaustion
preserves survivor/candidate state and has a corrected same-actor retry. Complete
source actor destruction leaves no cell intersections or acquired partition units.

Parallel ghost-attachment tests exercise the actual PartitionData path with a
test-only concrete GhostObject carrying source geometry and the original W3D
pointer-update contract. Snapshot operations explicitly reject and are never
called. This is attachment/backing ownership evidence, NOT shipped ghost/fog,
renderer snapshots, visibility transitions or save/load acceptance. Mobile actor
tests use the common ghost parent solely for empty removal and never claim fog.
The source normal orphan-retention path is unchanged. Ghost retirement explicitly
clears its borrowed partition back-reference after source unregister, matching
W3DGhostObjectManager::updateOrphanedObjects; unregister alone does not clear it.

## Validation

Focused command:
`ctest --test-dir <configuration> -R '^original_logical_world_partition' --output-on-failure`.
Normal-host sanitizer execution keeps unsuppressed ASan/UBSan/LSan, leak checking,
halt-on-error and original TIMEOUT60. Every family repeats three times.

| Configuration | Focused4 | Related137 | Full configured build |
| --- | --- | --- | --- |
| GCC | PASS, 8.34s | PASS, 125.09s | PASS |
| GCC ASan/UBSan/LSan | PASS, 34.37s | PASS, 557.03s | PASS |
| Clang ASan/UBSan/LSan | PASS, 19.37s | PASS, 337.72s | PASS |

Exact complete manifests match all3 configurations:

| Admission | Failure prefixes | Exact terminal |
| --- | --- | --- |
| Cold grid/cells/radius backing | [0,31) | 31 |
| Warmed mobile actor attachment | [0,1) | 1 |
| Warmed ghost attachment | [0,1) | 1 |

Discovery owners retire before baselines. Every failure and terminal success has
an immediate corrected retry. Partial grid backing retires exactly; accepted
actor state survives attachment failure; complete contexts and the original
memory manager retire all metadata. Pooled exhaustion is separate from standard
allocation manifests. No timeout, suppression or proprietary-data mutation.

Related regex:
`^original_(object_ownership|team_ownership|player_ownership|ai_pathfinder|waypoint_graph|native_terrain|logical_world|native_water|native_client|diplomacy|statistics|header_paths|map|chunk|runtime|core|enum|source_boundaries)`.
Full build: `cmake --build <configuration> -j2`.

## Retained failures and source findings

- Initial actor fixture compilation lacked the full Player declaration; a test
  inadvertently launched against the previous executable rejected the new family.
  Added the required header; subsequent commands condition tests on build success.
  This was fixture/build sequencing, not runtime behavior evidence.
- First actual actor failure reached COI allocation rollback and stopped in
  removeAllTouchedCells. Both actor and ghost paths published a positive array
  count before new[] succeeded. Reviewed every count/backing/used assignment;
  both now publish count only after initialized backing exists. The registration
  guards withdraw master-list, dirty-list and acquired ghost/candidate publication
  without normal fog retention. Focused4 then passes on frozen source all3.

Durable source details: `docs/original-engine-formats.md`. Local logs retain all
failures/retries under `/tmp/n2-partition-*`; raw private inputs were not used.

## Mandatory remaining outcome

The actual shipped ghost used/free lists, fog snapshot ownership/orphan retirement,
border release/restore, transfer representation and renderer attachment remain
pending. Actor templates here do not accept authored body/contain/AI creation or
full ThingFactory::newObject callbacks. Contact dispatch/whole simulation is not
accepted by direct touched-cell maintenance. Registered walls/model-derived
bridges, candidate-root rollback, real UI/client/control-bar binding, full original
GameEngine startup and compiler-matching GameLogic::update remain mandatory.
Accepted-actor geometry replacement is also pending; malformed geometry tests
exercise unattached candidates, not restoration of an accepted partition/snapshot.
Slice03/N2 remain incomplete. No vendor, asset or recovery-archive changes.
