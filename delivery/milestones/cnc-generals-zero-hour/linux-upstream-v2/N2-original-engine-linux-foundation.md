# Milestone N2: Original engine Linux foundation

## Objective

The actual original Zero Hour process initializes, loads supplied data and runs deterministic headless simulation on Linux.

## User/System Outcome

The actual original Zero Hour process initializes, loads supplied data and runs deterministic headless simulation on Linux.

## Scope

CMake original-source graph; fixed-width types, platform services, allocator boundaries, VFS/data roots, original startup/teardown and simulation.

## Explicit Exclusions

No toy replacement simulation, rendering acceptance, Windows compiler support or proprietary distribution.

## Source Requirements

R03, R04, R07 in docs/zero-hour-linux-port-plan.md. R02–R05 constrain all applicable work.
The user-approved replacement graph fixes this milestone's boundary.

## Preconditions

- N0 accepted before implementation consumes its output.
- PRE-01, PRE-04, PRE-05: see delivery/readiness/cnc-generals-zero-hour/linux-upstream-v2/prerequisite-manifest.md.

## Readiness checks

- Inspect status.yaml and the named dependency acceptance evidence; pending providers are not accepted.
- Inspect the governing plan and AGENTS.md; only linux-upstream-v2 is active.
- Check the prerequisite manifest's command results and milestone-owned deliverables before starting.

## Functional Requirements

- Port original runtime and shared services using standard C++/POSIX; remove platform-only build assumptions.
- Implement explicit user data roots, BIG/INI/CSF access, content precedence and actionable missing-data errors.
- Keep asset storage read-only; support cache miss/corruption/unwritable directory fallback.
- Prove original-source execution and repeated init/run/shutdown without replacing gameplay logic.

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

- [ ] GCC and Clang build and execute the same original simulation fixture with matching checkpoints.
- [ ] Malformed data and missing roots fail cleanly; original assets remain unchanged.
- [ ] Allocator ownership is sanitizer-clean and teardown permits repeat startup.

## Required Validation

Original-source reachability, deterministic generated fixtures, path/length rejection, source-owner allocation tests under GCC/Clang sanitizers; private runtime checks redact paths/results.
Freeze coherent changes before broad checks; document unexecuted checks honestly.
Documentation-only work receives artifact review, not invented game tests.

## Known Risks / Deferred Work

File/network encodings must be derived from original readers/writers. Original global memory pools cross library boundaries.
