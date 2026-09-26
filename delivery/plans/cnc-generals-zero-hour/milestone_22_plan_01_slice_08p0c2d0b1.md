# M22 plan 01 slice 08P0C2D0B1: candidate/COW bgfx resource lifetime

Status: independently accepted by this slice commit against parent
`9b7701a5a5645a93a9a9ef1bf2f5b430428b5b88`; see
[refrozen evidence](../../evidence/cnc-generals-zero-hour/milestone_22_slice_08p0c2d0b1_bgfx_resource_transaction.md).
Six complete builds/canonical suites 271/271, both sanitizer focused/strict
LSan, physical native/sanitizer resource controls, established Vulkan and all
six serial LAN gates pass. Frame capability and source/factory admission remain
closed; D0B2A is dependency-next.

## Goal, dependency and boundary

After accepted D0A, independently deliver bounded bgfx idle-preparation
resource/byte/marker commit, abort and retry. Frame capability remains false
until D0B2B; SDL/other devices remain unsupported. No frame/view/draw/present,
tree state, source maps/refs, retail input or allocator rewrite. Generated
local work has no authorization change. Parent D0B contains the complete
read-only native audit and resolved render-target/Memory/lifetime boundaries.

## Entry, state and native ownership

Use D0A's exact device/sequence/generation/mode token and explicit finite
budgets: no more than 4096 commands, 4096 checkpoint/candidate slots, 64 MiB
owned bytes, and zero idle views. Count baseline shadow/metadata and immutable
candidate/upload storage, including retained replaced native versions. Reject
overflow, malformed/nested/stale/cross-mode/ordinary-active-pass admission
before publishing a checkpoint or changing the output token.

Copy/retain five resource-slot baselines before admission. Transaction slots
are append-only; abort retains generation-advanced tombstones so candidate
handles cannot alias retries. Fault counters are outside the checkpoint.
CPU buffer bytes/written state, texture initialization, shader reflection,
pipeline parents/cache identity and sampler descriptors restore exactly.
All new aggregate/default members have explicit initialization; no padding
comparison establishes identity. Exceptions poison before diagnostic work.
Commit/abort are no-throw, exact-token and exactly-once.

Full ordinary sampled-texture mip uploads retain bounded source shadow bytes
before native upload so the transaction can COW later. Each changed sampled
texture has an owned native candidate; copy unchanged known mip bytes and
apply new full-mip uploads only to candidates. Feed every bgfx Memory directly
to its owned native consumer; never retain an unconsumed Memory requiring the
private release API. Complete CPU allocations, native creates/uploads and
publications before replacing the accepted record. Preserve opaque accepted
handle identity and known mip contents on successful COW; abort leaves its
native provider/bytes untouched. An unknown/uninitialized mip stays declared
unknown, not fabricated accepted data. Multiple replacement candidates and
queued cancellation retire exactly once and count toward admission limits.

Render-target create/resize uses a fresh candidate handle; accepted descriptor
and GPU-written pixels cannot be reconstructed from an upload shadow.
Transaction upload to render targets fails closed before mutation, with an
explicit negative control; ordinary uploads are unchanged. The required
TextureLoader/shroud sampled non-target route remains supported. Fresh target
create/destroy and existing target retirement are allowed without a pass.
Preserve accepted native resources until commit; abort releases only candidate
native textures/shaders/programs, in safe dependency order. Candidate wrappers
use RAII through allocation/publication failure, including texture and program
slot insertion. No resource leaks from a throwing vector insertion.

Finish itself makes no native call: public bgfx destroy can grow its native
command buffer. Admission reserves a bounded retirement queue; commit/abort
transfer replaced/canceled native ownership into it with nonallocating CPU
operations. A later ordinary nontransaction boundary drains public destruction,
and ordinary wait/shutdown completes it. Pending retirements remain explicitly
owned/count toward admission, never alias a retry and retire exactly once.
No hidden frame advancement or unbounded native residual obtains this guarantee.
Pinned shader/program creates can deduplicate native indices and increment
their reference counts. Count each successful create as one distinct native
ownership unit, not a unique handle. Commit consumes exactly one ownership
unit per live slot, queues every excess/canceled unit once, and abort queues
only candidate units. Cover repeated alias creation, replacement, abort,
drain and shutdown; accepted aliases must survive without ref leaks.

No frame advancement, view mutation, native target touch or window ownership
change is permitted during idle admission, commit or abort. Reject pass/end,
viewport/clear/draw/present, claim/release/wait/readback routes while live before
mutation. Destruction cancels the journal before ordinary completion/shutdown.
Do not force a reset/frame to repair a failed attempt. Ordinary behavior and
public ABI remain unchanged outside explicit capability use.

## Surfaces and validation

Expected owned surfaces: bgfx device/header, bounded resource-journal helper
used by the real owner, a new isolated `test_bgfx_transaction_resource.cpp`,
CMake registration, contract docs and narrow edge idle-capability admission.
Do not edit the unrelated renderer diagnostic or pinned runtime in this child.

Pure CPU tests exercise the real bounded resource-journal metadata/token/
generation/fault owner without opening Vulkan; physical tests exercise actual
native COW and publication. Across two generations prove all five resources,
baseline create/upload/retire rollback, shader/program parent and cache
identity, each mip/format/padded row, repeated COW, preserved prior sampled
pixels and target pixels, retry, zero native/public residual after completion.
Cover malformed/bounds+1/nested/stale/duplicate finish, wrong mode/side-route,
partial allocation/create/upload/publication and diagnostic allocation faults,
candidate generation non-aliasing, target-upload rejection, target fresh-resize
identity, candidate queued destruction and owner removal/destruction. Physical
readback is only outside the completed journal, never an abort shortcut.

Focused commands, persisted before production:

`cmake --build build/<preset> --target renderer_bgfx_transaction_resource_tests -j4`

`ctest --test-dir build/<preset> -R '^(renderer_bgfx_transaction_resource|renderer_recording_transaction|original_w3d_first_gpu_edge)$' --output-on-failure`

Run GCC/Clang native and sanitized focused (`ASAN_OPTIONS=detect_leaks=0`).
Exact serial host LSan: `ASAN_OPTIONS=detect_leaks=1 ctest --test-dir
build/<sanitized-preset> -R '^(renderer_bgfx_transaction_resource|renderer_recording_transaction)$'
--output-on-failure`, no UBSan override. New generated physical test is
`renderer_bgfx_transaction_resource_gpu`, wrapped by `tools/run_validation_clean.py`;
run GCC/Clang native via host graphical escalation with Khronos validation,
plus established display-owner/map controls. Do not substitute software Vulkan.
When the preset does not register opt-in GPU tests, run the exact physical
entry directly: `VK_INSTANCE_LAYERS=VK_LAYER_KHRONOS_validation python3
tools/run_validation_clean.py build/<preset>/renderer_bgfx_transaction_resource_tests --gpu`.
An empty CTest selection is not evidence. Run this generated physical entry
under both sanitized toolchains with detect_leaks=0 as well, serially.
All six complete builds and canonical `-LE 'gpu|lan|retail'` suites (sanitizer
detect_leaks=0), serial host LAN 4/4 each, ledger/header-neutrality/diff and
exact staged review are required on frozen source. Isolate heavy sanitizer
workloads. Record final evidence and commit one independently useful child:
`delivery: M22 08P0C2D0B1 own bgfx candidate resource publication`.
