# M10 slice 01: checked effects mapping and offline GLSL closure

## Goal and observable outcome

Provide one canonical mapping for water, particles/point sprites, projected textures, bump/environment mapping, stealth, post effects, and WWShade multipass materials. Every logical corpus entry resolves to named GLSL sources and immutable pipeline state, and every stage source compiles in the normal build.

## Scope

- Add a project-owned logical effects corpus manifest.
- Add an engine-facing effects mapping API and validation.
- Add named GLSL stages for the previously uncovered projected, bump/environment, stealth, and post-effect families.
- Drive offline compilation from the complete mapping source list.
- Test completeness, state invariants, manifest resolution, unknown/duplicate diagnostics, and compiled module presence.

## Non-scope

No command recording or device resource ownership is added in this slice. No retail program bytes are copied or interpreted. Pixel equivalence is deferred to M14.

## Dependencies and ordering

Depends on M4 logical-corpus conventions and M7 renderer contracts/shader compilation. Slice 02 depends on this slice's public mapping.

## Entry point and behavior

Callers inspect `effect_registry()`, validate it with `validate_effect_registry`, parse the logical manifest with `parse_effect_corpus`, and resolve a legacy name with `find_effect`. A failure reports the logical asset/material supplied by the caller instead of choosing a fallback effect.

## Data and state transitions

The registry and parsed manifest are immutable views/value objects. No runtime resources are created. CMake transforms each named GLSL stage into a generated SPIR-V module.

## Authorization and permissions

Not applicable: this is an in-process mapping and build-time shader compilation path with no privileged or external I/O. Tests read only repository-owned logical manifests.

## Validation, errors, and recovery

- Reject null, incomplete, duplicate, invalid-pass, and state-inconsistent registries.
- Reject malformed/duplicate/unknown manifest rows with line/logical-name context.
- Reject unknown runtime lookup with both logical asset and material in the diagnostic.
- A corrected input may be reparsed/resolved without retained state.

## Expected implementation surfaces

`include/zh/effects/*`, `src/effects/*`, `data/corpus/effects-manifest.tsv`, `shaders/effects/*`, `tests/effects/test_effect_registry.cpp`, and explicit CMake target/test/source lists.

## Positive tests

- Canonical registry validates and contains all seven scoped families.
- Logical manifest maps every canonical entry exactly once.
- All registered GLSL sources and generated SPIR-V modules exist.
- Point size, depth bias, premultiplied alpha, render-target input, and multipass invariants are represented.

## Negative tests

- Duplicate family/name and invalid state combinations fail validation.
- Unknown manifest effect and direct lookup name the logical asset/material.
- Missing manifest entry fails completeness.

## Required commands

```sh
cmake --preset linux-gcc-debug
cmake --build --preset linux-gcc-debug
ctest --preset linux-gcc-debug -R effects_registry --output-on-failure
ctest --preset linux-gcc-debug -L renderer-contract --output-on-failure
```

## Acceptance criteria

The mapping is complete, checked, actionable on errors, and every mapped stage compiles offline without a GPU.

## Delivered evidence

- `cmake --preset linux-gcc-debug` and `cmake --build --preset linux-gcc-debug` passed.
- `ctest --preset linux-gcc-debug -R effects_registry --output-on-failure` passed (1/1).
- `ctest --preset linux-gcc-debug -L renderer-contract --output-on-failure` passed (14/14).
- The logical manifest contains seven entries and exposes no retail bytes or host paths.

## Commit boundary

Commit the governing/slice plans, logical manifest, mapping API/implementation, named shaders, CMake closure, and focused mapping tests together as `delivery: M10 slice 01 map effects`.
