# Milestone N5: Playable single-player and persistence

## Objective

Representative campaigns/skirmishes and user maps complete with working saves and deterministic replays.

## User/System Outcome

Representative campaigns/skirmishes and user maps complete with working saves and deterministic replays.

## Scope

Original single-player journeys, loading, gameplay, save/load/replay formats and compiler/build determinism.

## Explicit Exclusions

No mixed Windows compatibility requirement or gameplay redesign.

## Source Requirements

R05, R10 in docs/zero-hour-linux-port-plan.md. R02–R05 constrain all applicable work.
The user-approved replacement graph fixes this milestone's boundary.

## Preconditions

- N3 accepted before implementation consumes its output.
- N4 accepted before implementation consumes its output.
- PRE-04, PRE-07: see delivery/readiness/cnc-generals-zero-hour/linux-upstream-v2/prerequisite-manifest.md.

## Readiness checks

- Inspect status.yaml and the named dependency acceptance evidence; pending providers are not accepted.
- Inspect the governing plan and AGENTS.md; only linux-upstream-v2 is active.
- Check the prerequisite manifest's command results and milestone-owned deliverables before starting.

## Functional Requirements

- Integrate complete game startup→menu→load→play→save→reload→finish→menu→shutdown journeys.
- Derive portable field widths and versioning from original persistence owners.
- Validate malformed/repeated load rejection and preserve prior valid state where supported.
- Document original bug fixes independently with source reproducer and regression.

## UX Constraints

Preserve source-established UI/gameplay behavior. Render loading/error states truthfully;
never claim a scene or operation succeeded when required content or output is missing.
N0/N1 have developer-facing diagnostics; they do not redefine player-facing UX.

## Architecture / Security Constraints

Stock dependencies/public APIs only. Retail roots are read-only, diagnostics redacted,
no proprietary distributions. No extra privileges or network accounts are introduced.
Validate externally supplied lengths/paths and preserve allocator/thread ownership.

## Interfaces and Compatibility

Zero Hour internal interfaces may change; no legacy MSVC/internal Windows ABI mandate.
Preserve source asset formats, externally meaningful encodings and intended behavior.
External dependencies use their supported interfaces; do not publish custom SDK APIs.

## Acceptance Criteria

- [ ] Representative campaign and skirmish sessions reach completion on Linux.
- [ ] Linux saves round-trip and replay checkpoints match across GCC/Clang Debug/Release.
- [ ] User map load, failed load recovery and repeated games preserve original gameplay.

## Required Validation

Original-engine end-to-end scenarios, generated persistence boundary negatives, Linux replay comparison and read-only private scenario acceptance.
Freeze coherent changes before broad checks; document unexecuted checks honestly.
Documentation-only work receives artifact review, not invented game tests.

## Known Risks / Deferred Work

Windows 1.04 import is optional future work. Source fidelity is required even when archived modeled-runtime tests passed.
