# Milestone N7: Release and dependency maintenance

## Objective

A clean checkout produces an installable asset-free game, with tested dependency upgrades.

## User/System Outcome

A clean checkout produces an installable asset-free game, with tested dependency upgrades.

## Scope

Clean builds, Arch/Ubuntu/Fedora portability, packaging, installation/data instructions, provenance and upgrade checks.

## Explicit Exclusions

No bundled proprietary assets, ARM64, Windows release or new packaging platform mandate.

## Source Requirements

R02, R03, R04, R12 in docs/zero-hour-linux-port-plan.md. R02–R05 constrain all applicable work.
The user-approved replacement graph fixes this milestone's boundary.

## Preconditions

- N6 accepted before implementation consumes its output.
- PRE-09: see delivery/readiness/cnc-generals-zero-hour/linux-upstream-v2/prerequisite-manifest.md.

## Readiness checks

- Inspect status.yaml and the named dependency acceptance evidence; pending providers are not accepted.
- Inspect the governing plan and AGENTS.md; only linux-upstream-v2 is active.
- Check the prerequisite manifest's command results and milestone-owned deliverables before starting.

## Functional Requirements

- Build from documented prerequisites without recovery trees or hidden developer caches.
- Package only redistributable project/dependency outputs; scan for assets and derived caches.
- Test installation from arbitrary CWD with supplied roots and XDG writes.
- Exercise a second verified upstream bgfx revision by rebuilding tools/shaders and rerunning N1 plus game smoke tests.

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

- [ ] Clean supported Linux builds and install smoke tests pass with GCC/Clang coverage.
- [ ] Distribution contains no proprietary assets, cache derivatives or private paths.
- [ ] Version update is accomplished through pins and game integration only, with recorded regression results.

## Required Validation

Clean-checkout/container builds where available, package inventory, installed-game smoke journeys, dependency cleanliness and upgrade matrix.
Freeze coherent changes before broad checks; document unexecuted checks honestly.
Documentation-only work receives artifact review, not invented game tests.

## Known Risks / Deferred Work

Distribution-specific external setup is declared, not silently inherited. A failing dependency upgrade is not shipped.
