# M24 slice 04: shipped special-power definition lifetime

## Goal and observable outcome

The original source SpecialPowerStore retains all shipped global INI definitions through game reset while original map.ini override layers are still discarded. Read-only retail initialization proves the shipped set remains present across source scenario start/reset, and a project-owned override test proves transient map definitions do not leak across reset.

## Scope / non-scope

Own the narrow Linux shipped-INI baseline correction and source reset tests. Do not change arbitrary template stores, allow map overrides to survive as global data, or claim full retail persistence/replay or M24 acceptance.

## Dependencies, entry and state

Slices 01–03 complete. The Linux loader currently passes the shipped second INI to original parsers as `INI_LOAD_CREATE_OVERRIDES`; `SpecialPowerStore::reset` removes newly named shipped powers because their `Overridable` root is marked transient. A read-only retail mission was observed with one power after source start despite the shipped INI being visible; `CHUNK_InGameUI` then rejected a missing power on load. Preserve source parser/reset authority and mark only the fully loaded shipped power chain as baseline before any map.ini processing.

## Permissions, validation and errors

Retail symlink is read-only; never commit private bytes/hashes/paths. Invalid or absent shipped INI still fails existing startup validation. A map-created power must remain removable on reset. Do not substitute a synthetic power list or exempt the entire store from reset.

## Expected surfaces

`Overridable`, original `SpecialPowerStore`, source `GameEngine` initialization, a focused source-owned reset test, read-only retail count gate and slice evidence. Other stores are not changed without separate source evidence.

## Tests and commands

- Project-owned source INI fixture: shipped new power survives `TheGameEngine->reset`; map.ini-created or overridden power reverts/removes. Negative missing/invalid shipped input remains fatal.
- Read-only retail mission and skirmish initialization: original store count and known source template lookup survive start/reset, with no retail writes and no private names/bytes in evidence.
- Focused source-identity/provider-removal, relevant original entry/simulation/persistence checks, four preset builds and Clang ASan/UBSan as applicable. Full suites remain slice 06.

## Acceptance / commit

Shipped powers survive source reset, map overrides remain scoped, retail input stays read-only, and accepted save/replay slices do not regress. Commit as `delivery: M24 slice 04 preserve shipped special powers`.
