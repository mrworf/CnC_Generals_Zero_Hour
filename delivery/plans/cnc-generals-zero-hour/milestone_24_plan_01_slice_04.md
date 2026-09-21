# M24 slice 04: cumulative original persistence acceptance

## Goal and observable outcome

The accepted M24 save/load/replay behavior is proven by production-linked source identity, all four presets, sanitizer, read-only retail and negative controls, with PRE-033 evidence ready for downstream M25.

## Scope / non-scope

Own cumulative tests/evidence, any narrowly required repair, ledger and plan closeout. Do not claim renderer, LAN, Windows import or release readiness.

## Dependencies, entry and state

Slices 01–03 complete. Test installed/arbitrary-CWD original executable and project-owned then read-only retail scenarios; writes remain XDG-only. Preserve source-owned block/command/CRC witnesses and exact failure rollback.

## Permissions, validation and errors

Retail symlink is read-only; never commit private bytes/hashes/paths. Provider-removal and corrupted real save/replay must fail. If required original behavior depends on M22 hardware or an unsettled authority decision, record the exact dependency/blocker and stop; do not bypass it.

## Expected surfaces

`tests/original_persistence/*`, CMake test registrations, `docs/original-runtime-dependency-ledger.tsv`, `evidence/qa/cnc-generals-zero-hour/M24-original-persistence.md`, governing/index and slice plans; production changes only for discovered M24 defects.

## Tests and commands

- Positive/negative original source identity and provider-removal; full round-trip/replay with corrupted controls and fault injection; cross-preset checkpoint comparison.
- `cmake --build --preset <each>` and `ctest --preset <each> --output-on-failure` for all four presets; focused Clang ASan/UBSan; source ledger/drift checks.
- Separate read-only retail mission/skirmish save/load/replay, arbitrary-CWD/installed executable, XDG write isolation; record logical outcomes only.

## Acceptance / commit

Every M24 clause maps to passing source-owned evidence, known limitations are explicit, and no out-of-scope work leaked. Commit as `delivery: M24 slice 04 accept original persistence`.
