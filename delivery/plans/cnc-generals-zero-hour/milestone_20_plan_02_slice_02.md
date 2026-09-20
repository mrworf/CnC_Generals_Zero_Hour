# M20 plan 02 slice 02: complete dispatch and live provider closure

## Goal and observable outcome

The complete production `INI::theTypeTable` and every callback/provider body referenced by it compile and link into one executable provider closure. Actual `INI::load` dispatches a representative production block to its original callback and rejects an unknown block. Link success cannot depend on garbage-collecting the table, callback functions, or required provider symbols.

## Scope and non-scope

Promote the complete production `INI::theTypeTable`, its actual callback translation units, and all constructor/vtable/global/registry bodies needed to keep those callbacks live. Include the Linux local-file adapter required to execute a real load. Exercise a production callback with source-owned state plus unknown-block rejection. Full CSF/GameText, GlobalData/name-key initialization, nonempty map metadata/localization/cache behavior, GameState/Xfer/CRC traversal, layered ordering, cache transitions, and write-denial behavior move together to slice 03. Bounded engine update remains slice 04. Do not substitute M27's selected fixture callbacks for the production table.

## Dependencies and ordering

Requires slice 01 plus accepted M27/M28 seams. Supplies a live complete callback/provider closure required by slice 03. This ordering was amended after a real `INI::load` link proved that the former slice 02 could not execute before its callbacks' provider bodies.

## Entry point and end-to-end behavior

An owned INI is opened through the production Linux local-file adapter and parsed by actual `INI::load`. The full table remains live in the executable, a representative original callback changes its source-owned value, and an unknown block throws the original failure. The same `setFPMode` accepted in M26/M27 is consumed in this production load.

## State and permissions

The owned fixture is immutable and the dispatch probe performs no generated writes. The local-file adapter normalizes legacy separators without mutating read roots. No authorization model applies.

## Validation and recovery

Test a valid production block and an unknown block through actual `INI::load`; inspect the executable/link map for the table, selected callback, and every required provider with no unresolved or discarded acceptance symbol. Removing one callback/provider must make the real link or load fail. Data-specific malformed/missing/cache cases remain slice 03.

## Expected surfaces

Actual original INI and callback/provider sources, Linux local-file adapter, owned dispatch fixtures, link/runtime identity and provider-removal controls, ledger/classification, and provenance controls.

## Required validation and acceptance

Focused dispatch/link tests and identity/removal controls pass in all four presets. Complete dispatch is present and live; a source-owned callback value is observed; an unknown block fails nonzero; provider removal fails a real link/load gate. No dead/discarded section or reduced fixture table satisfies acceptance. Commit as `delivery: M20 slice 02 link complete original dispatch`.

## Delivered evidence

- `zh_original_config_spine` force-links the actual `Common/INI/INI.cpp` object; the callback/provider closure is an ordinary `zh_original_config_providers` archive, so the link map records only members genuinely extracted for the live table.
- The identity gate derives all 64 production callbacks directly from `theTypeTable`, requires every live symbol in the executable, maps every external callback to one unambiguous target compile command and an extracted archive member, and executes the valid fixture. Presence in an archive or discarded section cannot pass.
- The positive probe calls actual `INI::load` through `FileSystem` and `PosixLocalFileSystem`; `OnlineChatColors` changes the production `GameSpyColor` table. The negative fixture reaches the same load and rejects an unknown production block.
- The removal control first proves `Chat.cpp.o` was extracted, deletes that member from a copied provider archive, and requires the real dispatch/probe relink to fail specifically for `INI::parseOnlineChatColorDefinition` or `GameSpyColor`.
- GCC Release exposed a wider real vtable/state closure than Debug. The additional ActionManager, Squad, AI dock/guard/tunnel, supply/hack/prone, resource/tunnel, and Line2D translation units remain M20 startup-link work; no behavior acceptance assigned to M21-M25 was claimed.
- `original_config_dispatch_valid`, `original_config_dispatch_invalid`, `original_config_dispatch_identity`, and `original_config_provider_removal` pass 4/4 in `linux-gcc-debug`, `linux-gcc-release`, `linux-clang-debug`, and `linux-clang-release`.
- `original_lifecycle_enum_abi`, the checked enum inventory, source classification, and dependency-ledger freshness remain green. The Posix local-file pool entry is required for the actual load and preserves the original allocator rather than bypassing it.
