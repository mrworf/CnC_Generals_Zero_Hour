# M22 slice 08P0C2D0B2B: deferred admitted bgfx frame journal

Parent: `0fc80b3` (plan-only admission refinement after accepted R2).
Status: independently accepted on the frozen source below; source-stage,
tree/factory and retail admission remain deferred.
No retail input or private diagnostic data was used.

## Complete ownership boundary

Bgfx frame mode captures owned immutable View/Draw/ResizeSwapChain/CompleteFrame
packets without native view/touch/submit/frame calls. CPU pass, initialization,
view prefix and window extent shadows restore on abort. Prepared typed VB/IB
and borrowed-attachment FBO units compose B1's exact native reference ownership;
captured unit leases and copied aligned uniform/geometry bytes cannot retarget
after public handle removal, COW replacement or generation reuse. Native alias
indices do not collapse create/reference multiplicity. Candidates retire at
ordinary/wait/shutdown boundaries, not native calls during commit/abort.

Capture checks logical generation, initialized vertex/index/reflection spans,
element/aligned payload sizes, shader/target rules, native sampler and integer
view representation. Shared texture stages require exact texture version,
source sampler identity/flags and reflected sampler handle; repeated uniform
handles require exact count/bytes. Conflicts fail closed rather than choosing
one shader stage by replay order. Signed view origins reject instead of narrowing.

Commit requires an inactive pass and one final present, including suspension.
CompleteFrame is uniquely last in the native manifest; later destruction is
CPU retirement metadata only. Accepted A submitBounded performs synchronous
deep-copy/validation/reservation before accepted-target touch and allocation-free
API-thread replay. Rejected reservation preserves the same journal for retry;
successful replay consumes exactly once. Worker/backend fatal allocation/device
loss remains outside the reversible API-thread guarantee.

Unsupported external encoder use is a distinct native readiness condition.
Pinned Context::end finalizes/posts but encoderApiWait resets encoder-handle
allocation only at an ordinary frame boundary. A rejects without emission;
abort, then ordinary wait and a fresh attempt are required. No live transaction
advances a frame to manufacture readiness. The witness independently retains
same-batch immediate retry for recoverable reservation rejection.

OriginalGpuEdge forwards exact optional device/token ownership and generation
admission. Successful abort clears source-frame phase without ordinary end_pass;
owner destruction cancels before source cleanup. Full source refs/maps/transform/
revision/filter rollback remains B0; no duplicated device checkpoint, tree phase
advancement, factory or scene/retail admission is claimed.

## Generated controls and packaging correction

Two native device generations cover thirteen checkpoint allocation boundaries,
eleven frozen-copy boundaries, six staged operation and three native-publication
faults; actual command/view/resource bound+1, diagnostic allocation poisoning,
late invalid providers/ranges/alignment, unfinished/duplicate completion and
wrong modes. CPU controls exercise exact 4096 command/resource, 64MiB byte and
256-view maxima/bound+1, signed origin32767/32768 and extent65535/65536, sampler
ownership aliases and uniform count/byte equality. No padding comparisons.

Compiled vertex/fragment providers prove identical aliases canonicalize and
conflicting source sampler/texture/uniform or distinct reflected sampler aliases
reject without candidate publication. Physical red full draw, green half-view
draw and blue selected clear prove source view/draw/clear order. Prior accepted
pixels survive each failure; ordinary queued prefixes are neither reset nor
rewritten. Controls cover resize abort/commit/retry, suspension, native rejection,
captured texture COW/removal/new handle, program/shader deduplication/removal,
post-capture geometry/index/uniform replacement, exact four-unit typed retirement,
idempotent drain and zero final public/native-reference residual.

Real Recording edge controls run twice, proving unsupported capability absence,
generation/nested and all four token mismatch fields, partial-frame abort,
commit once and owner removal. B1's physical resource witness remains required.

A pre-matrix packaging audit found that generated test shaders must not share
the strict shipping shader root. They now compile under generated/bgfx-transaction,
use a self-contained fixture device root with relative paths and are never installed.
Only current generated video VS/FS binary, manifest and layout are copied for
baseline/presentation; scope checks exact byte identity. Production absolute/
escaping-path rejection remains unchanged. A separate canonical scope
control checks exact fixture outputs, shipping exclusion and installation
exclusion; the existing exact shader-family closure is unchanged. Only sixty
owned superseded generated fixture files were quarantined recoverably outside
the repository. Earlier focused results are superseded by this correction.

## Frozen source

