# N2: Original engine Linux foundation

This governs one new N2 companion transaction, executed in this chat without
agents. It does not reopen N1 or consume archived implementation.

## Outcome and delivery context

Actual original Zero Hour startup and GameLogic run on Linux, read explicit
user-supplied roots without asset writes, and repeat deterministic headless
simulation with clean ownership under GCC and Clang. Full N2 completion requires
original GameEngine/GameLogic reachability, not merely core-service tests.

Goal: `workflow/delivery/state.yaml`, fixed N0–N7, autonomous; packet
`linux-upstream-v2`, semantic revision
`6ff07a07e808ae8fe0c3699c09e3739d11352cdf9bb7d79b47f54a3e984a6923`.
Source transaction d0483ca9; planning f92a3e5c; N2 transaction parent
`11295c4feb535f97978b8335632440fccae82794`. N0 accepted; N1 is independently
accepted but is not a headless prerequisite.

## Governing contracts

- `delivery/milestones/cnc-generals-zero-hour/linux-upstream-v2/N2-original-engine-linux-foundation.md`
- `docs/zero-hour-linux-port-plan.md`: R03/R04/R07; unchanged gameplay and file encodings.
- `AGENTS.md`, `docs/original-engine-formats.md`, readiness PRE01/PRE04/PRE05.
- Stock dependencies only; no asset modifications, raw retail selectors/logs,
  Windows compiler/ABI or ARM64 requirements, or imported archive code.

## Current source findings and decisions

Original PreRTS unconditionally imports ATL/Win32 and legacy STL. Include spelling
also differs from actual Linux paths (CRC/crc, Trig/trig, Basetype/BaseType).
Correct retained consumers' includes; do not emulate a Windows SDK or globally
silence semantic errors. Build explicit source targets from original project
graphs and inspect coupled compile surfaces before validating frozen batches.

GameMemory overrides ordinary global new/delete and assumes four-byte block
alignment. Library temporary/nothrow paths and process-static strings complicate
that boundary. General C++ allocation will use the standard allocator; retained
explicit game pools must preserve their own acquisition/release contract and
provide native alignment, constructor rollback and repeat-owner checks. Remove
the global override rather than introducing a new mixed-owner library boundary.
Strings can own standard storage independently of engine pool shutdown. This
changes internal allocation, not gameplay or persisted representation.

Original GameEngine init contains an obsolete patch-era DeleteFile against game
data. Remove this mutation entirely from Linux startup; retail mounts remain
read-only. Native services use POSIX/standard C++, not registry installation paths.
Headless presentation/audio boundaries may omit output, but never replace
GameLogic, object updates, module behavior, RNG, map parsing or timing semantics.

BIG entry offsets/counts/sizes are explicit big-endian 32-bit values; the original
header-size read is not sufficient authority for that field's interpretation.
CSF reads Windows native Int and WideChar; decode fixed file widths separately
from Linux wchar_t. Reader/writer source traceability precedes implementation.

## Scope and slices

| Slice | Plan | Observable outcome | Dependencies | Status | Commit | Evidence |
| --- | --- | --- | --- | --- | --- | --- |
| 01 | [Core owner process](milestone_02_plan_01_slice_01.md) | Original core values, strings, memory and RNG start/use/retire safely on Linux | N0 | completed | `delivery: port original core ownership to Linux` | `evidence/qa/N2-original-core.md` |
| 02 | [Supplied data access](milestone_02_plan_01_slice_02.md) | Original consumers read rooted BIG/INI/CSF with bounded errors and no asset writes | 01 | completed | `delivery: port rooted original data consumers to Linux` | `evidence/qa/N2-original-data.md` |
| 03 | [Original headless runtime](milestone_02_plan_01_slice_03.md) | Actual original startup and simulation run/repeat with compiler-matching checkpoints | 01,02 | in_progress | | |

Three independently testable behavior slices, not per-assert or per-file plans.
Renderer/UI/media/LAN/full match completion remain later milestones. No toy
simulation, cache-derived asset distribution, legacy ABI promises or framework
patches are in scope.

## Cross-slice and completion gate

Preserve fixed-width readers/writers and independent client/audio/logic RNG.
Explicit data roots override CWD; precedence follows verified original owners.
Asset storage is read-only, caches disposable with corruption/unwritable fallback.
Process-global standard storage is distinguished from runtime pool ownership.
Failures unwind unpublished candidates and permit corrected same-process retry.

Canonical CMake/Ninja/CTest gain original-source targets without breaking N1.
Each slice runs focused positive, malformed, boundary and retry tests under normal,
GCC ASan/UBSan and Clang ASan/UBSan builds; no leak suppression. At completion,
run original fixtures under both compilers with byte-identical checkpoints,
repeated initialization/teardown, source reachability and read-only retail
integrity/diagnostic evidence. Run the complete applicable suite once on frozen
source. Mark N2 complete only with evidence for every milestone criterion.

## Recovery and execution

Commit each completed slice with plan, code and evidence; revert independently
without touching user assets or recovery archives. Investigation may refine
settled engineering details in these existing plans, not create tiny replacement
plans. A genuine unresolved authority/external condition is recorded precisely;
ordinary port/build/fixture work remains inside this transaction.

N1 output is committed at 11295c4f and untouched upstream. N2 planning is
persisted before any original-runtime production edit. PRE04 completeness is
still pending at the actual retail boundary, not inferred from the symlink.

Slice02 is accepted under normal/GCC/Clang checks, 18/18 each. Its read-only
supplied-data probe reached stage4/mask63 with unchanged complete input snapshots.
This establishes basic indexing/families/text, not full PRE04 scenario or runtime
acceptance. The existing slice03 continues actual original startup; see its
explicit carry-forward service/encoding gates and the slice02 evidence limits.
