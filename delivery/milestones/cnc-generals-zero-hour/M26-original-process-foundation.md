# Milestone M26: original process allocation and ABI foundation

## Objective

Make original allocation/string consumers executable on Linux before the coupled engine port.

## User/System Outcome

Small original-consumer targets safely allocate, exchange and free original objects across target and process lifetime boundaries, without initializing the game.

## Scope

Port the permitted WinMain-owned bootstrap services: synchronization, logging, allocator and Version lifetime. Port actual GameMemory/MemoryPool, original strings/UnicodeString and required ABI/PCH/CRT consumers and C++ allocation overloads. Preserve M19's completed ten-provider evidence. Establish the evolving source dependency ledger and target-specific identity checks used by every remaining provider.

## Explicit Exclusions

No full engine or registry linkage, private assets, physical devices, Windows CWD mutation, protection or window creation. Original file/configuration consumers are M27; CPU resource adapters M28; integrated startup and final global teardown M20. Do not rebuild already-correct support components.

## Source Requirements

[Runtime reconciliation](../../../docs/zero-hour-runtime-closure-reconciliation.md) RC-004, RC-006, RC-007 and RC-008; RC-001/002 original ABI obligations allocated here. [Migration supplement](../../../docs/zero-hour-source-engine-migration.md) SE-001/002/010 and [base plan](../../../docs/zero-hour-linux-port-plan.md) §§5–6/10 govern preservation. Revisions and commits are in status.yaml.

## Preconditions

M19 provides bounded original support and source inventory; it does not prove GameMemory or UnicodeString portability. Toolchains and existing asset-free harnesses are available. The deliverables below are not prerequisites to begin; no M20 or retail acceptance is required.

- `PRE-001/PRE-002/PRE-004/PRE-028` — repository, installed tools, four presets and accepted M19 bounded original providers are available. This milestone produces `PRE-035`; original allocator/bootstrap and checked ledger implementation are outputs, not entry gates.

## Readiness checks

- Run `command -v cmake ninja gcc g++ clang clang++ pkg-config glslc` and `cmake --list-presets` from the repository root; all tools and four native presets must resolve.
- Inspect status.yaml and M19 acceptance for its ten-provider evidence, then the [prerequisite manifest](../../readiness/cnc-generals-zero-hour/prerequisite-manifest.md) for PRE-028/035. Do not require later INI/GameLogic/engine targets or retail/device access.
- Inspect the deep-review seed ledger and the single original `setFPMode` definition in GameLogic.cpp; plan bounded extraction/characterization here and actual INI/map caller integration in M27. Source identity and ledger check commands are implementation deliverables, not assumed available tools.

## Functional Requirements

Own services before original consumers and release them after consumers/workers, including pre-main allocation and static destruction. Preserve the real GameMemory new/delete linkage verification and conditional shutdown behavior; characterize pointer width, alignment and overload pairing rather than disable checks. Actual original string operations prove UTF-16 semantics; foundation helper tests alone do not count.

Move the one original setFPMode definition from GameLogic into lower shared support through narrow behavior-preserving extraction: INI::load and MapCache::updateCache must not force the whole simulation target into earlier providers. Characterize actual nearest-rounding/24-bit precision behavior rather than the stale CHOP comment and supply the Linux equivalent. M26 tests this actual original definition with bounded source-ABI consumers and preserves one production definition; M27 adds actual INI/map-caller integration using it. M26 does not require those later caller targets to pass.

Maintain a checked-in ledger of source path/symbol, consumer, lifecycle/configuration branches, provider/target, callback/vtable/global edges, logical assets/writes, tests and evidence grade. Seed from the deep review, mark inspected versus compile/link/runtime-proven edges, invalidate affected checks when consumed sources or registry lists change, and fail for required providers without owners. Routine discoveries extend owned scope; actual product/backend incompatibility follows base-plan escalation.

## Architecture / Security Constraints

Use real original class/method bodies, narrow characterized portability changes and OS adapters; no reduced classes, duplicate globals, fake success or linkage suppression. Preserve x86-64/fixed-width/UTF-16LE and allocator semantics. Keep all writes in isolated XDG roots and retail read-only. Do not extend the existing support-target NDEBUG workaround to new original-runtime targets.

## Interfaces and Compatibility

Preserve original consumer interfaces and cross-target allocation/free compatibility. Existing native support APIs and tests remain. Target-specific compile commands and live contributing symbols must prove identity, excluding discarded sections/basename-only matches. No save format or game rule changes.

## Acceptance Criteria

- [ ] Original allocation/string consumer targets compile, link and run under all four presets, independently of full engine initialization.
- [ ] Pre-main allocation, cross-target new/delete, required overloads, alignment, boundary UTF-16 operations and static destruction preserve characterized semantics; the real linkage check runs.
- [ ] Bootstrap failures preserve diagnostics and release initialized services exactly once; joined workers and explicit allocator/resource counts establish lifetime evidence.
- [ ] Removing a required provider fails a real compile/link/runtime gate; active tests and conditional-source review reject proxy/reduced implementations.
- [ ] Ledger entries identify each exercised edge, evidence grade and remaining M27/M28/M20 owner; source/registry drift and ownerless required edges fail checks.
- [ ] Existing asset-free regressions pass and no device or private asset is required.

## Required Validation

GCC/Clang Debug/Release original-consumer tests plus full asset-free CTest; focused ASan/UBSan with exact options disclosed and explicit ownership/live-count tests, not a leak-freedom claim from disabled leak detection. Exercise construction failure, static initialization/destruction and allocation/free in different original targets. Record source, compile/link and runtime evidence separately. Repository checks validate ledger freshness and negative controls. No complete engine build is required at this boundary.

## Known Risks / Deferred Work

Original headers may expose further tightly coupled consumers; isolate only real provider seams, never add stubs to pretend the engine links. M20 owns integrated global/service ordering and all remaining coupled runtime closure. New governing plan 01 is created only during delivery; no slices are prescribed here.
