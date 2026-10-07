# Milestone N0: Clean baseline and specification

## Objective

Verified recoverability and a single original-source baseline governed by the new plan.

## User/System Outcome

Verified recoverability and a single original-source baseline governed by the new plan.

## Scope

Recovery snapshots/history, original-source reset, dependency policy, findings, and replacement packet.

## Explicit Exclusions

No renderer/game implementation is accepted by resetting source.

## Source Requirements

R01 in docs/zero-hour-linux-port-plan.md. R02–R05 constrain all applicable work.
The user-approved replacement graph fixes this milestone's boundary.

## Preconditions

- No prior milestone; explicit reset authority and verified recovery are required.
- PRE-01: see delivery/readiness/cnc-generals-zero-hour/linux-upstream-v2/prerequisite-manifest.md.

## Readiness checks

- Inspect status.yaml and the named dependency acceptance evidence; pending providers are not accepted.
- Inspect the governing plan and AGENTS.md; only linux-upstream-v2 is active.
- Check the prerequisite manifest's command results and milestone-owned deliverables before starting.

## Functional Requirements

- Verify archive contents against live source before reset; never traverse retail symlinks.
- Retire old implementation/builds and archive old milestone authority.
- Persist source requirements, graph, readiness and recovery instructions.

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

- [ ] Archives verify and baseline original subtrees match the requested commit.
- [ ] Old source/build trees cannot enter the active build; retail link identity is unchanged.
- [ ] Every R01–R12 requirement maps to the replacement packet.

## Required Validation

Inspect evidence/recovery/20261007-reset.md, git diff against the baseline, packet validator, and requirement coverage.
Freeze coherent changes before broad checks; document unexecuted checks honestly.
Documentation-only work receives artifact review, not invented game tests.

## Known Risks / Deferred Work

Reset already executed under explicit user authority; archive is reference-only.
