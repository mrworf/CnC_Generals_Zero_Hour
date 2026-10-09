# N2 native terrain height/query checkpoint

Supporting progress in existing plan01/slice03; **N2 and slice03 are incomplete**.
Parent: `b984fff1fb0f804014109927c30f5bcb4754f766`. Worktree was clean.
Commit checkpoint title: `delivery: extract original terrain height queries`.
No retail files, dependency sources, recovery content or product authority changed.

## Delivered and source qualification

Game-owned NativeTerrainHeightMap extracts WorldHeightMap height-chunk backing
and BaseHeightMap simulation queries using the actual DataChunkInput, original
grid constants and source triangle/normal/LOS algorithms. It stages complete
backing before publication, rejects malformed or incomplete height candidates,
preserves accepted backing on failure, and permits corrected same-owner retry.
Boundary backing grows only after each actual pair is read; height backing grows
after bounded4KiB reads, not eagerly from an unproved chunk envelope. Large
declared boundary/height lengths on physically short input reject without large
candidate allocations. The final coupled acquisition audit strengthened both
parallel paths together after the initial source cohort had passed.
Nonfinite/unrepresentable query coordinates reject before integer conversion.
See `docs/original-engine-formats.md` for source locations and verified contracts.

The height-only entry point deliberately does not claim full map/world load.
Its v1 HeightChunkBacking purpose is the original full-reader intermediate:
BlendTileData v1 subsequently updates dimensions/dataSize. LogicalMetadata
retains the original size-only contract. This coupled source sequence disproves
the preliminary suspicion that the dimension difference alone was an engine bug.
The editor's dormant zero boundaries are preserved, not rejected as invalid
playable boundaries during unrelated height parsing.

Generated functional cases cover nonplanar upper/lower/diagonal height and normal
goldens (including the source unusual stencil), flat normal signed-zero behavior,
explicit distinct display clipping, all dominant-axis LOS directions, source
endpoint asymmetry/tolerance/offmap/same-cell rules, one-sample grids and all four
height versions. Negative cases cover every truncated wire prefix, missing/
duplicate height chunks, unsupported versions, signed dimensions/border/count
failures, length mismatch/trailing bytes, hostile floating coordinates and raw
XY rejection without linear aliasing, including forged1GiB height and large
boundary envelopes with tiny physical inputs. Acquisition censuses are6 (7x7)
and10 (96x96 multi-block): every preceding ordinal fails and receives a corrected
same-owner retry; each exact terminal completes without triggering an injected
fault. Failed candidates preserve both accepted backing identities; corrected
retry publishes genuinely different height bytes, then restores original data
before the next failure pair. No addresses are printed or retained as evidence.
All three whole-owner repeats retire standard allocations;
whole original memory-manager destruction also returns to the initial baseline.

## Frozen validation

| Configuration | All configured targets | Focused terrain3 | Related runtime145 |
| --- | --- | --- | --- |
| GCC | PASS | 3/3, 0.10s | 145/145, 66.64s |
| GCC ASan/UBSan/LSan | PASS | 3/3, 0.79s | 145/145, 273.36s |
| Clang ASan/UBSan/LSan | PASS | 3/3, 0.53s | 145/145, 195.33s |

Sanitizer execution used the normal host, ASAN_OPTIONS=detect_leaks=1:halt_on_error=1
and UBSAN_OPTIONS=halt_on_error=1:print_stacktrace=1; no suppression or reduced
ownership coverage. This generated-only cohort does not read retail content.
Each individual test retains its existing60s limit; aggregate suite times are
not individual test timeouts. No failures occurred on the frozen source.

Commands:

```sh
cmake --build build/original-core-gcc -j 4
cmake --build build/original-core-gcc-sanitize -j 4
cmake --build build/original-core-clang-sanitize -j 4
ctest --test-dir <build> -R '^original_native_terrain_' --output-on-failure
ctest --test-dir <build> -R 'original_(native_terrain|native_client|lod|animation|native_warning|header_paths|diplomacy|map|chunk|game_info|lexicon|function_registry|startup|service|configuration|runtime|core|module_data|template_backing|enum|source_boundaries)' --output-on-failure
```

Local terminal logs: `/tmp/zh-native-terrain-{gcc,gccsan,clangsan}-final-build.log`,
`...-focused.log`, `...-related.log`. The configured suite now contains255 tests;
the full255 matrix was not run and is not accepted from this related subset.
Generated count/status-only sanitizer captures also confirm6/10 acquisition
censuses in `/tmp/zh-native-terrain-{gccsan,clangsan}-census.log`.

Frozen cumulative source599 SHA256:
`c1f67920e095f8250aff4d3f20da4bf81e636d7350bdfe573331b76d629150cb`.
Toolchain-inclusive606 SHA256:
`07e84d571f370bed7ed744fda32c2809364a336825258e4c15e4965ea4565f5f`.
Recipe is the existing cumulative path/content snapshot since9e5c0e4f, including
nonignored new source: sorted paths, path bytes/NUL/binary SHA256(file bytes).
Canonical scope is CMakeLists.txt, GeneralsMD/Code, tests/original and cmake;
inclusive also includes tests/toolchain. Planning/source commits remain ancestors;
the authoritative specification hash remains0316702c3fb75979f3ef5cdafb3adcf3a4c1bfa3dc84e94f883aceb6ce49bf90.

## Remaining gates and resume

Continue the same N2 plan01/slice03: complete native BlendTileData/world candidate
and actual TerrainLogic integration, including authored cliff bits and legacy
blend transitions, objects/world dictionary/triggers/sides, bridges/walls, waypoint
load order and coherent accepted-world retirement. Preserve shipped ghost/
partition fog lifecycle, real camera timing, audio readiness, particle/radar
owners, actual native GameEngine startup and compiler-matching original world/
simulation. Root link closure and this query cohort are not execution acceptance.
N3–N7 remain dependency-gated; no new milestone or replacement plan was created.
