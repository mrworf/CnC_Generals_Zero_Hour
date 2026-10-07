# Milestone N1: Stock renderer suitability

## Objective

A pristine stock renderer demonstrates every required high-risk rendering behavior on the host GPU.

## User/System Outcome

A pristine stock renderer demonstrates every required high-risk rendering behavior on the host GPU.

## Scope

Upstream provenance/builds, supported shader transport, generated physical fixtures, capacity and lifecycle qualification.

## Explicit Exclusions

No original-world integration, proprietary fixture distribution, library patching or acceptance based only on mocks.

## Source Requirements

R02, R06 in docs/zero-hour-linux-port-plan.md. R02–R05 constrain all applicable work.
The user-approved replacement graph fixes this milestone's boundary.

## Preconditions

- N0 accepted before implementation consumes its output.
- PRE-01, PRE-02, PRE-03, PRE-05: see delivery/readiness/cnc-generals-zero-hour/linux-upstream-v2/prerequisite-manifest.md.

## Readiness checks

- Inspect status.yaml and the named dependency acceptance evidence; pending providers are not accepted.
- Inspect the governing plan and AGENTS.md; only linux-upstream-v2 is active.
- Check the prerequisite manifest's command results and milestone-owned deliverables before starting.

## Functional Requirements

- Acquire matched official upstream revisions; verify clean sources and build stock tools/runtime.
- Prove viewport clears, selective stencil writes with shadows, exact sampled mip ranges and filters.
- Prove draw/copy/distortion order, representative material shaders and packed attributes.
- Exercise source-derived draw/resource/upload workloads, resize, replacement, cancel and shutdown.
- If any mandatory behavior requires a patch/private API/visual compromise, fail and hand off for architecture reassessment.

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

- [ ] All seven tests in the governing plan's N1 suitability gate have current physical evidence.
- [ ] Shader compiler/runtime use unmodified upstream sources and public interfaces.
- [ ] GCC/Clang ownership tests and Vulkan validation pass; required draws/effects are not dropped.

## Required Validation

Build/test commands are N1 deliverables. First inspect pristine public API/source and install-free tool availability; use generated fixtures, recorded GPU identity, negative controls, and frozen broad matrix.
Freeze coherent changes before broad checks; document unexecuted checks honestly.
Documentation-only work receives artifact review, not invented game tests.

## Known Risks / Deferred Work

bgfx remains a candidate until all gates pass. API inventory may expose a real public capability gap.
