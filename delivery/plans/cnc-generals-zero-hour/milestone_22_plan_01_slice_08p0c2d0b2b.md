# M22 plan 01 slice 08P0C2D0B2B: deferred bgfx frame journal

## Goal, dependencies and boundary

After D0B1 candidate/COW resource publication, D0B2A proved public native
reservation and corrective D0B2R1/R2 service/filter admission with a clean
complete sanitizer/canonical matrix, enable the public device's frame_commands mode and narrow
OriginalGpuEdge admission/commit/abort. Idle behavior remains D0B1; Recording
is already accepted, SDL/others fail closed. No tree advancement/factory,
shroud semantics, retail input or allocator rewrite. Authorization does not
apply to generated local work.

Historical presentation null-service and invalid texture enum categories are
explicitly isolated in A's qualified evidence, not suppressed or waived. R1
owns real generated FileSystem-before-GlobalData lifetime; R2 owns defined raw
filter/profile representation and whole-tuple preflight before publication.
Both must be delivered independently before any B2B production edits.

R1 and R2 are now independently accepted; R2's final-source six canonical
suites are clean, including both formerly qualified sanitizer categories.
The read-only B2B audit below refines the existing owner without opening B0's
source-stage checkpoint or changing the accepted native reservation API.

## Complete admitted command owner

Use D0A's exact token and bounded command/resource/byte/view descriptor.
Reject nested/cross-mode/active ordinary pass, wrong generation, overflow and
unsupported provider before mutation. Capture an immutable source-order batch
of begin/viewport/clear/draw/end and at most one final present. Preserve
ordinary work already queued and view order; never reset/frame to admit.
No native view mutation, touch, clear, submit or present occurs while staging.
Speculative CPU initialization/active-pass state is journal-owned and restores
on failure. Resources compose D0B1, with retained versions needed by commands.

Before native admission, validate every target/load/generation/extent/view,
pipeline/shader/format/depth-stencil rule, layout/base/index/initialized vertex
range, reflected source uniform offset/count/written bytes, texture/sampler/mip,
viewport/clear bounds and budget. Move existing late stencil validation before
all native work. Freeze exact uniform payloads, native texture/version/sampler
flags, program, state, view/rect/clear and geometry. All fallible CPU copies,
native VB/IB and borrowed-attachment FBO wrappers complete before commit.
Retain wrappers and providers through queued use; no borrowed-target destroy.
Changed sampled textures use COW versions; captured earlier draws retain their
exact earlier version. Render-target upload rejects before mutation as D0B1.
Every FBO belongs to the journal and uses destroyTextures=false.

At capture, acquire a lease on the exact B1 ownership unit/version, not merely
an opaque handle or a unique native index. Baseline and candidate native units
stay owned until completion-safe retirement; multiple commands may borrow one
unit without inventing native reference increments. Captured unit identities
cannot be retargeted by later slot removal, COW replacement or reuse. Freeze
CPU buffer/uniform bytes and sampler values at the same admission boundary.
Retirement preserves native create/reference multiplicity, including aliases.

The lowering maps source texture slot N in either shader stage to native stage
8+N. Shared source-stage aliases require identical captured texture version,
source sampler identity/flags and reflected native sampler UniformHandle;
emit one canonical binding. Conflicting aliases reject before checkpointed CPU
mutation. Distinct reflected sampler handles sharing one stage are explicitly
unsupported: pinned setTexture also writes the sampler uniform, and the accepted
A manifest cannot express both writes with one canonical binding. Never drop
one write or choose a winner by vertex/fragment replay order. Repeated native
uniform handles likewise merge only with identical reflected type/count and
frozen payload bytes; conflicting payloads reject. Different native stages may
not alias one sampler handle. Ordinary nontransactional behavior is unchanged.

Use checked UInt64 range arithmetic for vertex/index spans and reflected source
offsets; prove initialized bytes, native vertex counts, index element alignment,
uniform count/byte-size and payload alignment before copies or publication.
Validate viewport/clear intersection and integer coordinates before conversion.
Native manifest view origins are signed int16: reject non-representable x/y,
including a clipped clear origin, rather than narrowing. Width/height must fit
uint16 and native caps; view allocation respects both the admitted view budget
and the existing ordinary prefix. No unsupported scissor/depth-range behavior
may be approximated. Each staging operation finishes its complete validation,
budget admission, owned copies and candidate preparation before publishing any
checkpointed CPU pass/init/view state.

Presentation preflights SDL extent callback, immutable current window ownership,
generation, program/uniform/sampler/source, fixed triangle bytes and resize.
Use prepared native vertices, not commit-time transient allocation. Suspension
is frame-only completion. Require one final present and reject later frame
commands; allow only bounded retirement metadata afterward. Reserve complete
resize/retirement/frame-finish storage with D0B2A before accepted-target touch.
Claim/release/wait/readback side routes reject during the live journal; owner
destruction cancels before ordinary shutdown. Do not roll back irreversible
driver work or treat device loss as an ordinary rejected transaction.

The final present is required to complete frame mode, including zero-extent
suspension; an unfinished/offscreen-only frame cannot commit. Validate the
window callback result and native-cap extent before publishing resize shadows.
CompleteFrame occurs exactly once and is the last native manifest packet, as
required by A. Later destruction is bounded CPU retirement-queue metadata only:
no native Retire or other command follows completion. Prepared VB/IB/FBO units
drain at ordinary/wait/shutdown boundaries, never by a native destroy or frame
call during commit/abort. Edge integration forwards the exact device capability
and owner token only; B0 separately owns source refs/maps/transforms/revision/
pending sampler rollback, with no private duplicate checkpoint here.

