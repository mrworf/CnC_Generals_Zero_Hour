# M27 plan 01 slice 03: map metadata, XDG cache and source gates

## Goal and observable outcome

Owned map fixtures produce deterministic independent metadata and CRCs through reusable source-owned MapUtil operations. Cold/warm/stale cache behavior is equivalent, generated state is XDG-confined, and checked gates prevent provider/source/ownership drift.

## Scope

- Extract only independently reusable MapUtil metadata operations: map enumeration, CRC, WorldInfo, ObjectsList framing where no ThingFactory callback is needed, height/border metadata, and cache-path/output primitives.
- Preserve standard/user map precedence and separator/case normalization through the M4 VFS.
- Derive cache output from `XDG_CACHE_HOME`, use bounded atomic replacement, and expose cold/warm/stale results.
- Extend the M26 dependency ledger and identity/provider-removal controls across every accepted M27 provider.
- Produce committed M27 QA evidence and run cumulative acceptance.

## Non-scope

`ParseObjectDataChunk`, ThingFactory/MapObject creation, nonempty object classification, `addMap`, `INIMapCache`, `map.str` localization, full cold/warm production MapCache equivalence, retail map metadata, or M20 startup.

## Dependencies and ordering

Depends on slices 01-02. This is the milestone-closing slice and owns full four-preset, sanitizer, and asset-free regression validation.

## Entry point and behavior

The focused executable mounts owned standard and user-map fixtures, enumerates effective maps, parses independent chunk metadata, computes CRCs, resolves a cache beneath an injected XDG root, and compares cold rebuild, warm read, and stale rebuild results. Failed parse/write paths report bounded errors without mutating read roots or a previously valid cache.

## Data/state transitions

Map bytes + logical identity -> validated metadata -> deterministic cache record -> atomic cache replacement. Warm cache bytes are validated before use. Stale or malformed cache/map data triggers a bounded rebuild or explicit failure; no partial state becomes visible.

## Authorization and permissions

No authorization surface. Read roots remain immutable. Writes are permitted only beneath the injected XDG cache root after canonical containment checks.

## Validation and error handling

Positive tests cover empty-object independent maps, WorldInfo/height/borders, map CRC, standard/user precedence, map enumeration, cold/warm equality, stale rebuild, arbitrary CWD, and deterministic cache path. Negative tests cover truncated/malformed/oversized chunks, nonempty ObjectsList deferral, traversal, cache corruption, read-only/write-denied cache roots, and unchanged read-root snapshots.

## Expected implementation surfaces

`CMakeLists.txt`; `include/zh/original_data.h`; original-tree MapUtil extraction; tests and identity utilities; dependency ledger/classification checks; `evidence/qa/cnc-generals-zero-hour/` and all M27 plan result fields.

## Required commands

- Focused `original-data` tests in all four GCC/Clang Debug/Release presets.
- Canonical full asset-free CTest in all four presets after stabilization.
- Focused Clang Debug ASan/UBSan with exact `ASAN_OPTIONS`/`UBSAN_OPTIONS` recorded.
- Compile-command, link-map/live-symbol, runtime-witness, source-drift, provider-removal, and ownerless-edge negative controls.

## Acceptance criteria

- Independent metadata is identical for cold/warm/stale rebuilds and across presets.
- All generated files remain below XDG cache and no read-root snapshot changes.
- Ledger assigns every remaining coupled INI/MapUtil/startup edge to M20 and distinguishes inspected from runtime evidence.
- Removing or drifting any accepted M27 provider fails real validation.
- Full asset-free and focused sanitizer suites pass; fixture failures never skip because retail is absent.

## Commit boundary

One commit containing map/cache behavior, assurance gates, cumulative evidence, and completed plan results. Outer milestone status files remain excluded for orchestrator-owned acceptance.