- CMakeLists.txt: `3576ef36a03de09fde75b06c63cb975ecd76f76172076d60bc87277cb0bf3183`
- include/zh/platform/bgfx_device.h: `3c8ae087204323cbb2720f2d97a0c3a62078d4a968fbff28bc2ded51ffb59501`
- src/renderer/bgfx_device.cpp: `d9d3f4bf3cdadee058189981da0862c2544570c500abf125ab971395a3d97e29`
- src/renderer/bgfx_transaction_state.h: `bd3eab245b6ad51d939211e993170d3e0f28f0517bf07111c567c62966696c35`
- src/original_runtime/original_gpu_edge.cpp: `65fe1caa5f5c4273ac62e7b46a69002956d228779fc45fd7028b0585374c3f81`
- src/original_runtime/original_gpu_edge.h: `35a9b66be66106f7994324ffa293aff3868d1aaed738e6c43d6159d17f6ff512`
- tests/renderer/test_bgfx_transaction.cpp: `8d2c2face2990039538809d9011ebdef4e839473c1b9c5670f5b35a328579224`
- tests/renderer/test_bgfx_transaction_shader_scope.py: `01624b4870ae075e7b2e514d6a0a31758bf4baad6219ef260edd7695921a08ba`
- tests/renderer/test_bgfx_transaction_resource.cpp: `80d6a46db7c5ccd7841692072c096a5f823e83560ff28ef6dc5a4c3b8fd713f1`
- tests/original_rendering/test_original_gpu_edge.cpp: `251422493c48cc0e7d9511ad0fafb5b338540f3fc05c6be034579b42eac53a40`
- tests/renderer/shaders/transaction_alias.vert: `c7d90258324e22a917b1ff6a69c5761011e8e2a98d131755d44d44dc670ca556`
- tests/renderer/shaders/transaction_alias.frag: `046f8b88279a6628d2f1eb38dffd4e1edacb06aee7952901ab972520f0d6649a`
- tests/renderer/shaders/transaction_distinct.vert: `ae790d2ab295ffc465b825abb1b3123bdbec8d762383363827e650bc67206a06`

## Final acceptance

Final-source native/sanitizer focused set includes the new transaction and
scope controls, unchanged strict shader-family closure, B1 resource and A
reservation CPU controls, Recording transaction, original first edge and real
edge failure controls (8 tests). Sanitizer use is detect_leaks=0, no UBSan override.
Strict host LSan selects transaction/resource/reservation/Recording CPU controls
with exactly ASAN_OPTIONS=detect_leaks=1 and no UBSan override, serialized.
All four corrected focused configurations pass 8/8; whole focused logs have
zero sanitizer/runtime-error findings. Twelve serial host-generated physical
controls pass: journal, resource and native reservation on GCC/Clang native
and both sanitized builds, with detect_leaks=0 and Vulkan validation wrapper.
Exact serial host strict LSan passes 4/4 on each sanitizer configuration
(GCC0.16s, Clang0.12s), with no UBSan override or findings.
All six complete final-source builds pass. Both native debug canonical nonretail
suites pass 274/274, with their complete logs containing no sanitizer/
runtime-error findings. GCC release additionally passes 274/274 (148.22s), clean.
Clang release passes 274/274 (100.42s), also clean.
GCC sanitizer passes 274/274 (870.16s), with a clean complete-log audit;
its isolated terrain witness passes in 264.15s without timeout.
Clang sanitizer passes 274/274 (691.02s), with a clean complete-log audit;
its isolated terrain witness passes in 223.47s. All six canonical suites and
their whole-log audits are complete. No failure, timeout or sanitizer finding
occurs. The Clang sanitizer full build completes all 849 dependency-log recovery
work items without an actionable source error. Unrelated renderer diagnostic
remains untouched and excluded.

Canonical queue uses `ASAN_OPTIONS=detect_leaks=0 ctest --test-dir
build/<preset> -LE 'gpu|lan|retail' --output-on-failure`, serially after all build
load clears. Each complete `LastTest.log` is audited for `Sanitizer|runtime error:`
before advancing or later host selections can replace it. No UBSan override.

| Configuration | Complete build | Canonical result | Seconds |
| --- | --- | --- | ---: |
| GCC Debug | pass | 274/274, clean complete-log audit | 278.13 |
| Clang Debug | pass | 274/274, clean complete-log audit | not retained |
| GCC Release | pass | 274/274, clean complete-log audit | 148.22 |
| Clang Release | pass | 274/274, clean complete-log audit | 100.42 |
| GCC ASan+UBSan | pass | 274/274, clean complete-log audit | 870.16 |
| Clang ASan+UBSan | pass | 274/274, clean complete-log audit | 691.02 |

Established physical Vulkan controls run serially with host graphical escalation
and `VK_INSTANCE_LAYERS=VK_LAYER_KHRONOS_validation`: GCC display/bootstrap/map
3/3 in 5.07s and Clang display/map 2/2 in 4.11s. Validation-clean fixtures report
no validation-error/VUID category. They remain distinct from the twelve generated
transaction/resource/reservation physical proofs.

Exact serial host LAN uses `ASAN_OPTIONS=detect_leaks=0 ctest --test-dir
build/<preset> -L lan -j1 --output-on-failure`. All six selections pass 4/4,
with clean complete-log audits, respectively 1.71s, 1.71s, 1.70s, 1.71s, 1.82s
and 1.78s in the matrix order. Required host escalation is retained; no transport
or graphical substitute is used.

Final ledger/public-header/whitespace checks pass. All thirteen recorded frozen
source/test hashes remain exact. The accepted bounded native patch remains
byte-identical (`adf7c210cad7b296c548944ae37b86d75460221d3b3a6cb60574230e6b1cba47`);
this child does not alter native reservation or allocator semantics. Review
confirms initialized aggregate members, field-only comparisons, exact reference
multiplicity and allocation-free no-throw finish/retirement within admission.
Exact staging contains only this journal owner, its coupled edge/tests/contracts,
ledger and delivery artifacts. No unrelated renderer hunk is included.

Commit boundary: `delivery: M22 08P0C2D0B2B defer admitted bgfx frame commands`.
The independently planned B2/B/D0 evidence-only aggregates follow; B0 still owns
complete delayed-source refs/maps/transform/revision/filter/sampler rollback.
