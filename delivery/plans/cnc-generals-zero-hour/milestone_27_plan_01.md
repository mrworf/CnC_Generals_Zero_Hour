# M27: Original data codecs and map metadata providers

This plan governs exactly one milestone transaction.

## Outcome

Asset-free x86-64 Linux executables exercise source-owned Zero Hour data behavior through the existing native VFS: logical file resolution, a bounded shared INI/CSF parser core, fixed-width transfer and chunk primitives, independent random streams, and map metadata/cache-path operations. Read roots remain immutable and generated metadata is confined to XDG cache storage.

## Delivery-goal context

- Goal status: `workflow/delivery/state.yaml`
- Product ID: `cnc-generals-zero-hour`
- Milestone: `delivery/milestones/cnc-generals-zero-hour/M27-original-data-providers.md`
- Active immutable packet fingerprint: `sha256:0d5f1fef5dcec3bbbbb26656c5aaf591d653e2454501b51cfb09059fbb6acd7a`
- Transaction-start `HEAD`: `a057410`
- Required predecessors: accepted M26/PRE-035 and M4/PRE-009
- Current milestone: M27

## Governing authority

- `docs/zero-hour-linux-port-plan.md`
- `docs/zero-hour-source-engine-migration.md` (`SE-002`, `SE-003`, `SE-007`, `SE-010`)
- `docs/zero-hour-runtime-closure-reconciliation.md` (`RC-002`, `RC-004`, `RC-006`-`RC-009`)
- M27 milestone contract and current readiness prerequisite manifest
- No repository-local `AGENTS.md` exists; CMake presets and established asset-free CTest conventions govern.

## Current-state findings and decisions

M4 already supplies a hardened read-only loose/BIG VFS and deterministic precedence, while M26 supplies the actual allocator, strings, synchronization, and sole shared `setFPMode`. The original `INI.cpp` has a deliberately complete static dispatch table whose callbacks span later subsystems, and `MapUtil.cpp` contains both independently reusable metadata operations and registry-dependent object/cache behavior. Compiling either file whole at this stage would require fake callbacks or pull M20 forward.

Accordingly, M27 will use narrow source-owned extractions in the original source tree for the parser core and independently reusable MapUtil operations. Extraction provenance and fingerprints will be checked against the authoritative methods, and production-facing declarations will be shared with later integration. The full `INI.cpp` table, `ParseObjectDataChunk`, nonempty object classification, `INIMapCache`, localized `map.str`, and registry/post-load callbacks remain intact and explicitly M20-owned. Original `RandomValue.cpp` is compiled as an actual translation unit after narrow portability include corrections; fixed-width codec/chunk behavior is implemented in extracted original-source units rather than a parallel synthetic authority.

## Slice index

| Slice | Plan | Outcome | Dependencies | Status | Commit | Evidence |
|---|---|---|---|---|---|---|
| 01 | [Logical files, INI and CSF](milestone_27_plan_01_slice_01.md) | Original-source data facade consumes native VFS with exact precedence, two-pass INI ordering, typed callbacks, CSF UTF-16LE, and bounded failures. | M26, M4 | complete | `af04a8f8beffc620274f3f8524d7057fa3352363` | 2/2 focused tests in all four presets |
| 02 | [Transfer, chunks and random streams](milestone_27_plan_01_slice_02.md) | Source-owned fixed-width codecs/chunks round-trip bounded data and actual original random streams remain independent. | slice 01 | complete | `2fb1a7dc1a2919be7ed43c0fee25e3f1e0fd353d` | 4/4 cumulative focused tests in all four presets |
| 03 | [Map metadata, XDG cache and source gates](milestone_27_plan_01_slice_03.md) | Independent map metadata/cache operations handle lifecycle and precedence safely, with checked identity/ledger evidence. | slices 01-02 | complete | recorded by the slice commit | [M27 QA evidence](../../../evidence/qa/cnc-generals-zero-hour/m27-original-data-providers-2026-09-20.md) |

## Cross-slice constraints

- Compatibility: native x86-64 Linux only; `WideChar` remains 16-bit without `-fshort-wchar`; no Windows binary compatibility claim.
- Security: all lengths/counts/path inputs are bounded; logical traversal is rejected; read roots are never written; cache replacement is atomic and XDG-confined.
- Authority: tests invoke code compiled from original-tree providers; no fake production callbacks, reduced INI table, surrogate registry, or dead-strip workaround.
- Lifecycle: missing or malformed input fails deterministically; stale cache rebuild is explicit; failed writes leave prior cache intact.
- Privacy: owned synthetic fixtures only. No retail paths, bytes, hashes, metadata, or checksums enter commits or evidence.
- Deferred integration: M20 retains every callback/registry/object/localization edge that cannot run independently without full engine initialization.

## Milestone completion gate

- Configure/build and run all focused M27 tests in GCC/Clang Debug/Release.
- Run the canonical asset-free CTest suite once after slice 03 stabilizes.
- Run focused Clang ASan/UBSan and record exact options and resource ownership counts.
- Verify compile-command, link-map/live-symbol, runtime-witness, source-drift, provider-removal, and ownerless-edge negative controls separately.
- Reconcile all M27 acceptance criteria in committed QA evidence without claiming M20 registry-dependent cache acceptance.

## Rollback and recovery

Each slice is independently revertible. Slice 03 removes metadata/cache integration and assurance while retaining codecs; slice 02 removes transfer/chunk/random behavior while retaining logical config; slice 01 removes the M27 target. Generated build and fixture directories are disposable.

## Execution notes

Planning and all slice files were created before production edits. The transaction started from clean `HEAD` `a057410`.

Slice 01 binds three named original-source extractions to the accepted M4 VFS and M26 FPU provider. Focused runtime and source-identity tests pass 2/2 in GCC/Clang Debug/Release; the checked dependency ledger and unchanged 3,293-path classification both pass. No manifest-listed legacy source was reclassified because this slice compiles new extraction units, not the coupled legacy translation units.

Slice 02 compiles the actual `RandomValue.cpp` and its actual CRC provider after narrow case/include and non-MSVC portable-CRC corrections. It removes an unnecessary `GameLogic.h` dependency in favor of the three direct random interfaces. Named Xfer and DataChunk core extractions provide bounded, fixed-width little-endian primitives without claiming M24 snapshot compatibility. Cumulative focused runtime and identity tests pass 4/4 in all four presets; only the two manifest-listed original units actually compiled by this slice are promoted in classification.

Slice 03 extracts only the independently reusable MapUtil metadata, CRC, enumeration, and cache-path/output behavior. Owned empty-object maps prove cold/warm/corrupt/stale equivalence, standard/user precedence, XDG-only atomic writes, arbitrary-CWD operation, unchanged read roots, and deterministic write denial. Nonempty objects and every registry/localization edge remain explicitly M20-owned and inspected rather than runtime-proven. Provenance and removal controls bind all accepted providers to the production target and preserve the complete original INI registry.

Final validation passed the 10-test `original-data` label in all four GCC/Clang Debug/Release presets, the complete 82-test asset-free suite in each preset, and 10/10 focused Clang ASan/UBSan tests. The dependency ledger, extraction provenance, source classification, drift controls, provider-removal control, and ownership probes all pass. Acceptance details are recorded in the linked QA evidence.
