# N2 actual Drawable current-model queries

Parent `51fc1495fd5dfce1f6c5fe3fa9227075006baf5d`; same N2 plan01/slice03.
Supporting current-consumer/ownership checkpoint, not concrete providers,
whole native factory, slice03 completion or N2 acceptance.

NativeModelDrawCurrentQueries connects the actual registered Drawable's current
bone consumers to separately owned native model/clip/pose/world state. Concrete
presentation, bounding-info and animation producers remain abstract. The fixture
supplies explicit manual frames, not a substitute renderer or accepted animation
clock. Pristine selected-condition queries remain independent.

## Behavior and ownership

- Actual ObjectDrawInterface and Drawable world-space/model-space consumers;
  distinct absent-model, index0 and missing-output contracts.
- Original orthogonal inverse and axes-only Scale; instance/tree scale, rotation,
  two-module order, case-insensitive bones, start0 and numbered01..99 termination.
- Generated71-pivot cohort proves actual128 outputs with64 per module, untouched
  uncounted tail and first-gap termination. Wide reads measure zero standard
  allocation demand. Explicit-frame updates plus actual reads likewise measure0.
- Main-thread parent identity checks; worker and logic-phase live reads reject
  before output. Model replacement allows legitimate logic-originated condition
  notification, changing client-owned state without querying it into logic.
- Complete asset-cache/model/clip/pose candidate rollback; malformed/nonfinite
  frame/root/scale, missing model, wrong animation hierarchy and late overflow.
  Native-only Drawable preflight preserves all output on second-module overflow;
  mixed legacy consumers are not double-called or accepted by this fixture.
- Three complete owner lifetimes with original factory/template/Drawable/client,
  namespace and pools. Accepted identity, standard backing and pooled-key used
  units survive every rejected replacement; production cursor rollback is proven
  before outer fixture rollback. Constructor acquisition remains separately tested.

Coupled attachment regressions preserve source positive-stale LOD fallback0,
reject encoded negative LOD indices, omit invalid aggregate nodes after child
resolution, and retain strict proxy/skin checks. Model dependencies still fail
explicitly when unavailable; this is not acceptance of all original prototypes.

Source anchors: `GameEngineDevice/Source/W3DDevice/GameClient/Drawable/Draw/
W3DModelDraw.cpp:2672` (three current accessors),
`Libraries/Source/WWVegas/WWMath/matrix3d.cpp:570` (orthogonal inverse),
`Libraries/Source/WWVegas/WWMath/matrix3d.h:975` (axes-only Scale),
`Libraries/Source/WWVegas/WW3D2/hlod.cpp:2562` and `:3519`
(aggregate admission and stale LOD clamp). Paths are relative to GeneralsMD/Code.

## Exact allocation manifests

Current replacement cold terminal173; populated Static.Mesh replacement176.
Every ordinal before each terminal rejects and gets a successful corrected retry;
the terminal ordinal succeeds. Each full sweep is repeated three times.
Retained original projectile cold245/seeded0, wide cold247/seeded2;
Drawable construction standard1 and module-pool2. Zero standard demand is not a
pooled-exhaustion exemption. These terminals are discovered, not fixed thresholds.

## Frozen validation

Canonical712 files SHA256
`2d31f6786bbe723b9dd51c8fd28238895b2223792ab6a7286552cca296cfa7a2`;
toolchain-inclusive719 SHA256
`80aad1e114e50e4569eaf0139ce9fdc58c6e664a116d9a417f91aaecbb5a5732`.
Hash procedure matches N2-drawable-queries.md, relative to `9e5c0e4f`.

Focused GCC25 PASS28.71s. Frozen related98 PASS in all three configurations:
GCC34.85s, GCC ASan/UBSan/LSan155.79s, Clang ASan/UBSan/LSan96.25s.
All three processes exited0; all191 allocation-manifest lines match exactly
across configurations, including every failure/retry and three repetitions.
No live jobs remain. Frozen source identity was rechecked before commit.
Configured454 full suite is unrun. No timeout or coverage relaxation.

Builds: build/original-core-{gcc,gcc-sanitize,clang-sanitize}. Build the same
related targets as N2-draw-provider-data.md; verbose CTest selector:

```text
^original_(draw_provider_data_|drawable_queries_|w3d_(draw_data|assets|animation|document)_|data_|core_|header_paths$|graph_headers_|module_data_|template_backing_|runtime_(names|floating-point))
```

Logs: `/tmp/n2-current-query-<configuration>-frozen-{build,related}.log`.
All ASan/UBSan/LSan checks enabled without suppressions. Independent families
retain their60s timeout and every failure/retry pair.

Initial linkage failed on the unported out-of-line Matrix3D math object. The
game-owned helper now implements that source transpose/-R'T algorithm with shared
inline math, without importing its platform assembly or modifying a dependency.
The scaled fixture oracle was checked against axes-only source Scale semantics.
Earlier initial/focused/math-reviewed/cohort/phase-reviewed logs retain attempts.
Final phase refinement followed actual condition-notification callers rather
than forbidding legitimate logic-originated current-model publication.

Pending: source sync-time frame clock, animation selection/client RNG/transitions;
all-LOD base bounds, special bounding primitive; concrete19 source providers and
callbacks; condition transfer, remaining materials/prototypes; full native root
startup and loaded deterministic simulation. No GPU/retail execution or parity
claim. No retail assets, adopted libraries or recovery content changed.
