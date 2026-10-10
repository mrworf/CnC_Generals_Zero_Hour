# Milestone N3: Original world rendering

## Objective

The actual game renders required scene categories and survives repeated scene/camera transitions.

## User/System Outcome

The actual game renders required scene categories and survives repeated scene/camera transitions.

## Scope

Terrain, models/animation, camera/LOD, stencil/projected shadows, shroud, water, particles, smudges, loading and persistent resources.

## Explicit Exclusions

No remaster, invented rendering proxy accepted as parity, dependency patches or changes to simulation cadence.

## Source Requirements

R05, R08 in docs/zero-hour-linux-port-plan.md. R02–R05 constrain all applicable work.
The user-approved replacement graph fixes this milestone's boundary.

## Preconditions

- N1 accepted before consuming its output.
- N2 accepted before consuming its output.
- PRE-03, PRE-04, PRE-05, PRE-06, PRE-10: named providers, boundaries and verification are in
  `delivery/readiness/cnc-generals-zero-hour/linux-upstream-v2/prerequisite-manifest.md`.

## Readiness checks

- Inspect the current definition/planning handoff and packet; only `linux-upstream-v2` is active.
- Verify N1 and N2 completed with revision-specific acceptance in `status.yaml`; H1 alone does not accept visual providers.
- Run `python3 tools/upstream_bgfx.py verify` for the exact locked dependency sources and `vulkaninfo --summary` in the normal host session. Inspect GPU/layer and supported display availability before physical tests; never treat sandbox ICD failure as missing host hardware.
- Review current tool/library identity against N1 evidence. Revalidate affected suitability when pins/configuration change; retain original N1 completion history.

## Functional Requirements

- Integrate a focused stock-bgfx game renderer and semantically translated owned shaders.
- Render to sampleable scene targets where needed; preserve ordered copies/clears and transforms.
- Stage loading without simulation ticks; update changed resources during play.
- Cover whole-source scene combinations and transitions, not only isolated renderer fixtures.

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

- [ ] Required scene categories have source-linked physical evidence and read-only retail acceptance.
- [ ] Large generated scenes exercise actual draw cardinality and bytes without omitted rendering.
- [ ] Repeated loads, camera/LOD changes, resize and teardown stabilize resource usage.

## Required Validation

Generated physical scenes plus original-source captures, read-only retail checks with integrity evidence, GCC/Clang owner sanitizers and Vulkan validation.
Freeze coherent changes before broad checks; document unexecuted checks honestly.
Documentation-only work receives artifact review, not invented game tests.

## Known Risks / Deferred Work

Prior simplified smudge and shroud fixtures are not visual parity references. User maps need explicit supported limits based on source behavior.

## R07 qualification boundary

H1 qualification in N2 omits physical output only. This milestone still owns complete source-required draw providers, ordered model animation/ticks, terrain/water/shadows/particles and real scene submission. Preserve existing useful CPU/model work and its affected regressions; H1 does not prove it complete.

Governing clarification: `product/headless-qualification.md::HQ-04/HQ-07`.
