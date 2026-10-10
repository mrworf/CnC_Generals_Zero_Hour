# N2 actual Drawable logic-query checkpoint

Supporting checkpoint under `linux-upstream-v2`, N2 plan01/slice03. This is
not slice completion, renderer acceptance, or N2 acceptance.

## Reachability and ownership

Generated INI passes through the original ModuleFactory data parser,
ThingTemplate ModuleInfo, NativeHeadlessGameClient DrawablesOnly construction,
actual Drawable module walk, and NativeModelDrawQueries pristine/projectile
consumers. Two registered generated modules exercise fall-through and recursive
attachment lookup. The abstract production adapter does not supply presentation
or current-animation callbacks; the fixture concrete subclass rejects those
services. The fixture factory registers one controlled provider, not all native
draw providers or the complete original factory.

Original ignored-condition selection, first/final pose, scale, contiguous bone
sweeps, barrel fallback, art-angle rotation, and turret attachment offsets are
preserved. Missing pristine transforms retain the original uncounted fallback
without publishing a position. The shared production logic-phase guard, FPU
setup, and construction-thread admission are exercised, but this phase fixture
does not advance actual simulation frames.

Root dispatch stages caller output and owns namespace, asset-cache, and all
participating module-cache rollback. Cold and already-accepted cohorts preserve
cache identities, standard live allocations and pool units on every rejection;
every prefix has a corrected retry. Production namespace/Drawable cursors are
checked before fixture rollback. Whole generated cohorts retire three times.

## Frozen validation

Canonical source fingerprint: 683 files,
`b5eb9f9f118c0d8e243de56975063e8ec91bfa418bf96d74dab2371fcc75505b`.
Including toolchain fixtures: 690 files,
`847673b29f8a3cd267b6eb1e364bdd0f2744ac60385269fe083f73323d9e31ae`.
Fingerprint uses sorted changed/untracked source paths relative to `9e5c0e4f`,
each path followed by NUL and its binary SHA256. Documentation/state are excluded.

Seven new families: functional, negative, faults, seeded_faults, wide_faults,
wide_seeded_faults, constructor_faults. Frozen related coverage includes native
client, actual actor/logical-world ownership, W3D definitions/assets/motion,
core/data/header/module/template and name/FPU families (136 tests per build).
The configured suite contains 441 tests; full-suite execution remains pending.
Original 60-second query-family limits and all ASan/UBSan/LSan checks are intact.

Results: GCC 136/136 PASS168.54s; GCC ASan/UBSan/LSan 136/136 PASS713.18s;
Clang ASan/UBSan/LSan 136/136 PASS427.49s. All processes terminal0; no live
validation jobs remain. This records the frozen checkpoint, not N2 acceptance.

Exact query standard-allocation terminals are projectile cold245/seeded0 and
wide cold247/seeded2. Actual Drawable construction terminal1 is distinct from
the native module pool's acquisition prefixes [0,2), successful terminal2.
Zero warmed projectile standard demand is measured, not exempted from checks.

Reproducible build directories: `build/original-core-gcc`,
`build/original-core-gcc-sanitize`, `build/original-core-clang-sanitize`.
Build the original drawable-query/native-client/object-ownership/logical-world,
W3D draw-data/assets/animation/document, data/core/graph/module/template/runtime
fixture targets, then run verbose CTest with:

```text
^original_(drawable_queries_|native_client_|object_ownership_|logical_world_|w3d_(draw_data|assets|animation|document)_|data_|core_|header_paths$|graph_headers_|module_data_|template_backing_|runtime_(names|floating-point))
```

Unabridged local build/test transcripts:
`/tmp/n2-drawable-query-<configuration>-frozen-{build,related}.log`, where
configuration is gcc, gcc-sanitize, or clang-sanitize. These disposable logs are
not a required repository/build dependency.

## Investigation history and limits

Initial fixture compilation exposed ThingTemplate's protected pooled allocation
protocol and mixed inferred scalar types. The generated definition was moved to
stack optional ownership. Initial execution then reported zero pose coordinates:
an unparsed ThingTemplate defaults asset scale to zero. The fixture now parses
the original Scale field as 1; production semantics were not changed to satisfy
that broken fixture. Corrected focused seven-family GCC run passed16.22s before
the frozen matrix. No test was removed, suppressed, or given a longer timeout.

The load-game admission branch, actor-bound missing-transform fallback and
recursive replacement by a different explicit condition are source-reviewed,
not runtime-proven here. Current-client pose/animation, rendered callbacks,
complete derived19 registrations, condition transfer, remaining materials and
prototypes, full native GameEngine startup and loaded deterministic simulation
remain N2 work. Mixed legacy/custom native provider parity is not established.
No retail asset, GPU output, framework/vendor source or recovery archive was
modified or used for this generated checkpoint.
