# M22 slice 08Q0: source-owned house-color evidence

Status: complete; frozen source passes final acceptance. The exact Q0 commit is
the governing index's slice commit, excluding the preserved active08 trial.
Dependencies: accepted 08L2R1 `4c37e9e5ddc2509cbcae518df2303f92108763a2`,
08Q0R1 `b16b39b3b420b6357ada08208922fbbd6fb0ec53` and the accepted texture,
filter-tuple, source-pin and device transaction owners listed in the Q0 plan.
Plan corrections: `6b0738c5ba108c1d46a0b43ea7325b4b69453c27` and
`8c4b17a42f552978e35a462332462feab9a0fc2b`.
Exact payload parent: `8c4b17a42f552978e35a462332462feab9a0fc2b`.
Scope: public generated source/texture/cache data only; no retail input or evidence.
The active08 trial and unrelated renderer diagnostic remain separate and unstaged.

## Ownership and source semantics

Actual W3DAssetManager mesh/HLOD custom creation consumes canonical detached
TextureLoadTask Begin/Load staging without source initialization, fallback,
LastAccessed changes or GPU publication. Original remap loops and PixelSize/
Convert_Pixel source bodies are shared, preserving D/d palette matching, A/a
inverse-alpha/hue behavior and unchanged-selector copying. Ordinary BitmapHandler
and generic hash/vector mutation remain unchanged.

Exact recolored level zero is retained privately. Lower levels use the explicit
D3DX8-compatible BOX contract: each reducible axis contributes two samples, a
singleton contributes one, and each channel averages with nearest/half-up rounding.
Generated ties/extrema/singletons and overlapping Wine D3DX9 native-expectation
references agree. This is not a claim of byte-exact D3DX8 channel quantization.

Bounded color-cache table/bucket/key graphs and prototype pointer arrays are built
offside and publish through nonthrowing swaps. R1 clone/RNG guards survive through
Q0 publication. All rejected candidates reverse exact refs and mapping ownership;
accepted sibling cache/prototype/source identities and order remain unchanged.
Finite zero/default/negative scales retain native semantics. Absent Edge is valid
only for plain geometry/prototype work without recolor or replacement; actual
private-backed residency requires the exact idle Edge generation.

Backing declarations and Linux-only access/override declarations are identical
across CPU/full/minimal consumers; no CPU-macro class-layout fork. Windows paths
remain unchanged. Private friend fault controls have no selector/serialized ABI.

## Generated controls

Registered runtime, source identity and six-provider-removal controls exercise
real mesh/HLOD creation, remap selectors, filters, cache hits/aliases, missing and
malformed input, finite/nonfinite scales, no-Edge/busy/stale-generation rejection,
retained-pixel replay without input rereads, reset/removal and two generations.
Interior candidate faults check exact refs/cache/prototype/mapping/native identity
and retry, including clone-local random sequence survival through later failures.
Sibling faults are repeated twice at each boundary and assert bounded operation
deltas without handle alias reuse. Actual 65536-slot cache/prototype capacity and
bound+1 fixtures reject without source/device work; actual prototype growth uses
the authored 32-slot policy. Exact 64MiB level-zero success and derived-mip
budget rejection preserve accepted bytes/identity.

Recording retains accepted monotonic commands/dead resource-slot diagnostics.
Immediate source rollback therefore checks exact source-owned state and live
resources separately from precise candidate create/upload/destroy deltas, rather
than imposing universal DMA equality on a live diagnostic device. Fresh-equivalent
Edge/device/provider teardown must restore exact raw/pool baseline.

Physical generated source drawing samples nonuniform recolored level zero and
the final 1x1 mip, preserves prior pixels on a late candidate fault, and retries
without altering the accepted frame. Both generations have 3655 visible pixels
and 3095 distinct nonblack colors on the generated fixture.

## Superseded development findings

The newly added overall baseline initially exposed four process-global default
DX8 render-state map nodes. The fixture now calls the same public
DX8Wrapper::Reset_Source_State before capturing its baseline. No source reset or
device teardown behavior changed.

The physical-only remaining block was localized by generated debugger allocation
traces to WW3D::End_Render → Debug_Statistics::End_Statistics → Record_Texture_End:
the static texture_statistics_string receives an empty StringClass assignment and
retains its 9-byte buffer in dmaPool_16. The second generation reuses that buffer.
The fixture now invokes the existing public Debug_Statistics::Shutdown_Statistics
after rendering, which releases that exact statistics owner. The initialized
whole raw/pool baseline assertion remains exact; no expectation or production
allocator/device behavior was weakened. These failures are superseded development
evidence, not acceptance gates.

## Final-source gate contract

Exact nine-control selection:
`^(original_w3d_house_color|original_w3d_house_color_identity|original_w3d_house_color_provider_removal|original_w3d_texture_decisions|original_w3d_texture_identity|original_w3d_texture_provider_removal|original_w3d_generated_construction|original_w3d_modeled_volume_ready|original_w3d_tree_module)$`.
Run native GCC/Clang and both sanitizers with `ASAN_OPTIONS=detect_leaks=0`.
Strict host LSan uses that selection and exactly `ASAN_OPTIONS=detect_leaks=1`,
without a UBSan override. Generated physical four-config command:
`ASAN_OPTIONS=detect_leaks=0 VK_INSTANCE_LAYERS=VK_LAYER_KHRONOS_validation python3 tools/run_validation_clean.py build/<preset>/original_w3d_house_color_tests --gpu`.
Host escalation is required for physical, strict LSan and LAN controls.
Six complete builds, six serial canonical nonretail suites (`-LE 'gpu|lan|retail'`),
established native Vulkan, serial LAN4/4 all six, complete sanitizer category scans,
source/link/header/ledger and exact staged-path review remain required.

