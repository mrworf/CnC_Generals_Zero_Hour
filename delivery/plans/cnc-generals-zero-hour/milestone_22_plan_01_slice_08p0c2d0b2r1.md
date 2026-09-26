# M22 plan 01 slice 08P0C2D0B2R1: generated presentation service ownership

## Goal, dependency and boundary

After accepted D0B2A (`59e8bbafc088f330e2d43ad395e2e072622cab6c`), make the
generated original presentation fixture follow the shipping FileSystem-before-
GlobalData precondition without invoking a null member. This is an independent
corrective owner before R2 and the deferred B2B journal. No renderer/native
submission, source gameplay/CRC algorithm, retail, original asset or allocator
change. Authorization is not applicable to generated local fixture services.

Read-only evidence: GCC CTest 161 and Clang CTest 174 report unchanged
GlobalData.cpp:1028/1039; Clang additionally reports FileSystem.cpp:175.
The fixture constructs GlobalData without TheFileSystem. Shipping GameEngine
creates FileSystem at 341, local/archive at 380/391 and GlobalData at 401.
FileSystem::openFile intentionally permits absent local/archive providers and
returns null for missing files; do not turn missing generated files into an
original-service omission or invent an empty-CRC production fallback.

## Complete service lifetime

After initMemoryManager, install an explicitly owned real FileSystem and the
real PosixLocalFileSystem rooted only in the existing mkdtemp generated directory.
GlobalData's two optional script reads use this generated root; an absent
archive provider is permitted by FileSystem::openFile and remains absent.
No private assets, caller CWD discovery or
archive mounting. Admit the service scope before publishing pointers: reject
overlapping borrowed ownership, preserve the prior globals, initialize the
local service before publishing/constructing GlobalData and use a no-throw
reverse cleanup. Missing-service negative coverage must reject before any
GlobalData constructor call, avoiding undefined null-member invocation.

The generated GlobalData construction entry checks the published FileSystem
precondition before construction. Keep the shipping construction and original
CRC/file behavior unchanged. Owned services outlive GlobalData and all fixture
consumers; release GlobalData before restoring/removing services, including
exception paths. Assertions prove prior singleton identities are unchanged on
rejected admission and restored on cleanup. Re-entry/retry must not retain a
borrowed pointer, file, cache, GlobalData original-owner chain or pool allocation.
Keep the existing two-generation presentation behavior and resource assertions.

## Surfaces and witnesses

Expected surfaces: tests/original_rendering/test_w3d_presentation.cpp, its
generated-service helper only if independently needed, dependency ledger if
provider ownership changes, and adjacent evidence/index. Use existing linked
FileSystem/Posix providers; no substitute FileSystem stub, new source global,
class/vtable/serialized state, service bypass or FileSystem null-this tolerance.
Initialize every new value member explicitly. Preserve the unrelated renderer
diagnostic and all production source-order behavior.

Positive controls establish the real generated service, construct/use/destroy
GlobalData and complete the existing presentation route twice. Negative controls
cover absent service before construction, overlapping owned/borrowed service
admission, missing generated files through a valid FileSystem, cleanup on
fixture failure and clean retry. Assert exact global identity and zero owned
service/file/pool residual before final memory-manager teardown.

Focused build: `cmake --build build/<preset> --target original_w3d_presentation_tests -j4`.
Focused test: `ASAN_OPTIONS=detect_leaks=0 ctest --test-dir build/<preset>
-R '^(original_w3d_presentation|original_w3d_presentation_identity|original_w3d_presentation_provider_removal)$'
--output-on-failure` on both native and both sanitizer configurations.
Audit complete selected output for runtime-error/Sanitizer categories rather
than successful exit alone. Exact serial host LSan uses
`ASAN_OPTIONS=detect_leaks=1` and the same selected CPU fixture/identity tests;
no UBSan override or undefined-behavior negative control.

Before commit require six complete builds and canonical nonretail suites
`-LE 'gpu|lan|retail'`, exact established host Vulkan controls, serial LAN 4/4
all six, ledger/header/diff review. R2's explicitly tracked unrelated texture
enum diagnostics remain qualified until R2, not globally waived or suppressed.
After R2 a clean complete matrix is mandatory before B2B opens. Commit only this
service lifetime owner: `delivery: M22 08P0C2D0B2R1 own presentation fixture services`.

## Planning checkpoint

Implementation-ready plan only; no production/test edits precede this durable
checkpoint. This owner cannot alter accepted A or open B2B. R2 owns the separate
defined filter/profile representation and complete pre-mutation validation.
