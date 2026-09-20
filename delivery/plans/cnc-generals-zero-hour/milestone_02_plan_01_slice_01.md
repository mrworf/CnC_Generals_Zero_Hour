# M2 slice 01: legacy renderer inventory

## Goal and observable outcome

Every D3D/D3DX/Direct3D identifier observed in the in-scope legacy C/C++ source has exactly one documented disposition. Contributors can rerun a deterministic checker, and any unclassified future identifier fails with an actionable diagnostic.

## Scope

- Add a versioned mapping-rule table covering resources, formats, primitives/FVF, render state, texture state, shaders, device lifecycle/calls, math helpers, capabilities, and explicitly unsupported platform behavior.
- Add an inventory checker that scans the Zero Hour and shared Generals source roots with the same file policy every time.
- Record SDL_GPU public API, repository shader, CPU conversion, compatibility adapter, or unsupported decisions with explanations.
- Add positive completeness and negative missing/ambiguous-rule tests under `renderer-contract`.

## Non-scope

No engine API, shader implementation, device creation, source conversion, or edits to legacy files.

## Dependencies and ordering

M0 source manifests and Python test support. This slice precedes the renderer contract and shader registry.

## Entry point and end-to-end behavior

`tools/renderer_inventory.py --check` scans configured roots, maps every identifier through the checked rule table, prints category totals and exits zero. Unmapped, multiply mapped, malformed, or empty rules exit nonzero and list the token/rule problem.

## Data/state transitions

Read-only source text becomes an in-memory set of tokens and a deterministic classification report. No source or retail data is modified.

## Authorization and permissions

Not applicable: the tool reads repository-owned source only and writes nothing.

## Validation and error handling

- Positive: the complete repository inventory maps exactly once and reports all required categories/dispositions.
- Negative: a synthetic source token with an incomplete rule set is rejected; overlapping rules are rejected.
- Rule schema errors name the row and missing field.

## Expected implementation surfaces

`docs/renderer/legacy-api-mapping.tsv`, `tools/renderer_inventory.py`, `tests/m2/test_renderer_inventory.py`, and CMake test registration.

## Required commands

- `python3 tools/renderer_inventory.py --check`
- `python3 -m unittest tests.m2.test_renderer_inventory`
- configure/build one debug preset and run its `renderer-contract` label.

## Acceptance criteria

- Every source-observed token maps exactly once.
- Mapping rows provide a category, disposition, target, and non-empty rationale.
- Required inventory categories and decisions are present, including direct-device/lifecycle, formats, fixed-function state, FVF/primitives, shader assembly, render targets, texture dimensions/BC fallback, lost-device handling, and WWShade.

## Commit boundary

Commit the rule table, checker, tests, CMake registration, this slice plan, and governing-plan status update as `delivery: M2 slice 01 classify renderer inventory`.
