# N2 native TerrainLogic combined map admission checkpoint

Parent: `26da2238885d0bd59dfbe9263baa9c4c4e1e721f`.
Packet: `linux-upstream-v2`; N2 plan01/slice03 remains incomplete.
No upstream dependency, read-only asset or recovery archive changes.

## Delivered behavior

Actual TerrainLogic-derived CPU provider connects original nonempty logical
world callbacks to original pooled waypoint graph and retained full visual-owned
height/blend/lighting. Dimensions/boundaries/active0/logical extrema, filename,
graph and retained map are staged under one admission scope. Prior visual ground
supplies initial waypoint Z; absent ground is zero. Later newMap resnap remains
after a mandatory concrete road/bridge admission hook. Query mode skips retained
visual load, preserving prior visual name/backing while admitting logical state.

Source post-load time selection updates active terrain ambient/diffuse/direction
rows and conditionally calls the mandatory device hook, including query mode.
An explicit late hook executes before original world commit. Rejection restores
original world/side/script/trigger/name/ID publication plus native metadata,
graph, both filenames, retained height/shadow and authored/active lighting rows.
Temporary CPU exchanges do not call output dirty hooks; successful completion
does. Device/root callbacks must guard their own external mutations.

Ground/cliff/LOS/extents dispatch uses retained source CPU behavior. Border
extent queries preserve caller Z. Logical min/max remain cached after deformation.
LOS uses a cached full-grid maximum rather than scanning every cell per query.
Wall queries invoke actual AI/pathfinder; bridge queries use actual source bridge
selection/height, never universal ground replacement. Missing wall prerequisites
reject. These fixtures do not accept nonempty wall/bridge behavior.

The class remains abstract for concrete device-map and road/bridge admission.
Test-only hooks are not production no-op rendering/bridge implementations.
The source version1 native logic snapshot wrapper is present but its complete
snapshot execution is unverified. Native startup/root, registered bridge objects,
model-derived BridgeInfo, nonempty wall/pathfinder, partition/ghost and actual
compiler-matching original simulation remain required, not substituted here.

## Validation

Three new families repeat three complete generated world/provider owners:
functional query/extent/ordering behavior, malformed-prefix corrected retries,
missing blend after complete logical admission, late device/outer-hook rejection,
and complete allocation prefixes[0,259)/exact terminal259 with every immediate
corrected retry. Accepted and replacement light tables, time and shadow differ.
All authored/active rows restore, not merely identical default values. Actual
Waypoint used pool units and existing world pool units supplement standard heap
retirement. Existing ten actual world-pool exhaustion cases remain in the matrix.

Canonical620 SHA256:
`26e0eb246c0c71bc01f153ced28d1fadf1985d46a56b72252f1a2a23d464826e`.
Toolchain-inclusive627 SHA256:
`af4054cb6f1483f03b0779adf70f76ca918246ed7bdee7d99057212a3e23bc1b`.
Existing changed-path/path-NUL-content-hash recipe; docs/evidence/state excluded.
All3 configured builds PASS. Focused3 PASS GCC9.67s,
GCC ASan/UBSan/LSan50.88s, Clang ASan/UBSan/LSan33.72s.
Frozen related83 PASS normal GCC47.48s, GCC ASan/UBSan/LSan231.92s,
Clang ASan/UBSan/LSan149.41s. Full configured296 suite is not run.
No timeout changes, leak suppression, isolated bus or disabled checks.

```sh
cmake --build build/<configuration> -j 3
ctest --test-dir build/<configuration> -R 'original_logical_world_terrain' --output-on-failure
ctest --test-dir build/<configuration> -R 'original_(waypoint_graph|native_terrain|logical_world|native_water|header_paths|map|chunk|runtime|core|enum|source_boundaries)' --output-on-failure
```

Logs: `/tmp/zh-n2-terrain-logic-{gcc,gcc-sanitize,clang-sanitize}-frozen-build.log`,
`-focused.log` and `-related.log`; CTest LastTest.log records exact terminal259
over three whole owners. Initial focused3 normal run passed before extending
fixtures with distinct lighting/shadow; final focused3 above exercises that
stronger frozen behavior. No failed acceptance was reclassified or waived.

Next: mandatory concrete headless/root providers, actual bridge CPU geometry and
registered-object/pathfinder ownership, final newMap/partition/ghost ordering,
then original process startup and compiler-matching simulation. Source bridge
mesh/hierarchy/transform inputs affect width/corners even without rendering.
Function-static GenericBridge template retention and constructor early-exit
ownership require one coupled bridge/object batch. Source references and exact
pending limits: `docs/original-engine-formats.md`.
