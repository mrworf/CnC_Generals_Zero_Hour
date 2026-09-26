# M22 slice 08P0C2D0B2A: bounded native bgfx submission reservation

Parent: `e7aa22e55052b23af7ae802460407a691be86535`.
Status: corrected final source accepted as non-regressing with explicitly isolated
pre-existing sanitizer categories below. Corrective presentation/filter owners
must be delivered before D0B2B. D0B2B device journal,
source shroud state, scene/tree/factory and retail admission remain closed.

## Ownership boundary

The reviewed public C++ extension owns one synchronous immutable copy, complete
ordered native identity/refcount validation, private replacement storage and
allocation-free API-thread replay. There is no exposed token interval. Rejection
restores render-completion synchronization without frame advancement, leaves
caller receipt and accepted commands/resources/pixels unchanged, and consumes
no receipt sequence. Success alone publishes exact context/sequence, caller
generation, frame numbers and binding counts. D0B2B owns logical handle
generations and high-level at-most-once journal consumption.

Limits are 4096 commands/draws/resource mutation operations, 256 unique views and
64 MiB checked storage including copied parameters/payloads and allocation nodes.
Supported static draw/view/touch/resize/retirement/frame categories are explicit.
Zero-size resize, changed native window identity/formats, dynamic retirement,
cached view uniforms, active debug text, profiler/custom callbacks, overlapping
encoders and single-thread rendering fail closed. Source suspension is frame-only.
Worker/backend allocation/device loss remains the ordinary non-reversible boundary.
No engine private native header, allocator replacement, original source/retail ABI
or serialized state change occurs. Exact combined shaderc/runtime patch checks
preserve pins/licenses, offline builds and no implicit acquisition/system fallback.

## Frozen executable hashes

- `third_party/bgfx_bounded_submission.patch`: `adf7c210cad7b296c548944ae37b86d75460221d3b3a6cb60574230e6b1cba47`
- `tests/renderer/test_bgfx_submission_reservation.cpp`: `3633422a85cebe1e49f0b012df8cb3a5b9183135276e66a5677d7a340e6009e1`
- `CMakeLists.txt`: `73312472ab36bca93fea8da14a67e73d69367dbc895e5deab0e59309956e1482`
- `cmake/ZhBgfxRuntime.cmake`: `f55838e23a7c15d78188f7a72852a963f835ea9f861d4525bf1aa130b16090ed`
- `tools/renderer/bootstrap_bgfx_runtime.sh`: `eb386ba1066d49e27f8cfc760265f34eabb2b4d023c413d55db43a7ec35a6b7a`
- `tools/renderer/bootstrap_bgfx_shaderc.sh`: `b0298882815a777462b98aa7b4f0396932d74d391b26a5fc07483a7deb89f9e9`

## Final focused checkpoint

GCC CPU reservation/resource/device-contract selection passes 3/3. Both generated
two-generation physical runs pass with validation enabled: imported native Release
runtime and separate native Debug runtime compiled with BX_CONFIG_DEBUG=1.
Required assertions stay enabled in the Debug proof. Explicit nontrivial
UniformBuffer candidate construction/destruction, typed arena publication and
zero API-thread replay allocations are audited.

Controls include each checked small/maximal admission-allocation boundary,
malformed/category/bounds/overlap/thread/handle/order rejection, duplicate bindings
and uniforms, immutable command/payload mutation after snapshot, late recognized
retirement rejection preserving accepted pixels, ordinary queued prefix, exact
shader/program ownership units, stale retirement rejection, 256 views, 4096
commands, maximal uniform stream and render/binding/depth arenas. A bounded
1048576-pattern generated search derives exact pinned hash collisions with opposite
sampled-stage values; full equality preserves three native binding entries and
physical sample correctness. Window publication/resize, frame-only suspension,
retirement and two-generation zero allocator residual pass. All earlier exploratory
results are superseded by this frozen-source checkpoint.

## Complete acceptance

The first acceptance queue was stopped before completion after a read-only
native encoded-state audit found out-of-range lookup-table inputs could pass
the reserved-bit check. No earlier final matrix or host result is acceptance
evidence for the corrected source. Complete public pure predicates now cover
all state/sampler gap bits, depth/blend/equation/cull/topology, independent rgba
target encodings, both stencil faces, min/mag/compare sampler fields, native
stage bounds, swapchain flags/MSAA/format extents and clear masks. Whole-input
preflight rejects encoded invalids before any checked allocation; the immutable
copied lifetime model repeats validation. Field-width-exhaustive address/mip/
border/ref/mask/alpha/point-size values have no representable in-field bound+1;
CPU maxima prove their entire width is valid. The third independent target has
only two equation bits, all valid. Native source limits are pinned, not guessed.

The fully corrected GCC CPU witness and generated two-generation physical
Release/Debug proofs pass. Both native libraries rebuilt after final stage/
format controls; the independent Debug fixture uses BX_CONFIG_DEBUG=1.
Exact hashes above define the new freeze. All complete builds/canonical and
host acceptance gates are rerun on this source; the earlier queue is non-evidence.

Generated mixed manifests place a valid red clear/draw before each later encoded
invalid and assert zero preparation/replay allocator traffic, exact live-owned
residual, unchanged receipt and surviving accepted green pixels. CPU predicates
cover exact maxima, every representable bound+1 and each unknown/reserved bit.

All six refrozen complete builds pass; Clang sanitizer completes its 810-item
dependency-log recovery without an actionable source error. The final focused
regex `^(renderer_bgfx_submission_reservation|renderer_bgfx_transaction_resource|renderer_bgfx_device_contract)$`
passes 3/3 both native toolchains (0.02s each), GCC sanitizer (0.05s) and Clang
sanitizer (0.04s), with detect_leaks=0 on sanitized runs.

