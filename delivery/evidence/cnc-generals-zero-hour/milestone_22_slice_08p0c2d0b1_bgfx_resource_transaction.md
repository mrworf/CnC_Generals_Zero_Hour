# M22 slice 08P0C2D0B1: bounded bgfx candidate resource publication

Parent: `9b7701a5a5645a93a9a9ef1bf2f5b430428b5b88`.
Status: independently accepted on the refrozen source below; frame replay and
engine source-stage wiring remain deferred.

## Ownership and generated controls

The public bgfx device admits bounded idle-preparation transactions only.
Frame capability remains false. Admission copies five resource-slot baselines
and reserves candidate/retirement ownership before publishing an exact
device/sequence/generation/mode token. Candidate slots are append-only; abort
restores accepted records and retains generation-advanced tombstones.
Exceptions poison before diagnostic formatting. Fault counters remain consumed.
Malformed or nested admission and foreign finish attempts do not poison or
consume the existing owner. Commit/abort are allocation-free, no-throw and
exactly once, with no native destroy or frame call.

Full ordinary sampled uploads retain known mip source bytes. Transactional
updates COW a native texture, preserve unchanged known mips and upload only to
the candidate. Commit retains the public accepted handle; abort leaves prior
native contents untouched. Render-target upload fails closed in a transaction;
fresh target identity can represent resize without reconstructing prior pixels.
Unknown mip contents are not fabricated. Pending native ownership retires at
an ordinary boundary, wait or shutdown, never as an abort/frame shortcut.

Pinned native shader/program creation can deduplicate native indices while
incrementing reference counts. The journal tracks each acquired ownership unit,
not unique handle values. Commit matches one unit per surviving live slot;
excess/canceled units retire once. Abort retires only candidate acquisitions.
This confirmed correction is covered by repeated aliases, replacement, drain
and active shutdown, and recorded as a narrow reusable AGENTS lesson.

The generated CPU witness exercises the actual metadata/token/budget owner.
Physical controls run two complete device generations and cover all five
resource families; exact retirement capacity and bound+1; candidate generations;
twelve partial checkpoint-copy allocation boundaries; eight operation faults;
four post-native-publication faults; prior vertex/index/uniform byte rollback;
COW repeated mip updates; padded RGBA8/BGRA8/BGR5A1 content; survivor mip pixels;
fresh target creation; nine shared shader/program ownership units through abort,
canceled-alias commit and replacement; eleven forbidden idle side routes;
diagnostic allocation poisoning; command/byte bounds; ordinary/wait/shutdown
drain; zero public/native-owned residual. Readback occurs only after finish.

The final admission audit found an existing owner's poison state changed by a
rejected nested attempt. Admission now returns a fixed side-effect-free error.
Eight nested/malformed classes and four independently mismatched token fields
prove the original owner can still create/upload, then commit or abort once.
All earlier build/focused results are superseded by this correction. No source
or test edit follows the recorded refreeze.

## Frozen hashes

- `include/zh/platform/bgfx_device.h`: `6aae000ac7c5618e34c3d4acd48e3b019d685e7d0060b6dc904e775e29a64cee`
- `src/renderer/bgfx_device.cpp`: `57163ae74bcd21d16b776bfd3741122b12fe0f4318a6bdb6f2b552e32704a7fb`
- `src/renderer/bgfx_transaction_state.h`: `748b1d8397334d10a62d3f6b88a1ebdc5d8dc2169b91e46c3501f11fec6baedc`
- `tests/renderer/test_bgfx_transaction_resource.cpp`: `e504f668af14bf490978002a6ce337c3f230b597ff280d3448d52709537d0f96`

## Acceptance

All six refrozen complete builds pass. Clang sanitizer completed all 810
dependency-log recovery work items without an actionable source error.
Native focused CPU regex
`^(renderer_bgfx_transaction_resource|renderer_recording_transaction|original_w3d_first_gpu_edge)$`
passes 3/3 on GCC and Clang (0.08s each), GCC sanitizer 3/3 (0.33s), Clang
sanitizer 3/3 (0.25s). Sanitizer focused runs use detect_leaks=0.

The presets do not register the opt-in new physical CTest. Empty selection was
classified as non-evidence. Required direct physical entry is
`VK_INSTANCE_LAYERS=VK_LAYER_KHRONOS_validation python3 tools/run_validation_clean.py build/<preset>/renderer_bgfx_transaction_resource_tests --gpu`
with host graphical escalation. It passes on final GCC/Clang native source
(0.426s/0.416s) and on both sanitizer toolchains with detect_leaks=0, serially.
No validation error/VUID or sanitizer finding occurs.

The following serial canonical suites pass on the refrozen source:

| Configuration | Canonical result | Total seconds | Terrain seconds |
| --- | --- | ---: | ---: |
| GCC Debug | 271/271 | 278.15 | 59.85 |
| Clang Debug | 271/271 | 263.30 | 59.60 |
| GCC Release | 271/271 | 147.97 | 24.81 |
| Clang Release | 271/271 | 100.03 | 23.69 |
| GCC ASan+UBSan | 271/271 | 869.57 | 263.80 |
| Clang ASan+UBSan | 271/271 | 689.82 | 223.31 |

Command: `ASAN_OPTIONS=detect_leaks=0 ctest --test-dir build/<preset>
-LE 'gpu|lan|retail' --output-on-failure`. All six suites ran serially after
build load cleared. No failed assertion, sanitizer finding or timeout occurs.

Exact strict host LSan uses `ASAN_OPTIONS=detect_leaks=1` only, no UBSan
override, selecting `^(renderer_bgfx_transaction_resource|renderer_recording_transaction)$`.
Both host-escalated serial runs pass 2/2: GCC 0.11s, Clang 0.09s.
Established host Vulkan controls pass GCC display/bootstrap/map 3/3 (5.32s)
and Clang display/map 2/2 (4.19s) with their required validation-clean fixtures;
no validation-error/VUID category occurs. These are unchanged original-source
physical controls, separate from the new resource transaction witness.

Exact serial host LAN command is `ASAN_OPTIONS=detect_leaks=0 ctest --test-dir
build/<preset> -L lan -j1 --output-on-failure`. Results are 4/4 all six, in the
table order: 1.70s, 1.71s, 1.71s, 1.71s, 1.82s and 1.78s.
Public-header neutrality, original dependency ledger and diff checks pass.
Frozen source/test hashes remain identical after all final gates.

## Closeout boundary

Review confirms bounded no-throw CPU finish, native reference multiplicity,
fully initialized new aggregate/value members and field-only identity checks.
Exact owned staging excludes the unrelated renderer diagnostic. Delivered
against the recorded parent by
`delivery: M22 08P0C2D0B1 own bgfx candidate resource publication`.
No pinned runtime modification, frame replay, source shroud state, tree draw/factory,
retail input or allocator change belongs to this child. Dependency-next is
D0B2A's narrow public bounded native submission reservation; D0B2B supplies
the deferred frame journal only after that capability is accepted.
