# N2 actual default-team/prototype ownership checkpoint

Supporting evidence in existing N2 plan01/slice03, not milestone acceptance.
Parent `872cdd97826ef7583f6b565d4bc2c3111b63f6ee`, clean transaction start.
Commit boundary: `delivery: preserve original team admission ownership`.

## Frozen scope and source fidelity

Canonical625 SHA256:
`21b32f18dc8f411ed0b69aefc1b26b18dac406f09ebe0354e73e760a4e94a303`.
Toolchain-inclusive632 SHA256:
`0da9af4a68ff3b6f296ac69f7f6b2e12c0b92d431dc4d09ea4deb9d090864a8d`.
Same baseline/path-NUL-binary-file-digest recipe as previous checkpoints;
documentation/evidence/state excluded. Configured312; full312 not run.
Active specification and packet unchanged.

Actual retained TeamFactory, TeamPrototype and Team execute with actual player,
rank, name and ScriptEngine parents. Neutral `setDefaultTeam` finds the original
`team` name and activates its real instance. Singleton reuse does not allocate a
second instance or consume another ID. Non-singleton creation retains separate
active/inactive instances and original intrusive prepend order. Actual factory
deletion notifications withdraw player/team relationship entries while the team
identity remains live; complete retirement then removes intrusive membership.
The source clear -> player init phase withdraws the default-team pointer. No
retired pointer is dereferenced to test removal.

Factory map swap preserves the original entire-registry withdrawal before
callbacks, without its former fallible map copy. Prototype dual registration
unwinds factory membership if player-list insertion fails. Team constructor
rollback retires acquired relation maps and intrusive membership, not complete
team callbacks on a partial constructor. Factory failure restores both cursors
and retires completed candidates under legitimate globals. Missing parents/data,
duplicate/foreign prototypes and missing names reject. Fixed-width unsigned IDs
remain intact; overflow guards are source-reviewed, not a claimed executed
maximum-ID fixture. Authored script creation actions and real actors are pending.

## Validation

All3 configured full builds PASS: `cmake --build <configuration> -j2`.
Focused command:
`ctest --test-dir <configuration> -R '^original_team_ownership_' --output-on-failure`.
Normal-host execution; GCC/Clang retain unsuppressed ASan/UBSan/LSan checks,
halt-on-error options and original TIMEOUT60.

| Configuration | Focused4 | Related122 |
| --- | --- | --- |
| GCC | PASS, 0.81s | PASS, 81.96s |
| GCC ASan/UBSan/LSan | PASS, 3.33s | PASS, 381.94s |
| Clang ASan/UBSan/LSan | PASS, 2.61s | PASS, 239.80s |

Related regex:
`^original_(team_ownership|player_ownership|ai_pathfinder|waypoint_graph|native_terrain|logical_world|native_water|native_client|diplomacy|statistics|header_paths|map|chunk|runtime|core|enum|source_boundaries)`.

Initial normal build failed only because the new fixture omitted Dict's concrete
header; the header was added. First normal focused4 run then passed3/failed1:
functional whole-factory heap equality included newly warmed bucket capacity
owned by the still-live player's relationship unordered_map. Source inspection
confirmed erase retires the node but retains buckets until player retirement.
The corrected fixture warms actual nonempty relationships/clear/default-team
behavior before the factory baseline; every full player/context/memory-manager
retirement still must return exactly to the process baseline. Corrected focused4
passes all3. No production workaround, suppression or relaxed lifetime assertion.

All3 configurations report the same warmed allocation manifests:

| Operation | Every failure prefix | Exact terminal |
| --- | --- | --- |
| Singleton prototype + member dual registration | [0,39) | 39 |
| Inactive instance on existing non-singleton prototype | [0,5) | 5 |
| Active instance on existing non-singleton prototype | [0,5) | 5 |

Discovery owners retire before fault/resource baselines. Every prefix preserves
the accepted prototype, singleton, neutral default-team identity and all heap/
pooled units; every failure is followed by corrected same-factory retry. Failed
candidate retries produce ID2; successful terminal creation demonstrably produces
ID2 before retirement, then retry produces ID3. Each fixture factory is destroyed
in full, rather than restoring private cursor fields. Player-owned warmed
capacity remains permitted only until its enclosing owner retires exactly.

Actual fixed-capacity exhaustion separately covers PlayerRelationMapPool,
TeamRelationMapPool, TeamPrototypePool and TeamPool, including cleanup of prior
acquisitions and prototype/player membership, then corrected ID2 retry. Every
family repeats across three factories; complete context and memory manager
retirement returns to baseline. Armed allocation ordinal0 observes zero
standard acquisitions during full factory clear.

## Pending outcome

Real registered Object/Drawable/partition/radar/UI participants, nonempty wall
footprints and model-derived bridges, authored creation actions and rollback of
their gameplay side effects, full root publication/retry/startup and original
compiler-matching simulation remain mandatory. Team ownership does not establish
those outcomes. See `docs/original-engine-formats.md` for durable source anchors.
No retail assets, recovery storage or upstream dependency sources were changed.