Exact strict host LSan uses `ASAN_OPTIONS=detect_leaks=1`, no UBSan override,
selecting `^(renderer_bgfx_submission_reservation|renderer_bgfx_transaction_resource)$`.
Both serial host-escalated runs pass 2/2: GCC 0.05s, Clang 0.04s.
Direct generated physical entry uses `VK_INSTANCE_LAYERS=VK_LAYER_KHRONOS_validation
python3 tools/run_validation_clean.py build/<preset>/renderer_bgfx_submission_reservation_tests --gpu`
with host graphical escalation. Serial final native runs pass GCC/Clang
(1.326s/1.285s), then both sanitized runs with detect_leaks=0
(8.161s/7.616s); no validation-error/VUID or sanitizer finding occurs.
The independent native Debug physical proof also passes with assertions enabled.

Established physical controls pass GCC display/bootstrap/map 3/3 (5.33s) and
Clang display/map 2/2 (4.23s), using exact validation-clean generated fixtures.
Exact serial host LAN command is `ASAN_OPTIONS=detect_leaks=0 ctest --test-dir
build/<preset> -L lan -j1 --output-on-failure`; all six pass 4/4 in the usual
GCC Debug/Clang Debug/GCC Release/Clang Release/GCC sanitizer/Clang sanitizer
order: 1.71s, 1.71s, 1.71s, 1.71s, 1.83s, 1.78s.
Offline runtime/shaderc combined-patch bootstrap, public-header neutrality,
dependency ledger, shell syntax, diff and frozen hash checks pass. The narrow
AGENTS lesson records packed subfield bounds independently from known-bit masks.

All six serial complete canonical nonretail suites finish 272/272 on the
refrozen source. Successful sanitizer exits are qualified by the complete-log
classification below, not described as globally sanitizer-clean.

Command: `ASAN_OPTIONS=detect_leaks=0 ctest --test-dir build/<preset>
-LE 'gpu|lan|retail' --output-on-failure`, serially after all build load clears.

| Configuration | Result | Total seconds | Terrain seconds |
| --- | --- | ---: | ---: |
| GCC Debug | 272/272 | 277.96 | 59.76 |
| Clang Debug | 272/272 | 262.82 | 59.39 |
| GCC Release | 272/272 | 147.64 | 24.75 |
| Clang Release | 272/272 | 100.06 | 23.83 |
| GCC ASan+UBSan | 272/272; unrelated diagnostics below | 872.71 | 264.68 |
| Clang ASan+UBSan | 272/272; unrelated diagnostics below | 690.85 | 222.85 |

### Successful-exit sanitizer log audit

GCC's complete successful log contains three nonfatal UBSan reports; it is not
reported as globally sanitizer-clean. Exact CTest 161 `original_w3d_presentation`
reports null FileSystem member calls at unchanged `GlobalData.cpp:1028/1039`.
The unchanged fixture constructs GlobalData before a FileSystem provider;
accepted slice 07FG0 already records this category. Exact CTest 251
`original_w3d_texture_decisions` reports invalid `TxtAddrMode` value 9 at unchanged
`texturefilter.h:112`. Its unchanged fixture explicitly creates that invalid enum
at `test_original_texture_decisions.cpp:426`; the GCC sanitizer executable was
built before the accepted D0B1 parent and has not been rebuilt for this child.
Individual GCC reruns reproduce these categories (2/2 successful exits, 0.23s),
without suppressions or a strict UBSan override.

Neither executable, native dynamic dependency list nor source link map contains
bgfx/submitBounded. Both use original providers and Recording, so this child's
native encoded sampler preflight/replay is unreachable. All four implicated
source/fixture blobs equal HEAD; there is no current-child source change.
The new native mixed-invalid sampler controls instead reject resolved sampler
encodings before any admission allocation, table/replay effect, receipt advance
or pixel touch, and pass under both sanitizer configurations plus Vulkan.
These source/fixture categories require separate corrective ownership before
M22 acceptance; they are not waived by successful CTest exits.

Clang's full-log and individual rerun (2/2, 0.09s) agree. CTest 174
`original_w3d_presentation` reports the same GlobalData calls plus the
`FileSystem.cpp:175` callee. CTest 276 `original_w3d_texture_decisions` reports
the same value-9 category at `texturefilter.h:114/112`, plus invalid profile
value 99 at unchanged `texturefilter.cpp:133/134`, explicitly constructed by
the unchanged fixture at line 462. These exact source/fixture blobs are also
unchanged; native bgfx is absent from both Clang executables and dependencies.
No other sanitizer category appears in either complete final log. New native
reservation/resource focused, strict host LSan and generated Vulkan witnesses
have no sanitizer finding. The parent authorizes this non-regressing A closeout
with explicit classification, not a suppression or M22 acceptance waiver.

Before D0B2B opens, persist two independently reviewable corrective owners:
presentation fixture/runtime service precondition and defined texture-filter
raw-value admission with complete pre-mutation range checks. Shipping startup
already creates FileSystem/local/archive before GlobalData. Current Linux
filter Apply mutates min/mag/mip stages before validating U/V; its correction
must reject the whole tuple before table access or stage mutation, including
profile admission. Both owners require affected sanitizer controls and a clean
complete canonical matrix before the deferred journal resumes.

Final exact hash, dependency-ledger, public-header and shell-syntax checks pass.
Staged whitespace checks pass for all non-patch files, and the applied native
source's `git diff --check` passes. The newly tracked unified patch's standard
space-prefixed tab/blank context lines produce artifact-format whitespace
warnings if treated as new source lines; exact byte comparison with the applied
native diff is required instead of altering those context bytes. The unrelated
renderer diagnostic remains unchanged and unstaged.
