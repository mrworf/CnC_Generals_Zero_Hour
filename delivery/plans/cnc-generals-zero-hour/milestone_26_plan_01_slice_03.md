# M26 plan 01 slice 03: FPU and dependency evidence

## Goal and outcome

The one original `setFPMode` definition lives in shared lower support and produces characterized nearest-rounding/24-bit x87 behavior, while a checked source dependency ledger and target-specific identity gates prevent M26 providers or newly discovered required edges from becoming unowned.

## Scope

Included: behavior-preserving extraction from `GameLogic.cpp`, Linux x86-64 fenv/x87 control, bounded original ABI consumer, M26 dependency ledger, source/registry drift checks, compile/link/runtime identity, provider-removal and ownerless-edge negative controls, full four-preset and sanitizer evidence. Excluded: M27 real INI/MapCache caller integration and all later runtime providers.

## Dependencies and ordering

Depends on slices 01 and 02. It is the final M26 slice and runs cumulative milestone validation.

## Entry point and behavior

The FPU test deliberately changes rounding/precision, calls the actual extracted original definition, and verifies nearest rounding plus 24-bit x87 precision characterization where the architecture exposes it. The ledger checker maps every M26 original source/symbol to its consumer, target, lifecycle/config branches, globals/callbacks, assets/writes, owner, and source/compile/link/runtime evidence grade. It fingerprints consumed source and registry lists and rejects missing owners. Identity checks bind exact compile-command source paths to live executable symbols and runtime witnesses.

## Data and state transitions

FPU state is changed, normalized, verified, and restored by the test process. Ledger evidence advances only from inspected to compile/link/runtime-proven when its corresponding artifact exists; source changes invalidate the checked fingerprint.

## Authorization and permissions

Not applicable. All evidence is generated from repository sources/build products and contains no retail/private paths.

## Validation and error handling

Positive: nearest-rounding and x87 precision characterization, exact source compilation, live symbol contribution, runtime witnesses, all required owner/edge fields, clean drift check. Negative: remove provider from a scratch target, omit owner, alter a scratch source fingerprint, and substitute a basename-only/discarded symbol; every control must fail.

Run focused M26 tests in all four presets, then the canonical full asset-free CTest once per required preset as practical under the milestone contract. Run ASan/UBSan with exact options disclosed; report explicit live counts and do not infer leak freedom from disabled leak detection.

## Implementation surfaces

- Original `GameLogic.cpp` and a shared original FPU source near `FPUControl.h`.
- `CMakeLists.txt`, M26 tests, identity/ledger tools, and checked ledger under `docs/` or `data/` according to repository convention.
- Durable QA evidence under `evidence/qa/cnc-generals-zero-hour/`.

## Acceptance criteria

- Exactly one production `setFPMode` definition exists and its actual body executes in GCC/Clang Debug/Release.
- Ledger freshness and ownerless-provider checks fail closed and name M27/M28/M20 owners for deferred edges.
- Compile, link, and runtime evidence is target-specific and cannot pass on a basename/discarded section alone.
- Full asset-free regressions and focused sanitizers pass with settings and counts recorded.
- Every M26 acceptance criterion maps to committed executable evidence.

## Commit boundary

Commit FPU extraction, dependency/identity controls, cumulative evidence, and final plan status/commit references as the final M26 slice.

## Result

Complete. The sole production `setFPMode` definition is extracted into lower shared original support and its actual x86-64 Linux body is runtime-characterized as nearest rounding with x87 24-bit precision. The checked ledger covers M26 runtime providers and retains explicit M27/M28/M20 ownership for deferred consumers; source drift and ownerless mutations fail. Compile-command, link-map, live-symbol, runtime-witness, and real provider-removal gates pass in GCC/Clang Debug/Release. Focused tests pass 10/10 and full asset-free CTest passes 72/72 in each preset. Clang Debug ASan/UBSan passes 10/10 with the exact options recorded in the evidence report; a sanitizer-discovered Linux pool alignment defect was fixed before final validation.
