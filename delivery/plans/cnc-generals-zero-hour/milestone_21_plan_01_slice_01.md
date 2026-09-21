# M21 plan 01 slice 01: original map and chunk state

## Goal and observable outcome

The production original-simulation target opens a valid initial `.map` through original file/chunk parsers and publishes nonempty source-owned map objects, world properties, terrain/waypoints, sides, and scripts. A missing file or malformed/truncated/unknown-required chunk fails without partially published state.

## Scope

Included: narrow extraction/shared compilation of original CPU `MapObject` and map-object chunk behavior from W3D code; original `DataChunkInput`, terrain, side, polygon/script and map dispatch; owned mission/skirmish map fixtures; state witness and negative controls. Excluded: object instantiation, simulation commands/updates, renderer behavior, retail validation.

## Dependencies and ordering

Requires accepted M20 and precedes all other M21 slices. Preserve its production VFS, allocator, original registries, and failure unwinding.

## Entry point and end-to-end behavior

An M21 scenario request reaches the production original entry, resolves the initial `.map` through the existing original filesystem, invokes the original chunk parser registrations, and reports derived counts only after all required chunks succeed.

## Data and state transitions

Empty pre-scenario state becomes a committed map state containing original `MapObject`, world dictionary, side/script, terrain and waypoint data. Any parse/open failure restores the empty state and allocator/resource baselines.

## Authorization and permissions

No user authorization applies. Owned fixtures are repository data. Data roots are read-only inputs and state writes remain within test-owned XDG roots.

## Validation and error handling

Positive: mission and skirmish fixture chunks produce expected original counts/keys. Negative: absent map, truncated header/payload, invalid chunk length/version, missing required module/provider, and parser removal cannot report success or retain partial state.

## Implementation surfaces

Expected: original `WorldHeightMap`/`MapObject` CPU source organization, GameLogic/TerrainLogic map dispatch where narrowly necessary, `CMakeLists.txt`, `src/original_runtime`, `tests/original_simulation`, owned fixtures, identity/provider-removal tooling, and source classification/ledger entries.

## Required commands

- Configure/build `linux-gcc-debug`; run focused `original-simulation` map/parser and negative tests.
- Run focused Clang ASan/UBSan parser tests before commit.
- Run identity and provider-removal controls against the production-linked target.

## Acceptance criteria

- Witnesses are original state objects created by original parsers, not test labels or duplicate models.
- Both fixture modes publish nonempty state with stable expected derived counts.
- Every negative case fails before success publication and returns allocation/resource counts to baseline.
- Removing the original map provider or parser breaks the gate.

## Commit boundary

Commit the complete governing/slice plan set with the original map/chunk behavior, fixtures, tests, and focused evidence as one slice commit.

## Delivered evidence

Status: complete.

- One canonical original CPU loader is consumed by Linux terrain and W3D logical construction; duplicate W3D callbacks were removed.
- Owned mission and skirmish `CkMp` fixtures publish original terrain, world, `MapObject`, side/team, and nested script-list state and replace that state on re-entry.
- Malformed, truncated, and missing inputs roll back published object/side state; provider removal breaks the production link.
- GCC Debug focused identity/negative gates passed 3/3, and strict Clang ASan/UBSan passed 1/1 after correcting reached original teardown, alignment, and packed-key undefined behavior.
- Evidence: `evidence/qa/cnc-generals-zero-hour/M21-plan01-slice01-original-map.md`.
