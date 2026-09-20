# M20 plan 02 slice 02: complete data and configuration startup

## Goal and observable outcome

Original filesystem, complete INI dispatch, CSF/GameText, GlobalData/name keys, map cache, and Xfer/CRC startup consumers load owned nonempty fixtures and produce source-owned values through the lifecycle spine. Generated cache and benchmark state appears only below isolated XDG roots.

## Scope and non-scope

Promote actual Local/Archive/FileSystem implementations through the accepted VFS seam; the complete production `INI::theTypeTable`; GameText/GlobalData/name-key initialization; GameState/Xfer/CRC startup relationships; and registry-dependent nonempty map metadata/localization/cache paths. Exercise cold/warm/absent/stale cache, read-only inputs, denied writes, two-pass INI directory order, and malformed/missing data. Factory/provider completion and bounded engine update remain slices 03-04. Do not substitute M27's selected fixture callbacks for the production table.

## Dependencies and ordering

Requires slice 01 plus accepted M27. Supplies configured objects and callbacks required by slice 03.

## Entry point and end-to-end behavior

The original engine initialization sequence creates file systems, GlobalData and name keys, loads owned fixture INI/CSF, runs complete callback dispatch, creates GameState/XferCRC relationships, and refreshes a nonempty localized map cache. The same parser/setFPMode methods accepted in M26/M27 are linked into production.

## State and permissions

Logical read roots are immutable. Cache state transitions absent/stale -> generated -> warm equivalent below `XDG_CACHE_HOME`; permission denial leaves read data unchanged and preserves diagnostics. No authorization model applies beyond filesystem isolation.

## Validation and recovery

Test valid layered data, unknown fields/modules, malformed/truncated INI/CSF/map data, missing resources, standard/user precedence, arbitrary CWD, denied writes, and Debug/Release cache behavior. Initialization failures unwind slice-01 ownership and do not enter execute/reset.

## Expected surfaces

Actual original filesystem/INI/GameText/GlobalData/MapUtil/GameState/Xfer/CRC sources and callbacks, Linux VFS/XDG adapters, owned fixtures, lifecycle tests, ledger/classification, and provenance controls.

## Required validation and acceptance

Focused data-startup tests and identity/removal controls pass in all four presets; source-owned values and nonempty objects are observed; complete dispatch is present; cold/warm results match; all writes are XDG-isolated; malformed/missing/denied cases fail nonzero without execute/reset. Commit as `delivery: M20 slice 02 integrate original data startup`.

