# M0 slice 01: legacy manifest closure

## Goal and observable outcome

Provide a checked, explicit inventory in which every source record from every tracked Zero Hour `.dsp` and every project from `RTS.dsw` has one owner or one exclusion/unavailable reason. A deterministic audit exits successfully for the repository and rejects a deliberately incomplete copy.

## Scope

- Parse VC6 `SOURCE=` records with Windows separators and case-insensitive path resolution.
- Record each manifest, project classification, source classification, intended native target, and reason.
- Emit explicit CMake lists for native candidates and excluded/unavailable records; use no production glob in generated CMake.
- Add an audit command/test that compares the checked inventory with tracked manifests.

## Non-scope

- Compile legacy sources, resolve their Win32 dependencies, or decide later renderer/API translations.
- Treat base Generals or tool executables as the Zero Hour runtime.

## Dependencies and ordering

No implementation dependency. This slice must complete before the bootstrap graph consumes its inventory.

## Entry point and behavior

`python3 tools/legacy_manifest_inventory.py --check` reads the legacy manifests and checked inventory. It reports counts and succeeds only when the inventory is complete and deterministic. `--generate` is a maintainer action that rewrites only the documented inventory outputs.

## Data and state transitions

VC6 manifest records become repository-owned `cmake/LegacySourceInventory.cmake` and `docs/legacy-manifest-inventory.md`. Regeneration is deterministic and contains relative paths only.

## Authorization and permissions

Not applicable. The tool reads tracked source/manifests and writes only project-owned inventory files when explicitly invoked with `--generate`.

## Validation and error handling

- Positive: checked outputs exactly match regeneration and all paths are classified once.
- Negative: an incomplete temporary inventory is rejected with the missing record named.
- Missing manifest paths are classified as unavailable/out of scope rather than silently discarded.

## Implementation surfaces

- `tools/legacy_manifest_inventory.py`
- `cmake/LegacySourceInventory.cmake`
- `docs/legacy-manifest-inventory.md`
- `tests/m0/test_legacy_manifest_inventory.py`

## Validation commands

- `python3 tools/legacy_manifest_inventory.py --check`
- `python3 -m unittest tests.m0.test_legacy_manifest_inventory`
- `git diff --check`

## Acceptance criteria

- All tracked `.dsp` SOURCE records and `RTS.dsw` projects have exactly one classification.
- Runtime candidates map to `zh_main`, engine/device, W3D, WWShade, support, or compression ownership.
- Tools, DRM/browser/GameSpy/download/benchmark/obsolete audio and absent third-party projects carry reasons.
- Generated CMake uses explicit entries and no source glob.

## Commit boundary

Commit the plan updates, generator/auditor, generated inventory, documentation, and tests together as `delivery: M0 slice 01 audit legacy manifests`.

## Completion evidence

- `python3 tools/legacy_manifest_inventory.py --check`: 1,889 candidate, 1,085 excluded, and 381 unavailable unique manifest/source records.
- `python3 -m unittest tests.m0.test_legacy_manifest_inventory`: two tests pass, including rejection of an incomplete inventory.
- `git diff --check`: pass.
