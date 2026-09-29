# M22 slice 08T0R3: bounded camera-startup capacity

Status: accepted on frozen final source; exact implementation commit is this revision.
Transaction parent and plan checkpoint: `6e7d4c04393ed05f3d503b8df07e8ac62e53a8a7`.
Implementation subject: `delivery: M22 08T0R3 admit bounded camera startup capacity`.

## Bounded source and device contract

The public HeightMap limit is32x32 tiles,1024 total, with draw dimensions at
most1025 and32-cell tiles. XYZDUV2 has32-byte vertices and four vertices per
cell, giving131072 bytes per tile and128MiB total terrain payload. The typed
camera checkpoint ceiling is derived from actual compiled owner types:
`1024*(2*tile_bytes+sizeof(CameraStartupAttempt::Tile))+sizeof(CameraStartupAttempt)`.
Linux64 Tile72/Attempt2080 yield268,511,264 bytes;255 needs66,867,160 bytes,
whereas256 needs67,129,376 and cannot fit the former64MiB ceiling. Count,
dimensions and tile-grid consistency reject before backing-array access.

Only the shipping camera attempt opts into the named `camera_startup` idle
transaction profile. Its704MiB ceiling is64MiB plus five128MiB terrain payloads:
bgfx's double checkpoint of both retained shadow and written arrays accounts for
four payloads, and a full update accounts for the fifth. All auxiliary baseline,
reservation and upload charges must still fit; unused terrain allowance never
exempts them. Recording preserves its own existing actual charge formula.
Ordinary/default/frame transactions and every individual upload retain64MiB.
Unknown profiles, camera+frame, malformed bounds and cap+1 reject without
checkpoint mutation or poisoning an existing owner. No native bgfx patch changes.

The project transaction descriptor consumes prior tail padding: GCC and Clang
syntax-only ABI assertions prove prior size/alignment/member offsets unchanged,
standard layout and default ordinary behavior. Original class fields, virtuals,
serialization and Windows success paths are untouched. All renderer consumers
are rebuilt against the same descriptor. Source camera/provider/ray math,
publication tokens, resource identities and existing rollback remain unchanged.

## Generated controls and classified fixture corrections

The existing full-probe/Python owner supplies real255/256/1024 maps, including
partial edge tiles. Every map proves first full update, all-tile CPU/backup
content (and Recording native bytes), exact full-upload sequence, no-update
identity with zero uploads, representative early/middle/late source and device
faults, unchanged accepted state and clean retry. Malformed1025/negative/grid/
dimension states reject before backing access. Existing172 typed small-map fault
boundaries and16 physical pixel-fault boundaries remain unchanged. Two internal
source/device generations run in each of two fresh process generations.

Recording/bgfx CPU controls cover profile maxima/bound+1, all malformed
descriptor families, nested owner preservation, exact retained-baseline plus
reservation/upload budgets and ordinary per-upload limits. Native resource
controls admit the exact combined budget, reject its adjacent lower/upper
boundary before native touch, preserve pixels/resources, and retry deterministically.
Native large-map camera tests assert no frame advancement and zero staged frame
commands. The existing small-map physical source-camera proof remains intact.

An initial generated upload-trace comparison incorrectly expected the initial
full-update sequence to disappear on retry; source order appends the same full
sequence. The fixture now checks that exact suffix while requiring every other
accepted state identity. Production behavior was unchanged.

An initial allocation baseline preceded the real logical map load. Read-only
owner localization found source-faithful retained LinuxTerrainLogic height-vector
capacity and SidesList default-player string storage, already present before
Edge/camera construction. The approved fixture baseline is after real logical
load and before device/camera construction. Live source/device ownership must be
zero; after device destruction and logical reset, exact initialized allocation
equality is required. Device diagnostic/tombstone storage follows the accepted
monotonic contract and retires at device teardown; no camera/tile/device live
ownership residual is waived.

An optional large full-display continuation passed camera preparation then hit
the public W3DShroud ordinary64MiB upload admission. That exploratory continuation
was removed from the camera-only control; no shroud/default/frame limit changes.
Large native tests retain the exact idle/no-frame/no-command assertions, and the
small-map physical pixel proof is unchanged. The downstream fixed category is
`original shroud upload transaction rejected`, an active08 boundary, not a
max-map full-display acceptance claim.

