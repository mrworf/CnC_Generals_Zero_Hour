# M4 slice 03 — metadata inventory and retail corpus manifest

## Goal and observable outcome

`--verify-data` safely inventories the selected corpus, names required logical resources and detected format/effect expectations, rejects unsupported required formats, and emits a stable logical-only manifest that can be checked into the repository without retail bytes, hashes, or private paths.

## Scope

- Add bounded signature/header classification for INI, CSF, W3D, DDS, TGA, WAV/MP3, Bink, TTF/OTF, RefPack/zlib, and WWShade/effect metadata.
- Traverse the mounted logical namespace deterministically and report formats, locale/font/effect expectations, archive order, missing requirements, and unsupported required content.
- Add a project-owned corpus manifest containing logical names and derived expectations only.
- Enable opt-in retail CTest coverage using the existing three cache variables.
- Characterize unexpected formats under authority section 9; adopt an existing selected decoder path only when supported, otherwise produce a named blocker rather than skipping content.

## Non-scope

Full asset decode/render/playback, extraction, hashing, uploading, conversion helpers, private path persistence, GPU/audio/window initialization, or broad historical-locale support.

## Dependencies and ordering

Depends on slices 01–02. This closes PRE-009 and M4 if the supplied English corpus passes.

## Entry point and end-to-end behavior

The verifier mounts the resolved data, loads the checked-in logical manifest, confirms required entries and expected metadata, inventories all safely exposed logical names, prints stable format/archive/locale summaries, and exits success only when required content is supported. Output root values are runtime-only; repository evidence uses generic root labels and logical names.

## Data/state transitions

Bounded entry prefixes and names become ephemeral metadata records and deterministic textual output. The checked-in manifest is manually reviewed derived metadata, not generated retail content. No retail data changes.

## Authorization and permissions

No authorization model applies. Retail inputs are read-only; tests fail if verification would require a write or device.

## Validation, error handling, and recovery

Each parser checks minimum/maximum sizes and internal dimensions/counts before classification. Unknown optional entries are counted; a required manifest entry with an unknown/unsupported format fails with its logical name. Missing logical assets, corrupt metadata, unsupported Bink/audio/font/texture/compression/effect variants, and discovered NOX/LZH or Granny requirements are explicit failures/blockers per section 9.

## Expected implementation surfaces

`include/zh/data/inventory.h`, `src/data/inventory.cpp`, verifier integration, `data/corpus/english-manifest.tsv`, `tests/data/test_inventory.cpp`, opt-in retail test wiring, and `docs/building-linux.md`.

## Tests and commands

- Positive synthetic header fixtures for every listed family and deterministic manifest output/order.
- Negative/boundary fixtures for truncated/oversized dimensions/counts, mismatched expected format, missing required asset, and unsupported required format.
- Build and run `ctest -L data` for all four presets.
- Configure a separate retail-enabled build with the supplied local roots/language and run verification from an unrelated CWD; confirm no display/GPU/audio access and review logical-only output.

## Acceptance criteria

- Every required format family has bounded positive and negative evidence.
- The supplied English corpus passes, unexpected required formats are classified rather than skipped, and output is stable across CWD/presets.
- No committed diff contains retail bytes, hashes, or private absolute paths.

## Commit boundary

Commit this plan artifact with inventory behavior, manifest, tests, retail wiring, documentation, and logical-only evidence as `delivery: M4 slice 03 verify retail corpus`.
