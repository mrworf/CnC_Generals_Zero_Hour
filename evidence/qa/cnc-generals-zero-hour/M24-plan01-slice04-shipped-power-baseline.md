# M24 slice 04 — shipped special-power lifetime

Grade: original-source global INI/reset behavior, project-owned positive/negative fixture and separately gated read-only retail mission/skirmish. Exact retail persistence, replay and first tick remain slices 05–06.

The Linux shipped second-INI loader uses original `INI_LOAD_CREATE_OVERRIDES`. The original `SpecialPowerStore::reset` deletes newly named shipped powers because their root is marked transient; a retail mission then reached load with only one power, and `CHUNK_InGameUI` failed on a missing template. Immediately after original `GameEngine::initSubsystem` has loaded the global shipped SpecialPower INIs, the Linux path marks the complete source `SpecialPowerStore` chain as baseline. Later map.ini definitions and overrides remain transient under the original reset traversal. No other template store or renderer path changes.

`original_persistence_power_baseline` uses an asset-free source-parsed map and INIs. It verifies a shipped `PublicTimer=Yes` power is overridden to `No` by map.ini, then returns to `Yes` after source reset; a map-only power is removed. A missing shipped power INI is rejected. The separately gated retail mission and skirmish check verifies the shipped power count remains stable through source start/reset and the retail corpus metadata is unchanged. Only aggregate counts and outcomes are recorded, not retail contents or names.

Validation:

- All four preset `zh_original_main` builds and 14/14 focused original persistence/simulation tests per preset — pass. The strengthened owned map override case was run after its source change in each preset.
- Read-only retail mission/skirmish power baseline — pass; input metadata unchanged.
- `python3 tools/check_original_dependency_ledger.py --root . --ledger docs/original-runtime-dependency-ledger.tsv` and `git diff --check` — pass.

This slice does not resolve the separately observed retail `PartitionManager` CRC divergence or first simulation-tick fault.
