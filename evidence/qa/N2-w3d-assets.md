# N2 W3D asset graph supporting checkpoint

Parent: `bd5b835f`. Existing N2 plan01/slice03 remains in progress.
This checkpoint is independently testable asset admission, not N2 acceptance.

`NativeW3DAssets` reads generated original FileSystem inputs into immutable,
owned mesh/HLOD/null model graphs and decoded animation handles. Requests publish
complete offside cache state only after dependencies resolve. Tests exercise
localized/base/explicit user-storage routing, original BIG files, authored root
order collisions, nested HLOD levels and aggregates, metadata-only proxies,
skin-bone relationships, named/default hierarchies, retained handles after cache
and VFS retirement, and rejection/recovery for malformed/missing/cyclic inputs.
Exact hierarchy identity prevents mixing model/animation cache generations.

Frozen source: canonical671 SHA256
`a56981c8634df5a88322aa503d07d1499cfd501978b98394bbcc175379d92ebb`;
toolchain-inclusive678 SHA256
`411b526da842d6bb2408b5d6aa85aa7538132e0c77b5face580bb5b663209ae9`.
Builds cover six original fixture/audit targets per configuration and related42
tests (assets, animation, document, data, core, header paths).

| Configuration | Related tests | Terminal |
| --- | --- | --- |
| GCC | 42/42 PASS, 3.06s | 0 |
| GCC ASan/UBSan/LSan, normal host | 42/42 PASS, 13.32s | 0 |
| Clang ASan/UBSan/LSan, normal host | 42/42 PASS, 9.81s | 0 |

All leak/undefined-behavior checks remain enabled without suppressions. Initial
sandbox sanitizer executions returned8 because LeakSanitizer process inspection
was prohibited, including unchanged core tests. Identical host reruns passed;
no source changes, system ptrace settings, coverage reductions or timeout changes
were made. Earlier fixture compile failures (private pool getter and mixed auto
declarations) were repaired using the existing public pool census and explicit
declarations; production behavior was not changed to satisfy them.

Exact standard-allocation terminals: cold model144, cold animation47, seeded
model143, seeded animation48. Each family repeats three times, retaining every
failed prefix and successful same-owner retry through the exact terminal.
Seeded sweeps start with the explicit NULL cache entry; they do not establish
fault coverage for every possible populated-cache shape. Failure preserves
accepted counts/identity and live standard allocation count; descriptors and
pooled NativeDataFile used units retire exactly. Cache hits and clear allocate
nothing; whole-owner retirement is checked separately.

Commands: build targets `original_w3d_assets_fixture`,
`original_w3d_animation_fixture`, `original_w3d_document_fixture`,
`original_data_fixture`, `original_data_audit`, `original_core_fixture`;
CTest selector `^original_(w3d_(assets|animation|document)_|data_|core_|header_paths$)`
with `-V` in each `build/original-core-{gcc,gcc-sanitize,clang-sanitize}`.
Local logs: `/tmp/n2-w3d-assets-<configuration>-frozen-build.log`, GCC
`-frozen-related.log`, sanitizer `-host-related.log`; failed sandbox evidence is
retained in sanitizer `-frozen-related.log`, and fixture compile attempts in
GCC `-iteration-build.log` / `-cohort-build.log`.

Configured suite423 was not run in full. No GPU/retail acceptance is claimed.
Materials, other original prototype types, morph/combo channels, incidental eager
clip-loading behavior, condition binding and original draw consumers remain
pending, followed by full native factory/startup and loaded simulation. Upstream
libraries and retail inputs were not modified.
