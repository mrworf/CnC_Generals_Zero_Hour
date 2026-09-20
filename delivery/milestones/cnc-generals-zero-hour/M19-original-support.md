# Milestone M19: original support closure and source identity

## Objective

Required original support code compiles and executes with auditable compiler/link provenance.

## User/System Outcome

Required original support code compiles and executes with auditable compiler/link provenance.

## Scope

Classify every legacy inventory source as production-compiled, platform-replaced, tool-only or excluded with provider/rationale. Compile original compression, WWMath/WWLib/WWSaveLoad, strings, pools and chunk I/O. Repair ATL/PCH, pointer/typedef, CRT/assembly, UTF-16 and floating-point portability at original call sites; inventory proprietary consumers and approved adapters.

## Explicit Exclusions

GameEngine factories and complete production linkage follow later. Full engine compilation and fixing its existing ATL errors are deliverables, not entry prerequisites. Preserve valid component implementations/tests. No ARM64, base Generals executable/tools, retail redistribution or gameplay rewrite.

## Source Requirements

[Migration supplement](../../../docs/zero-hour-source-engine-migration.md): SE-001, SE-002; SE-010 assurance/evidence constraints. [Base port plan](../../../docs/zero-hour-linux-port-plan.md) §§1–6 and §10 govern preservation, stack, formats and validation. Revisions/commits are in status.yaml.

## Preconditions

Direct providers: M0, M1. Consume evidenced contracts, not historical title/status alone. Historical providers supply qualified component evidence; this contract provides the missing original-source assurance. Its unfinished deliverables are not entry prerequisites. Readiness separately audits external inputs.

- `PRE-001/PRE-002/PRE-004/PRE-005` — repository sources, installed toolchain and qualified M0/M1 component foundations are available. PRE-028 is produced here; original-support compile/link repair is not an entry prerequisite.

## Readiness checks

- Run `cmake --list-presets` from the repository root: all four native presets must resolve. Inspect status.yaml and linked provider acceptance for the direct dependencies; require the original-source grade for new providers, not only historical component status.
- Inspect [prerequisite manifest](../../readiness/cnc-generals-zero-hour/prerequisite-manifest.md) for the IDs above and their current verification. Use its retail/session checks only at the named validation boundary. No unfinished feature acceptance command is an entry requirement.
- Run `command -v cmake ninja gcc g++ clang clang++ pkg-config glslc` and `cmake --preset linux-gcc-debug`; tools/modules and baseline configure must succeed without fetching. Existing ATL failure belongs to this source-port work.

Produces `PRE-028` (M19 original support compile/link and runtime identity); inspect this contract's source-owned positive and negative acceptance evidence before downstream consumption.

## Functional Requirements

A project-owned harness exercises actual original support implementations. Actual compiler commands and final-link records identify objects executed. Mark bootstrap and toy simulation fixture-only; maintain an explicitly incomplete production-closure report until remaining providers land.

## Architecture / Security Constraints

Original engine owns authoritative state/lifecycle; native adapters own OS/device/library edges. Preserve original allocators except narrow characterized portability changes. Retail roots/symlinks are read-only; never commit retail bytes/hashes/private paths. Isolate XDG writes. Never suppress unresolved symbols or use fake success-returning production subsystems.

## Interfaces and Compatibility

Preserve original engine interfaces and fixed-width, char16_t/UTF-16LE, bounded serialization and floating-point rules; never -fshort-wchar. Incremental source targets coexist with fixture-only scaffolding, never parallel authoritative states. Rollback uses reviewed commits without overwriting user saves/configuration or retail. Keep valid native component APIs unless source integration proves a narrow correction necessary.

## Acceptance Criteria

- [ ] The stated outcome and all scoped consumers execute original implementations with recorded source-owned transitions or outputs.
- [ ] A project-owned harness exercises actual original support implementations. Actual compiler commands and final-link records identify objects executed. Mark bootstrap and toy simulation fixture-only; maintain an explicitly incomplete production-closure report until remaining providers land.
- [ ] Negative controls fail against the actual implementation; no toy/placeholder fallback satisfies acceptance.
- [ ] Existing component tests pass; evidence names its grade: component/fixture, original-source compile/link, original-source integration or retail runtime.

## Required Validation

Boundary/rounding/UTF-16 and malformed/endian codecs; all four compiler presets; sanitizers on original allocator teardown. Removing a required original provider must fail compile/link identity gates.

Build applicable targets with linux-gcc-debug, linux-clang-debug, linux-gcc-release and linux-clang-release. Run full asset-free CTest regression at milestone acceptance plus relevant ASan/UBSan. Record exact commands/context and map formerly unproven historical obligations to real-source tests. Retail/GPU checks use explicit separate gates; defaults remain private-data/device independent. Only permitted logical/derived evidence is committed.

## Known Risks / Deferred Work

GameEngine factories and complete production linkage follow later. Full engine compilation and fixing its existing ATL errors are deliverables, not entry prerequisites. Component tests do not establish original execution. Unexpected required asset/backend limits follow base-plan §9 evidence-gated escalation, never reduced silent coverage.