## Measured wrapper-only workload correction

Diagnostic drivers used the existing run timeout parameter, unchanged executable,
generated inputs, workload and assertions under isolated load. Completed child
processes had exact camera/capacity/resource markers and zero sanitizer/Vulkan
validation categories. These temporary drivers remain uncommitted and are not
final acceptance evidence.

| Configuration | CPU child seconds | Physical child seconds |
|---|---|---|
| GCC native |44.17 total registered control; individual times not measured |44.924 /44.872 |
| Clang native |22.497 /22.558 |45.633 /45.667 |
| GCC sanitizer |82.209 /82.402 |184.872 /185.951 |
| Clang sanitizer |77.457 /78.433 |177.737 /176.725 |

Only the camera wrapper mission-child override changes30→250s, giving34.44%
headroom over185.951s. Shared/bootstrap30s, TimeoutExpired failure, both generation
loops and all assertions/workload remain unchanged. Original30s physical timeout
and interrupted120s diagnostic are superseded non-evidence. A temporary driver's
zero-count validation-category label triggered the outer substring classifier;
raw child categories and assertions were clean, and subsequent safe-label runs
completed cleanly. Final acceptance uses the registered250s wrapper, not a driver.

## Frozen source and acceptance provenance

All fourteen preserved active08/renderer paths remain unchanged. The shared
ledger changes only the R3 camera-inc row; reversing that exact row recovers the
entire prior ledger hash. The unrelated `tests/renderer/test_bgfx_device.cpp`
diagnostic and all active08 source/test/plan/ledger hunks remain unstaged.
Generated gates use no original corpus input or images. The exact commands and
17-control selection are in the [plan](../../plans/cnc-generals-zero-hour/milestone_22_plan_01_slice_08t0r3.md).

| Frozen source/test | SHA256 |
|---|---|
| include/zh/renderer/contract.h |2d40ee73f19d335fd594f69d07c5d5c47a57620300e22c9bc308fade06b5ae21 |
| src/original_runtime/w3d_camera_startup_cpu.inc |d1b39ab1923c4622f7bf48c54f79707ff8311a5302a9f50894c0115c89526c22 |
| src/renderer/bgfx_device.cpp |e62f66bd875a1bb49ccbc465da66ae3ce850c7bbe6bb35d54c6529aa2ed71574 |
| src/renderer/bgfx_transaction_state.h |9c8ee5724014aa18d5a371f72e8f838cb865b955e27cfdeb1cd7041ac4bb28ed |
| src/renderer/recording_device.cpp |9de1afc1e1c264c9222be21789d9f6ade7d75dc71975d41f3bcca3530c24690f |
| tests/original_rendering/camera_startup_probe.cpp |e3ed0ab5c571557a0579d8e2a5395d1c94feb3d27f5c2cc8d58292fa0ec47205 |
| tests/original_rendering/test_w3d_camera_startup.py |19b4e1e46dd47ca74e0c6c0a4771ab36d624345eb48133e4b06b279432eb1e9f |
| tests/renderer/test_bgfx_transaction.cpp |0690c590b8a65fbb85931a743008d0e7df808f484854b8b9c0ae52269ef59856 |
| tests/renderer/test_bgfx_transaction_resource.cpp |0a3c81e1c1db7b88c22ba288d29bfd9f3b1eb0f5b22fc1aff6bc82d2ebd09539 |
| tests/renderer/test_recording_transaction.cpp |d5cee5e468df750d4052b5a26ebb4b68e8cf193324d430d8e8714191b45c33a7 |

All six complete configure/full builds pass on this frozen executable composition.
All six CTest manifests verify339 registered tests, exact17, LAN4 and297 canonical
controls.102 selected binary hashes are frozen after all six builds. Per-gate
source, preserved-worktree and binary checks prevent evidence reuse across byte
changes. No final gate is inferred from accepted R2 history or exit status alone.

