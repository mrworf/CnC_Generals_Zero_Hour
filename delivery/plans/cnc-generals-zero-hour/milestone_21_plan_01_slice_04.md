# M21 plan 01 slice 04: lifecycle, retail, and cumulative acceptance

## Goal and observable outcome

Mission and skirmish can return/reset and re-enter using original ownership without stale callbacks/resources, and the complete M21 contract passes four-preset, full-suite, sanitizer, identity, negative-control, and gated read-only retail validation.

## Scope

Included: scenario exit/reset/re-entry, exact resource release, production identity and source ledger freshness, four native presets, one full asset-free CTest pass per preset, focused sanitizers, gated retail map runs, final M21 evidence. Excluded: renderer hardware/session, interactive media/UI, full persistence, LAN, release/distro acceptance.

## Dependencies and ordering

Requires slices 01 through 03. This is the milestone completion boundary.

## Entry point and end-to-end behavior

The production target runs mission, returns/resets, enters skirmish, completes a terminal outcome, and tears down. The same target is exercised against separately provisioned read-only retail maps when the gate is enabled.

## Data and state transitions

Active game state returns to empty/reset baselines between scenarios and after shutdown. Re-entry creates fresh state; no old map objects, commands, callbacks, script/AI owners, props, preload records, or recorder controls survive.

## Authorization and permissions

Retail trees are read-only. Evidence records only logical outcomes and aggregate counts, never private paths, bytes, filenames beyond public logical identifiers, or hashes.

## Validation and error handling

Positive: repeated mission/skirmish entry and gated retail maps complete required checkpoints. Negative: provider removal, missing map/module, malformed chunk, invalid command, failure-stage injection, and stale-state sentinels all fail correctly. Sanitizers and explicit allocation/resource counters must be clean under the documented leak-detection setting.

## Implementation surfaces

Expected: reset/teardown corrections, cumulative tests and scripts, source classification/ledger checks, `evidence/qa/cnc-generals-zero-hour/M21-original-simulation.md`, governing plan status and commit/evidence references.

## Required commands

- Configure/build and focused original-simulation tests in all four presets.
- Complete asset-free CTest suite once in all four presets.
- Relevant Clang Debug ASan/UBSan original-simulation suite.
- Installed/arbitrary-CWD owned fixture acceptance and explicitly gated read-only retail mission/skirmish acceptance.
- Identity, provider-removal, source-classification, ledger-freshness, and no-physical-device checks.

## Acceptance criteria

- Every M21 acceptance clause maps to current concrete original-source evidence.
- Reset/re-entry and every failure leave zero M21-owned live resources and no destroyed-global access.
- All required validation passes in the stated contexts, or a genuine blocker is durably recorded with its exact unblock condition and resume point.
- Evidence makes no M22/M23/M24/M25 claim and discloses no retail-private information.

## Commit boundary

Commit lifecycle corrections, cumulative tests, final evidence, and completed plan status as the final M21 slice commit. Milestone and workflow status remain parent-orchestrator owned.

## Completion

Complete in this slice commit. The four-preset matrix, focused strict sanitizer
run, owned arbitrary-CWD lifecycle, provider-removal controls, recursive
read-only retail gate, and cumulative acceptance are recorded in
`evidence/qa/cnc-generals-zero-hour/M21-original-simulation.md`.
