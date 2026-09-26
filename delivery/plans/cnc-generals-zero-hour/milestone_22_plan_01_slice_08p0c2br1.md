# M22 plan 01 slice 08P0C2BR1: ordinary nothrow allocator pairing

Status: approved architecture correction; plan-only checkpoint before implementation.
Parent checkpoint: `6dea8647699c36a77eff92c6304fd00c1f32c1b7`.

## Goal, scope and dependency

After accepted B0 and before C2B acceptance, ensure non-aligned ordinary
`new(size, std::nothrow)` and `new[](size, std::nothrow)` allocate through the
same existing original DMA as ordinary/sized delete. Define their matching
nothrow placement deletes for constructor-failure cleanup. Linux-only correction;
no aligned overload, DMA/pool algorithm, concurrency, runtime allocator redesign,
Windows/serialized ABI, shader/validation suppression or C2B executable change.
Generated local allocation has no authorization gate. Existing C2B source/test
edits remain unstaged throughout this independently reviewed corrective owner.

Read-only classification: both sanitizer C2B physical routes fail during bgfx
initialization, before physical shroud creation. Clang identifies SPIRV-Tools
`checkLayout` stable-sort temporary cleanup in the Vulkan validation layer,
then original ordinary/sized delete and GameMemory.cpp:1794. The independent
unchanged GCC-sanitized tree-program physical control reproduces this pairing
failure (GameMemory.cpp:989). The executable lacks ordinary nothrow new/new[]
definitions; the library allocation resolves to the runtime overload while its
cleanup resolves to original delete. No VUID, timeout or pixel assertion caused
the failure. Private generated logs stay local; durable evidence records only
category and public source locations. Do not waive either required physical gate.

## Behavior, state and error contract

Only the four non-aligned ordinary nothrow overloads are added. Allocation
delegates to existing ordinary scalar/array new and catches every allocation
exception, returning null without throwing; representability is checked before
narrowing to the existing Int-size DMA (including private header overhead).
Zero-size behavior remains the existing ordinary allocator behavior. Matching
placement deletes delegate to corresponding ordinary delete, including null.
No new pool/global state or fallback malloc owner is introduced. An admitted
allocation acquires exactly one existing pool/raw unit; ordinary, sized or
matching placement deletion releases it once. Failed allocation preserves live
pool/raw counts and the next successful retry works normally. Aligned ordinary
and aligned nothrow allocation/deallocation remain entirely C++-runtime owned.

## Surfaces and focused controls

Expected production surface: only GameMemory.cpp overload block. Supporting
surfaces: existing original allocator test/provider, its narrowly scoped test
link option if needed, dependency ledger, plan/index/discovery and adjacent
evidence. No C2B source/test/ledger hunk is included in the corrective commit.

Cover scalar/array, zero-size and raw 4097-byte allocation, cross-target allocate
and ordinary/sized/placement free with exact raw/pool count restoration; throwing
constructors invoke matching nothrow placement delete. Reject unrepresentable
size before DMA mutation. Prove actual allocation-failure null semantics with
a test-executable-local one-shot malloc wrapper returning null for the next
raw request, then clean retry; no production fault selector. The wrapper remains
in the allocator test only and forwards all ordinary calls to the real allocator.
Retain existing aligned foreign-style and cross-target controls, including
aligned nothrow; counts must not enter original DMA. Inspect defined executable
symbols for all four added overloads and confirm no aligned definitions appear.

Focused build: `cmake --build build/<preset> --target original_process_allocator_tests original_w3d_tree_program_tests -j4`.
CPU command on GCC/Clang native and sanitizer: `ASAN_OPTIONS=detect_leaks=0 ctest
--test-dir build/<preset> -R '^original_process_(allocator|pool_config|identity_allocator)$'
--output-on-failure`. Exact host strict LSan uses `ASAN_OPTIONS=detect_leaks=1`
with the same selection, no UBSan override. Audit complete logs, not exit alone.
Independent host physical command on both sanitizer toolchains:
`ASAN_OPTIONS=detect_leaks=0 python3 tools/run_validation_clean.py
build/<preset>/original_w3d_tree_program_tests --gpu`.
Both must pass with validation enabled, no sanitizer findings and unchanged
source program/pixel assertions. No validation-layer disabling or allocator
suppression is permitted.

Before allocator commit require six complete builds, six serial canonical
nonretail suites `-LE 'gpu|lan|retail'`, established native Vulkan controls,
serial LAN4/4 all six, ledger/header/symbol/diff review. These runs contain the
frozen unstaged C2B composition and do not by themselves accept or commit C2B.
After the allocator commit C2B resumes with refreshed affected focused/physical
gates and its remaining acceptance; no source change is inferred authorized.

## Acceptance and commit boundary

All pairing/count/failure/retry/aligned controls and both independent sanitizer
validation-enabled physical controls pass. No mixed allocator ownership remains
on this ordinary nothrow route. Plan-only checkpoint commits only plan/index/
discovery before production. The later implementation commit exact-stages only
this allocator owner and evidence: `delivery: M22 08P0C2BR1 pair ordinary nothrow allocation`.
Preserve C2B executable changes and unrelated renderer diagnostic unstaged.