| Complete build | Result | Seconds |
|---|---|---|
| GCC Debug |0 |204.460 |
| Clang Debug |0 |24.586 |
| GCC Release |0 |84.371 |
| Clang Release |0 |63.968 |
| GCC sanitizer |0 |62.289 |
| Clang sanitizer |0 |341.353 |

Fresh final exact17 passes on all four focused configurations. Complete log
sanitizer/validation category audits are zero, and every post-gate source,
preserved-worktree and102-binary identity check passes.

| Focused configuration | Result | Seconds | Camera seconds | Prop-frame seconds |
|---|---|---|---|---|
| GCC Debug |17/17 |357.09 |43.87 |249.46 |
| Clang Debug |17/17 |361.17 |44.17 |255.29 |
| GCC sanitizer |17/17 |1526.56 |162.58 |1125.66 |
| Clang sanitizer |17/17 |1360.67 |171.06 |980.23 |

Fresh minimal/headless controls pass8/8 on GCC in8.06s and Clang in7.96s,
with zero diagnostic categories.

Both strict host LSan gates pass under exactly `ASAN_OPTIONS=detect_leaks=1`,
without a strict UBSan override. Complete sanitizer/leak/validation category
audits are zero. Post-gate source, preserved-worktree and102-binary hashes
remain exact.

| Strict configuration | Result | Seconds | Camera seconds | Prop-frame seconds |
|---|---|---|---|---|
| GCC sanitizer |17/17 |1519.30 |164.24 |1116.16 |
| Clang sanitizer |17/17 |1365.16 |172.15 |982.52 |

Validation-enabled generated camera physical passes all four configurations
under the final registered250s child bound. Each complete log has zero sanitizer
and Vulkan validation categories and the wrapper requires every source/capacity/
physical/two-generation/resource marker.102-binary and source/preserved-worktree
identity checks pass before and after every gate. The binary-identity manifest
SHA256 is `1cc9913cc12c5fa1402a01b3fde54555ffeb4374dec1bdae6a42b9e25dc2ca84`.

| Configuration | Camera physical status / wall seconds | Renderer physical result / CTest seconds |
|---|---|---|
| GCC Debug |0 /90.552 |2/2 /1.24 |
| Clang Debug |0 /91.572 |2/2 /1.25 |
| GCC sanitizer |0 /369.493 |2/2 /9.60 |
| Clang sanitizer |0 /388.780 |2/2 /9.24 |

Established validation Vulkan controls pass GCC3/3 in5.09s and Clang2/2 in4.22s,
with zero diagnostic categories. Serial LAN passes4/4 on all six configurations:
1.71/1.71/1.71/1.71/1.82/1.79s in prescribed configuration order. Complete
renderer physical/Vulkan/LAN category audits are zero and all identity checks
remain exact. No build or test workload overlaps these heavy gates.

Six fresh serial canonical suites are complete, with full sanitizer/validation
category audits zero and post-gate source/preserved/102-binary identities exact.

| Configuration | Canonical result | CTest seconds |
|---|---|---|
| GCC Debug |297/297 |685.72 |
| Clang Debug |297/297 |680.88 |
| GCC Release |297/297 |325.71 |
| Clang Release |297/297 |273.15 |
| GCC sanitizer |297/297 |2599.61 |
| Clang sanitizer |297/297 |2263.32 |

The unchanged bounded redacted scene-once wrapper was run with the existing
privately provisioned expansion/base pairing after generated acceptance. It
returned status3 with reach1/setup1/scene0, init3/logic5/mapini4, stage-request1,
Recording0 and teardown1; ASan/UBSan categories were zero. A memory-only
public-literal diagnostic classified the exact next rejection as
`W3DDisplay.cpp:126`, immutable-tree frame-owner checkpoint. Neither the prior
camera-capacity nor bounded-camera rejection remained. No private selector,
root, logical name, asset, raw output or image was recorded. The earlier
same-root diagnostic had stopped sooner and is not R3 guard-closure evidence.
This establishes R3 guard closure, not retail-scene acceptance; active08 owns
the next frame/scene continuation. Final exact ledger, preserved-path,
staged-diff, ABI and source/binary hash audits passed before this commit.
