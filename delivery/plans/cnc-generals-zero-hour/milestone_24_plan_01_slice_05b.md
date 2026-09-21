# M24 slice 05B: original shipped GameData ownership and override baseline

## Goal and observable outcome

The original Linux `GlobalData` default, shipped, and map-specific layers hold complete source-defined values with independent owned memory. The installed shipped `PartitionCellSize = 40.0` survives repeated original reset; a temporary map.ini value is removed. This slice is asset-free and independently testable, not a retail first-tick acceptance claim.

## Scope / non-scope

Own the source `GlobalData` override copy/ownership and the Linux shipped-baseline promotion after installed INI parsing, plus source-owned lifecycle/negative tests. Preserve the original `INI_LOAD_CREATE_OVERRIDES` dispatch for shipped and map files and parser error semantics. Do not suppress `UseTrees`, skip drawables, change M22 renderer factories, invent an alternate partition size, or weaken exact source CRC.

## Dependencies and source trace

Slice 05A is complete. `GlobalData::newOverride` currently invokes an intentionally unimplemented copy assignment; a valid copy also needs independent `WeaponBonusSet` ownership. The subsystem list owns the original root, while Linux loads shipped GameData as an override; `GlobalData::reset` deletes all overrides and loses shipped `PartitionCellSize = 40.0`, leaving 0 and causing a 1-unit/20.8-million-cell partition on the original retail mission tick. Promote installed shipped values into the root before any map.ini load while retaining map override reset semantics. A correct copy newly exposes an optimized-tree draw request during retail mission `GameLogic::startNewGame(FALSE):1868`; that is an M22 §9 integration dependency assigned to 05C, not a reason to discard correct GameData values.

## Entry, transitions, permissions and recovery

At source bootstrap, the default GameData populates the root and shipped GameData creates an override through the unchanged parser. After both finish successfully, Linux promotes the top installed value to the subsystem-owned root and destroys the temporary layer chain. Map.ini later creates independently owned transient layers; reset deletes them and returns to shipped root. Malformed shipped input fails the original loader without publishing a partial baseline. Owned fixture roots are read-only; all output is isolated under XDG.

## Expected surfaces and tests

Original `GlobalData` copy/override logic, Linux `GameEngine` installed-layer commit point, a source-owned default/shipped/map fixture, the original dependency ledger, and slice evidence. Positive values: default 16, shipped 40, map 5, reset/repeated reset 40. Negative invalid shipped definition must reject; strict Clang ASan/UBSan must show no use-after-free/double-free. Re-run focused original persistence/simulation in all four presets, source identity/provider-removal and ledger checks. No private retail material enters fixture or evidence.

## Acceptance / commit

Full `GlobalData` copy and ownership are correct; installed shipped values persist while map values remain temporary; malformed input rejects and original-source regressions pass. The genuine M22 retail draw dependency remains recorded for 05C/05D. Commit one coherent slice as `delivery: M24 slice 05B retain original shipped GameData`.
