# Slice 01: Align platform and prerequisite authority

## Goal and outcome

Make the implementation-ready port plan accurately describe the supported x86-64 environments and currently verified external prerequisites.

## Scope

- Remove ARM64 requirements from scope, presets, prerequisites, milestones, validation, and recommendation text.
- Establish Arch Linux x86-64 as the primary playable environment.
- Retain Ubuntu 26.04 and Fedora 44 as clean-build portability environments.
- Record the local retail corpus mapping, English locale, working RTX 4070 Vulkan driver, missing validation layer, and absence of optional Windows replay/save fixtures.
- Ignore the local `original_game_symlink` without modifying or copying its contents.

## Non-scope

- No source port implementation, package installation, retail-data extraction, or GPU rendering work.
- No change to fixed-width, UTF-16, endian, or serialization safety requirements.
- No claim that M14 visual acceptance has run.

## Dependencies and ordering

- Governing decision: `20260919174617_arch_x86_64_target_update.md`.
- Retail and Vulkan facts were established through read-only inspection before this slice.
- This slice precedes M0 implementation.

## Behavior and state

- Documentation consumers receive one consistent x86-64-only target matrix.
- PRE-008 identifies the local corpus as verified but keeps private paths out of tracked configuration.
- PRE-012 distinguishes available Vulkan hardware/driver from the still-missing validation-layer package and unexecuted SDL_GPU acceptance.
- PRE-015 remains optional and explicitly records that no fixtures were found.
- The retail symlink remains local and untracked.

Authorization is not applicable: this slice changes repository documentation and ignore metadata only. It does not access external services or mutate the linked retail installation.

## Implementation surfaces

- `docs/zero-hour-linux-port-plan.md`
- `.gitignore`
- This governing plan and slice artifact

## Validation

- Positive: search confirms Arch is the primary playable target and Ubuntu/Fedora remain clean-build targets.
- Positive: PRE-008, PRE-012, PRE-015, M0, M14, M17, and the validation matrix agree with the approved decision.
- Negative: any remaining `ARM64` or `aarch64` reference only states that the architecture is unsupported; none assigns implementation or validation work.
- Negative: Git status does not list `original_game_symlink` after the ignore rule is added.
- Review the staged diff for private absolute paths or retail bytes.

## Acceptance criteria

- The plan is internally consistent and x86-64 only.
- Arch package documentation and validation-layer installation are assigned to M0/M14 respectively.
- The host Vulkan probe is evidence of prerequisite availability, not premature M14 acceptance.
- No replay/save fixture is asserted to exist.
- All slice-owned files are committed together in one documentation commit.

## Commit boundary

Commit the plan artifacts, revised port plan, and symlink ignore rule as one coherent documentation slice.
