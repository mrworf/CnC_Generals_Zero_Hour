# M20 plan 02 slice 03: complete data and configuration startup

## Goal and observable outcome

Original filesystem/archive integration, CSF/GameText, GlobalData/name keys, map cache/localization, and GameState/Xfer/CRC startup consumers load owned nonempty fixtures through the live complete dispatch from slice 02. Source-owned values and objects are observed, and generated cache/benchmark state appears only below isolated XDG roots.

## Scope and non-scope

Execute the original GameText/CSF, GlobalData/name-key, MapUtil/INIMapCache, GameState/Xfer/CRC, filesystem/archive, and registry-dependent configuration paths now that every production callback provider is live. Exercise cold/warm/absent/stale cache, read-only inputs, denied writes, original two-pass INI directory ordering, malformed/missing data, and representative configured module/object resolution. Later full scenarios, hardware rendering, interactive UI/media, save/replay scenarios, and LAN matches remain M21-M25 acceptance.

## Dependencies and ordering

Requires slices 01-02 and accepted M27/M28. Supplies initialized data stores and configured objects for slice 04 integrated update/reset.

## Entry point, state, and permissions

The original initialization sequence creates file/archive systems, GlobalData and name keys, loads owned INI/CSF and nonempty map/map.str fixtures, builds or refreshes the original cache, and traverses GameState/Xfer/CRC startup relationships. Cache state transitions absent/stale -> generated -> warm equivalent below `XDG_CACHE_HOME`; permission denial preserves source data and diagnostics. Offline network remains null.

## Validation and recovery

Test valid layered data, representative configured modules/objects, malformed/truncated INI/CSF/map data, missing resources, unknown required modules, standard/user precedence, arbitrary CWD, denied writes, and Debug/Release cache behavior. Initialization failures unwind slice-01 ownership and do not enter execute/reset. Empty maps, M27-only fixture parsing, and proxy cache/localization state fail assurance.

## Expected surfaces

Original filesystem/archive/INI/GameText/GlobalData/MapUtil/GameState/Xfer/CRC sources, complete live callbacks from slice 02, Linux VFS/XDG adapters, owned nonempty fixtures, lifecycle tests, source classification/dependency ledger, and identity/runtime controls.

## Required validation and acceptance

Focused data-startup tests pass in all four presets; source-owned localization, name-key, configured object, nonempty map/cache, Xfer and CRC values are observed; cold/warm results match; all writes are XDG-isolated; malformed/missing/unknown/denied cases fail nonzero without execute/reset. Commit as `delivery: M20 slice 03 integrate original data startup`.

## Delivered implementation notes

- The runtime test force-links the production INI dispatch and observes actual `GlobalData`, `NameKeyGenerator`, `GameTextManager`, `ThingFactory`/`ThingTemplate`, `INIMapCache`, `GameState`, `Snapshot`, and `XferCRC` state. The map catalog fixture is nonempty: localized metadata includes two starts plus tech and supply positions; it is not an empty-map or M27 proxy witness.
- `PosixLocalFileSystem` now accepts an explicit read root, preserving original logical paths while making reads independent of the process CWD. Loose-file enumeration retains logical relative names and the archive provider remains an optional second source rather than a required null dereference.
- Generated standard and user `MapCache.ini` files now use atomic replacement below the resolved XDG cache root. Cold, warm, absent, refreshed, malformed, and denied-write paths are exercised without changing source data.
- The live-link identity test requires eight exact provider members and source symbols, and provider-removal deletes the actually extracted `XferCRC.cpp.o`; the real data-startup relink must then fail for the XferCRC symbol.