This child constructs and orders the complete immutable native manifest before
calling D0B2A's single synchronous public admit/reserve/replay primitive. A
deep-copies, validates and reserves under native API/frame ownership, then
emits that admitted manifest before returning; there is no exposed native
reservation token or mutation interval. Recoverable native admission rejection
returns before accepted-target touch and leaves this journal available to abort
or retry. Native success returns a context/frame/sequence receipt; this child's
device token/generation owns consumption and at-most-once invocation.
An external unsupported encoder is not assumed released by bgfx::end: pinned
Context::end only finalizes/posts, and encoderApiWait resets encoder-handle
allocation at an ordinary frame boundary. A rejects that native admission and
preserves this journal; abort before ordinary wait, then use a fresh attempt.
Never advance a live transaction's frame to manufacture native readiness.
Recoverable reservation rejection separately proves same-journal immediate
retry; external encoder removal/readiness is a different admission condition.
Only after the entire immutable batch and native reservation succeed may the
no-throw exactly-once native replay emit source order. It uses owned data and public
admitted native operations only: no source registry lookup, callback, heap
allocation, wrapper creation, late validation or error diagnostic formatting.
Retire candidates/replaced resources in bounded completion-safe order. Abort
before replay destroys only candidates, restores CPU shadows/init/view/target
identities and invalidates candidate generations without aliasing retry.
Fault counters remain monotonic. Repeated/stale finish cannot submit or retire
twice. Ordinary nontransactional rendering remains unchanged.
Worker/backend allocation is outside the reversible API-thread boundary;
ordinary device-loss/error handling and physical validation cover it. Never
report driver work as rolled back or hide an API-thread allocation in that
category. Unsupported native categories reject during whole-manifest admission.

## Surfaces, witnesses and acceptance

Expected surfaces: bgfx device/header/journal helper, narrow OriginalGpuEdge
capability integration, new isolated `test_bgfx_transaction.cpp`, CMake and
contract docs, and generated-only cross-stage shader fixtures (not installed
as shipping shader families, compiled under a separate generated output root;
the shipping shader-family closure remains exact). The fixture device uses that
separate owned root with byte-identical generated video binary/manifest/layout
copies for baseline/present controls; relative-path admission is unchanged.
Never alter the unrelated renderer diagnostic. New aggregate/
value members explicitly initialize; no padding assumptions. No second native
reservation implementation or private checkpoint container in the edge.

Two-generation CPU metadata and physical Vulkan tests prove complete commit,
exact source view/clear/draw order, multiple textured/uniform/indexed draws,
prior accepted pixels unchanged after faults at every later staged boundary,
native provider identity, byte/init/view/resource baseline, allocation-free
replay and clean retry emits once. Cover target LOAD, selected clears, multiple
passes, later stale/wrong providers, COW versions, shader/program retirement,
FBO/candidate destruction, window resize/query failure/suspension, final present
and wrong-mode routes. Include all command/resource/byte/view bounds+1,
checkpoint/frozen-copy/native-wrapper/reservation failures, diagnostic exception,
partial abort, duplicate finish, owner removal and ordinary behavior controls.
Accepted-target pixels are inspected only outside the completed attempt;
readback cannot repair a failed batch.
Include shared-stage identical/conflicting aliases, distinct reflected sampler
aliases, equal/conflicting uniform payloads, signed-coordinate maximum/bound+1,
checked span/count/alignment rejection, removed/replaced captured owners,
unfinished/duplicate completion and mutation attempted after final present.
Each rejection proves unchanged CPU shadows, receipt, native view/frame state
and accepted pixels before clean retry; no gate relies on exit code alone.

Focused commands before production:

`cmake --build build/<preset> --target renderer_bgfx_transaction_tests -j4`

`ctest --test-dir build/<preset> -R '^(renderer_bgfx_transaction|renderer_bgfx_transaction_shader_scope|bgfx_shader_family_closure|renderer_bgfx_transaction_resource|renderer_bgfx_submission_reservation|renderer_recording_transaction|original_w3d_first_gpu_edge|original_w3d_gpu_edge_failure)$' --output-on-failure`

Both native toolchains and focused sanitized builds (detect_leaks=0), exact
serial host LSan detect_leaks=1/no UBSan override selecting the three new CPU
witnesses and Recording transaction, all six complete builds and canonical
`-LE 'gpu|lan|retail'` suites are mandatory on frozen source. Physical test
`renderer_bgfx_transaction_gpu` is validation-clean/host graphical on both
native toolchains plus established display-owner/map controls. Require
zero Validation Error/VUID, allocator pairing, immediate residuals and no
emulation substitute. Serial host LAN 4/4 all six, ledger/header/diff and exact
staged review complete acceptance. Commit one coherent frame behavior child:
`delivery: M22 08P0C2D0B2B defer admitted bgfx frame commands`.

## Accepted result

Delivered the complete immutable frame journal, exact captured native leases,
source-stage alias/coordinate preflight, one-final-presentation completion and
allocation-free native invocation/once-only retry consumption. OriginalGpuEdge
forwards exact capability/token ownership; complete source-stage rollback remains
B0. The self-contained generated fixture root preserves production relative-path
admission and byte-identical required video providers, never shipping/install
enumeration. [Final evidence](../../evidence/cnc-generals-zero-hour/milestone_22_slice_08p0c2d0b2b_bgfx_frame_journal.md)
records both native/both sanitizer focused 8/8, strict host LSan 4/4 each, twelve
generated physical controls, all six complete builds/canonicals 274/274 with clean
complete-log audits, established Vulkan 3/3+2/2 and serial LAN 4/4 all six.
Frozen hashes, ledger/header/diff and exact-owned review pass. The unrelated
renderer diagnostic is preserved unstaged. B2/B/D0 aggregates are dependency-next;
no factory, tree scene or retail acceptance is claimed.
