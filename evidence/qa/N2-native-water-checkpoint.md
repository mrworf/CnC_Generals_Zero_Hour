# N2 CPU water-provider checkpoint

Parent: `f300fca5df1fe3763bfd547fbaa2969e52f9e854`.
Packet: `linux-upstream-v2`; existing N2 plan01/slice03 remains incomplete.
This transaction owns the six pending source/plan/documentation paths found at
the user's worktree-cleanup request, plus this evidence and delivery state.
No assets, framework sources, recovery archive or unrelated files are changed.

## Behavior and scope

NativeWaterGrid retains original CPU layout, world/vertex query behavior,
attenuation, transforms, velocity, frame updates and scalar snapshot encoding.
It uses actual GlobalData and Snapshot/Xfer dispatch. The detailed source
contract and intentional original-bug repairs are recorded in
`docs/original-engine-formats.md`, Source CPU vertex-water contract.

Five independent test families cover functional behavior, motion, invalid
inputs, snapshots and complete enable/resize/load allocation fault manifests.
Each repeats three complete owners and verifies whole-manager retirement.
Snapshot tests reject every incomplete prefix with an immediate corrected retry;
the 3x3 grid's 36 cells serialize to exactly369 bytes, without struct padding.
Fault tests derive each exact success terminal and retain every failure/retry:
enable prefixes[0,1), resize[0,1), load[0,2), with terminals1/1/2,
each repeated across three owners.
All fixtures are generated; no retail audit or GPU acceptance is claimed.

This is independently testable CPU-provider support, not complete native
TerrainVisual/TerrainLogic integration, simulation startup or audiovisual parity.
Waypoints, providers/layers, partition/ghost semantics, presentation metadata and
native root/simulation remain required by the existing slice.

## Frozen source identity

Canonical611 source SHA256:
`3ef47336350e06c7cdb98de96cb9b48c00a0f975cc740857f8854f65c6146b10`.
Toolchain-inclusive618 SHA256:
`047293bf72980103b8758687ce461722757cc66db4a08294b1718e2086d26247`.
Same recipe as the logical-world checkpoint: changed paths since9e5c0e4f plus
nonignored untracked paths, sorted path/NUL/binary content SHA256 stream;
canonical CMakeLists.txt and GeneralsMD/Code/, tests/original/, cmake/ prefixes;
inclusive additionally tests/toolchain/. Docs/evidence/state are excluded.

## Validation

All configured builds pass under GCC and GCC/Clang ASan/UBSan/LSan. Leak
checks remain enabled without suppressions or an isolated-bus fixture. Final
GCC focused5 passes (0.36 seconds). GCC related106 passes (80.42 seconds).
Clang sanitizer related106 passes (242.44 seconds); GCC sanitizer related106
passes (386.42 seconds). All five water families are included in each matrix,
with exact enable/resize/load terminals1/1/2 in every configuration. The full
configured282-test suite has not run; neither slice03 nor N2 is complete.

```sh
cmake --build build/<configuration> -j 3
ctest --test-dir build/<configuration> -R '^original_native_water_' --output-on-failure
ctest --test-dir build/<configuration> -R 'original_(native_water|logical_world|sides_scripts|polygon_world|native_terrain|native_client|header_paths|map|chunk|runtime|core|enum|source_boundaries|transfer)' --output-on-failure
```

Build logs: `/tmp/zh-water-{gcc,gcc-sanitize,clang-sanitize}-final-build.log`.
Related logs: `/tmp/zh-water-{gcc,gcc-sanitize,clang-sanitize}-related.log`.
CTest LastTest.log captures exact fault terminals. These local logs are evidence
locations, not runtime/build dependencies.

Retained iteration history: initial production compile lacked the original
Terrain constant and SaveCode declarations; direct source headers fixed it.
Initial focused5 failed because the fixture constructed GlobalData without its
required FileSystem singleton. Installing a generated fixture filesystem fixed
that prerequisite; four families then passed. The remaining rotation oracle
used a boundary whose source cosf roundoff makes it strictly outside. The
corrected interior point preserves source bounds, without adding an epsilon.
Source review also preserves representable negative-fraction byte truncation.
Logs include `/tmp/zh-water-gcc-focused.log`, `-focused-retry.log` and the
corresponding build/retry logs. No production behavior was weakened to satisfy
the two fixture errors. Final source was frozen before related validation.
