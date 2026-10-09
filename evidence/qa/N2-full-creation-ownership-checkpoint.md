# N2 full-creation ownership checkpoint

This supports existing N2 plan01/slice03, not complete startup, simulation,
rendering, fog or milestone acceptance. No proprietary input or framework edits.
Batch source parent: `587b12343a51164c592433c6cb033271eedc9f5a`; resumed after
documentation handoff `e5a0fefb`. Source frozen for current validation:

- Canonical626: `97bfe4fa32b00f1e6a8629352c2f03ead5471990fa98ff802f3479a878476677`.
- Toolchain-inclusive633: `3ca742ad1ed229dce339005c9ca2dbc790b052d83b7ee30ccb296163332d095c`.

## Actual behavior exercised

Generated INI uses original InactiveBody and VeterancyGainCreate creators, with
StartingLevel VETERAN. Actual ThingFactory::newObject executes registered Object
construction, source create interfaces, nonempty partition admission and
Object::initObject, including actual GameLogic::sendObjectCreated, source
Drawable construction, reciprocal binding and source lookup identities. An
accepted survivor retains its actor/Drawable/partition identities on failure.
Retirement uses source destroyObject/processDestroyList, not fixture deletion.

Real NativeHeadlessGameClient initializes before logical UI publication, with
source MessageStream, rank, AI, team/player, radar, module, upgrade, terrain and
partition parents. Source setDrawIconUI(FALSE) disables absent physical promotion
feedback; source body/veterancy callbacks still execute. This fixture does not
call the headless frame updater or establish full native root composition.

ThingFactory holds the completed candidate across all later creation/admission
steps. Failure withdraws registration then calls the existing pooled deletion
endpoint while legitimate parents remain live; actor cursor restoration occurs
only if no later actor consumed it. sendObjectCreated owns the new Drawable across
both binding phases and clears only matching candidate pointers without another
acquisition callback before withdrawing its registry and deleting it.

The allocation families use no replacement gameplay or draw modules. A separate
binding family registers explicitly test-only DrawModule/BehaviorModule probes
through original ModuleFactory. They observe each real publication phase and
throw a typed error, then prove complete candidate retirement and corrected
creation with an accepted survivor. They reject physical rendering and provide
no renderer/gameplay acceptance. Probe checks preserve the complete Object
destructor's legitimate null-binding callback instead of suppressing it.

## Validation

Focused command:
`ctest --test-dir <configuration> -R '^original_logical_world_factory-creation' --output-on-failure`.
All three full configured builds, focused3 and related140 pass on the
same frozen source. Required
unsuppressed ASan/UBSan/LSan settings and TIMEOUT60 remain unchanged.

| Configuration | Focused3 | Related140 | Full configured build |
| --- | --- | --- | --- |
| GCC | PASS, 9.51s | PASS, 135.26s | PASS |
| GCC ASan/UBSan/LSan | PASS, 38.46s | PASS, 593.13s | PASS |
| Clang ASan/UBSan/LSan | PASS, 22.06s | PASS, 359.44s | PASS |

Matching all3 full-creation standard-allocation manifests: all prefixes [0,11), exact terminal
11, every prefix/terminal paired with corrected same-root creation and retirement,
three repeated complete contexts. Actual fixed InactiveBody, VeterancyGainCreate
and Drawable pools are separately exhausted and retried. Source cursor advance
is proved before fixture lease restoration. Whole-root standard and pool metadata
retire exactly. All3 independently confirmed this complete manifest and the
separate partial-binding failure/retry family.

Related command regex:
`^original_(object_ownership|team_ownership|player_ownership|ai_pathfinder|waypoint_graph|native_terrain|logical_world|native_water|native_client|diplomacy|statistics|header_paths|map|chunk|runtime|core|enum|source_boundaries)`.
Full build command: `cmake --build <configuration> -j4`. Configured suite: 330 tests.
The related matrix ran only after final focused validation was clean. The full330
configured test suite was not run: it remains the milestone completion gate
rather than a supporting batch claim. No timeout changes or leak suppressions.

## Retained failures/retries

1. First ownership guard compilation used Object's protected deletion endpoint
   and unsigned rather than ObjectID storage. Corrected to public
   friend_deleteInstance and typed ObjectID; no tests ran on that failed build.
2. Both creation families initially failed INI::prepFile: generated creation.ini
   was written after the empty tree was indexed. Refresh generated-only VFS
   through mountReadOnly; no production filesystem fallback.
3. That retry reached real creation, then duplicate-definition rejection on the
   second warm call. Warm complete fresh roots; do not reuse templates after
   their ModuleFactory data owner has retired or disable duplicate validation.
4. Binding-probe compile failures required complete ThingTemplate declarations,
   public constructors after pool macros and a mutable generated definition.
   No tests ran after these failed builds.
5. Binding-probe runtime assumption rejected the legitimate destructor null
   notification and aborted. Separate acquisition from unbind observations;
   original destruction notification remains unchanged. Focused3 then passes.

Local `/tmp/n2-factory-creation-*` logs retain failures and corrected retries;
this report and existing slice03 retain the durable explanation. Source findings:
`docs/original-engine-formats.md`.

## Mandatory remaining outcome

No arbitrary child/script/RNG side-effect rollback is claimed by an actor resource
guard. Full native root journal, logical UI/client/camera-freeze composition,
shipped ghost snapshots/orphans/border/transfer, accepted geometry replacement,
registered walls/model bridges, full GameEngine startup and actual compiler-matching
GameLogic::update remain required. N2/slice03 are incomplete.
