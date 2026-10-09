# N2 actual sides/scripts ownership checkpoint

N2 plan01/slice03, packet `linux-upstream-v2`; parent `69ec4282`.
This supersedes the side/script WIP validation status, not its historical record.
It is supporting ownership/format evidence, **not N2 or slice completion**.

## Frozen source and configuration

Canonical605 SHA256:
`db977a56d3fbb8b347cd4b838a7f1b0108b54f325845b0a20964e35e63dc252d`.
Toolchain-inclusive612 SHA256:
`7f2d11760ece12f6bea4166ad3c0dcd05a7c258cbf465406d217f5461a096c01`.
Identity uses sorted original/runtime/cmake paths changed since9e5c0e4f,
path bytes + NUL + binary content SHA256, optionally adding tests/toolchain.
All configurations are Debug; normal GCC has ZH_SANITIZE=OFF, GCC/Clang
sanitizer builds have it ON. Test environments enable leak detection and
ASan/UBSan halt-on-error, without suppressions. All tests execute on the normal
host; no isolated-bus fixture is involved in these original-runtime tests.

All configured builds completed. Focused command, for each build directory:
`ctest --test-dir build/<configuration> -R '^original_sides_scripts_' --output-on-failure`.
Nine families: functional, negative, copies, faults, setters, formats, parameters,
publication and pool-failures.

| Configuration | Focused | Seconds | Affected matrix |
| --- | --- | --- | --- |
| original-core-gcc | 9/9 PASS | 24.44 | 96/96 PASS, 69.78 seconds |
| original-core-gcc-sanitize | 9/9 PASS | 132.85 | 96/96 PASS, 332.73 seconds |
| original-core-clang-sanitize | 9/9 PASS | 82.50 | 96/96 PASS, 211.00 seconds |

Affected command regex:
`original_(sides_scripts|polygon_world|native_terrain|native_client|header_paths|map|chunk|runtime|core|enum|source_boundaries|transfer)`.
Configured suite272; full272 is not run at this supporting checkpoint.

## Direct evidence

- Actual source SidesList, nested script owners and complete ScriptEngine template
  registry; no mock condition/action provider or alternate simulation.
- All side versions1–3, authored build values/defaults, original neutral/default
  team/owner repairs, nested group/direct scripts, ordered AND predicates and
  true/false command branches, actual writer round trips and independent clones.
- Every physical prefix of the generated modern side/script file and long
  Parameter file; raw counts/types, nested condition/action versions, trailing
  condition payload and side version/count rejection preserve accepted owners.
- Seven original surface-condition repairs and all23 listed legacy action input
  forms, including both waypoint-camera arities; actual registry type/order/
  arity and repair defaults. Modern names rematch invalid numeric ordinals;
  unknown names/mismatched arities retire parameters and use source false/no-op.
- Long600-character object suffix migration, all three old capture upgrades,
  source status/kind conversion, and malformed empty-name kind indices.
- Exactly MAX_PLAYER_COUNT pending lists admitted, excess rejected preserving
  accepted pending identity; successful/failed side parsing restores borrowed
  pending trees. Legitimate simultaneous pool capacity is warmed before baselines.
- Strong side/team assignments retain accepted backing on allocation failure.
  Every failure has a corrected same-process retry; three complete source owner
  lifetimes verify heap and nine original pool used-unit baselines, singleton
  withdrawal, and final memory-manager metadata destruction.

## Exact fault manifests

Each positive count retains failures `[0,count)` and exact terminal `count`,
with every failure/retry pair; zero means the measured warmed operation performs
no standard allocation, not that pooled acquisition cannot fail.

| Operation | Standard allocation count |
| --- | --- |
| Modern side parse, 2/20 builders per side | 295 / 619 |
| Legacy side parse, 2/20 builders per side | 291 / 615 |
| Script deep / qualified copy | 1 / 30 |
| Side copy / accepted-target assignment | 1 / 1 |
| Linked build copy | 0 |
| Team copy / accepted-target assignment | 1 / 1 |
| Typed condition/action constructor and both setters | 0 each |
| Complete legacy/name/arity repair scene | 634 |
| Long object-prefix Parameter migration | 8 |

The separate pool-failures family exhausts each of eight actual source owner
pools using documented non-overflow fixture pool configuration. Every parse
failure preserves accepted tree/build identity, group cursor and all used-unit/
heap counts; releasing held blocks permits the corrected retry. This supplements
standard allocation interposition, without adding production fault hooks.

## Diagnosis and limits

The WIP copy/setter failures were a fixture's unjustified nonzero-heap census
requirement. The parse terminal drift was a fixture state mismatch: advancing
group IDs changes default-name string allocation demand. An outer fixture
IDTransaction now restores the same starting cursor for each measured operation;
successful parse still proves both original group-ID increments before restore.
Production sequencing was not changed to satisfy those failures.

Additional production corrections remove an out-of-domain enum sentinel cast
and bound empty KIND_OF_PARAM's file index before table access. Prior ownership
changes from69ec4282 are now covered by the expanded source families. Verified
wire/ownership/root phase findings are durable in docs/original-engine-formats.md.

All23 legacy action input forms are parse/ownership evidence, not executed action
semantics. No retail audit or asset mutation occurred. Assets, dependencies and
recovery storage remain untouched. Other legacy Parameter kind-name quirks are
not redesigned. Enclosing-world object/Dict/weather/name/terrain publication,
waypoint initial/final provider phases, native TerrainLogic/ghost/root and actual
compiler-matching original simulation remain pending. No rendering parity,
whole-game startup, gameplay or completed milestone claim follows from this file.

The zero standard-allocation constructor/setter terminals are not proof of
mid-construction pooled failure coverage. The pool-exhaustion family currently
exercises full parse paths; explicit partial pooled prefixes for typed setters
and factories remain to be covered alongside the enclosing runtime owner tests.
