# M19 slice 01: auditable source classification and identity gate

Status: completed

## Goal and observable outcome

Every legacy inventory entry has exactly one current disposition with a provider and rationale, and a project-owned check rejects missing, duplicate, malformed, or bootstrap-backed production providers.

## Scope

Generate a deterministic repository-owned classification from the canonical legacy inventory plus a reviewed provider policy. Add a build/test entry point that validates coverage and production-source identity. Mark bootstrap targets and the toy simulation fixture-only in machine-readable build metadata and human-readable evidence.

## Non-scope

This slice does not compile original algorithms or claim runtime execution. It does not classify retail files and never accesses `original_game_symlink`.

## Dependencies and ordering

First slice. It defines the identity contract consumed by slices 02 and 03.

## End-to-end behavior and state

Configure generates a complete classification report. The validator reads the report and actual build declarations, succeeds only when every inventory item is classified once, and fails when a required production provider is absent or points to bootstrap code. Inputs are repository source metadata; no persistent user state is changed.

## Authorization and permissions

No privileged operation, network, retail data, display, or GPU access applies.

## Validation and recovery

Positive tests cover complete deterministic classification and known fixture-only targets. Negative tests inject a missing source/provider and a bootstrap production provider. Malformed policy data fails with a precise diagnostic. Regenerate from canonical input after correcting policy errors.

## Expected implementation surfaces

`CMakeLists.txt`, `cmake/`, `tools/`, `tests/original_support/`, and `evidence/qa/cnc-generals-zero-hour/`.

## Commands

- Configure/build the focused original-source identity target with GCC Debug.
- Run the identity/classification CTest labels.
- Run the validator's negative fixtures directly through CTest.

## Acceptance criteria

- All canonical legacy inventory paths are covered exactly once.
- Every classification has provider and rationale.
- Bootstrap/toy code cannot satisfy a production-provider gate.
- Output contains no retail path, bytes, or hash.

## Commit boundary

One commit containing policy/generator, generated classification/evidence, tests, build integration, and this slice's completion record.

## Completion record

Implemented deterministic classification for all 3,293 unique canonical inventory paths, with provider/rationale and duplicate-origin consolidation. Added positive coverage and negative missing-policy/bootstrap/toy/fixture controls. GCC Debug configured and built successfully; focused CTest passed 2/2 and direct unit tests passed 4/4. Commit: pending.