Frozen executable composition contains 30 changed/untracked source/test/CMake
files, including the preserved active08 trial and unrelated diagnostic for byte
identity only. Q0 exact staging will exclude those other owners. Generated freeze
manifest: `/tmp/m22-q0-final-source.sha256`.

| Owned frozen file | SHA256 |
| --- | --- |
| House-color witness | `16b5e999a0b5a682507ae18b432d8a101fda47bc00a7e3c27f10b83a3da2c75b` |
| Original W3DAssetManager source | `106b455a7b370b6fbe42cb7d70179385ec33c1fc95a54aeb6e6e41f24087ac09` |
| Owner-local cache/prototype transaction | `cb186f1b61d448c414809b116449e38f26a5fd79b425461df12505f615045042` |
| Private recolored backing/mip value | `739bb723c8814a45ff7e98403da7a6b1d0266370bf57757546c7031444df8f5c` |

All six complete builds passed. Clang sanitizer recovered incomplete Ninja
metadata and rebuilt 929 tasks without actionable compiler error; this was build
metadata recovery, not source remediation. Extracted native pixel-packing bodies
match the accepted parent implementation apart from whitespace. Source ledger,
owned diff and frozen-hash checks pass. Exact staged-path review excludes every
active08 path/hunk and the unrelated renderer diagnostic.

| Final configuration | Complete build | Exact focus | Seconds |
| --- | --- | --- | --- |
| GCC Debug | pass | 9/9, category-clean | 15.48 |
| Clang Debug | pass | 9/9, category-clean | 16.70 |
| GCC Release | pass | canonical 286/286, category-clean | — |
| Clang Release | pass | canonical 286/286, category-clean | — |
| GCC sanitizer | pass | 9/9, category-clean | 44.93 |
| Clang sanitizer | pass | 9/9, category-clean | 31.03 |

Strict host LSan passes the same nine controls on GCC in 66.86s and Clang in
34.49s with exactly the options above, host escalation and complete clean category
scans. All four generated physical configurations pass both generations with
3655 visible/3095 distinct pixels, exact terminal mip sampling, prior-frame
preservation and clean retry, live-resource zero and initialized raw/pool equality.
Configured Khronos validation is clean; no physical route was weakened or replaced.
Established native Vulkan passes GCC3/3 in 8.24s and Clang2/2 in 5.51s. Serial LAN
passes 4/4 all six in 1.72–1.86s. These gates ran under isolated load after all six
relinks. Their complete logs are category-clean.

Six serial canonical nonretail suites pass from the frozen composition, each
enumerating 286 controls. No prior R1 or superseded Q0 result is substituted.
The full sanitizer logs are category-clean and the frozen source hashes match.

| Final canonical configuration | Result | Seconds |
| --- | --- | --- |
| GCC Debug | 286/286, category-clean | 388.82 |
| Clang Debug | 286/286, category-clean | 339.87 |
| GCC Release | 286/286, category-clean | 178.45 |
| Clang Release | 286/286, category-clean | 129.31 |
| GCC sanitizer | 286/286, category-clean; registered tree-decal unchanged | 1200.74 |
| Clang sanitizer | 286/286, category-clean | 950.85 |

The first GCC sanitizer canonical attempt reported one actual failure at CTest
187, original_w3d_tree_decal: generation0 reached the unchanged 90-second
subprocess limit (`TimeoutExpired`, 90.60s CTest duration), without a sanitizer or
validation finding. The accepted R1 run passed that same two-generation control
in 132.05s total. A read-only host snapshot immediately after detection showed
multiple concurrent high-CPU processes with lifetimes overlapping the timeout;
contention is a possible cause, not established causality or an acceptance waiver.
No external process, source, assertion or timeout was changed. The completed
attempt passed all other 285 controls; the same registered isolated control also
reached generation0's unchanged limit in 91.65s without a sanitizer category.
These attempted gates are not passing final evidence.

An explicitly approved generated-only timing driver reused the unchanged helper,
executable, inputs and assertions with its existing run timeout parameter set to
240s. This was diagnosis only, not acceptance or a registered-bound change. Its
two positive processes completed in 94.56/96.49s; an identical second diagnostic
completed in 69.08/67.45s. All assertions and three negative controls passed in
both runs. The registered 90s bound remains unchanged. Read-only source audit
shows tree-model acquisition uses the unchanged ordinary Create_Render_Obj route;
the generated grid/intent/byte/batch capacity loops invoke no house-color work.
Bootstrap ModelDraw can enter the color=0 geometry route, whose additional work
is bounded residency inspection and guarded prototype publication, not recolor
or upload. Native release tree-decal timing is effectively unchanged against the
accepted R1 composition (GCC12.62→12.38s, Clang12.37→11.97s). Diagnostic wall-time
variation and concurrent high-CPU host work support an environmental explanation
as an inference only; they do not identify an exact slow source boundary or waive
the required registered and canonical retries. Read-only debugger attachment was
denied by the host ptrace policy; no policy or external process was altered.
The exact registered 90s control then passed both positive generations and all
negative assertions in 138.88s total, with a complete clean category scan. The
canonical retry also passed it in 132.98s, matching accepted R1's 132.05s total;
terrain attachment and tree draw returned to 267.49/84.73s versus R1's
265.01/84.46s. These unchanged-byte results support the environmental timing
classification without changing or waiving any registered assertion or bound. Only
the remaining sanitizer canonical suites were run; the four completed
native canonical suites and all completed focused/strict/physical/LAN gates retain
the identical frozen executable provenance. GCC and Clang sanitizer canonicals
completed 286/286 with clean full-log scans. No test bound or workload was changed.
