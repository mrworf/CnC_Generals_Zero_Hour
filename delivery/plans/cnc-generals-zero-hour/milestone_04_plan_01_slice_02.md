# M4 slice 02 — hardened BIG and deterministic VFS

## Goal and observable outcome

The executable can mount loose roots and BIG archives with the exact required priority and deterministic first-loaded-wins behavior, resolve normalized case-insensitive logical names, and reject ambiguous or unsafe content before exposing it.

## Scope

- Implement a bounded BIGF/BIG4 reader using explicit big-endian decoding and 64-bit checked ranges.
- Discover archives deterministically, accept explicit selected-mod archives/directories, and implement all five precedence layers.
- Index loose and archive entries by normalized logical identity.
- Detect and report case-only loose paths and case-only archive filenames with both physical sources.
- Add synthetic BIG builders/fixtures and positive, negative, and boundary tests.

## Non-scope

Extraction, conversion, file mutation, general package replacement, automatic mod discovery, or metadata interpretation beyond safe entry exposure.

## Dependencies and ordering

Depends on slice 01 resolved roots and M1 fixed-width codec/path primitives. Slice 03 consumes the VFS inventory.

## Entry point and end-to-end behavior

Verification creates a read-only VFS from the resolved roots and optional selected `--mod` path(s), prints archive mount order, and resolves representative logical resources. Loose files take precedence over archive entries according to the five defined layers; archive discovery is ASCII-case-insensitive sorted and the first mounted archive/first entry wins for ordinary duplicates.

## Data/state transitions

Host directory metadata and bounded BIG tables become an in-memory mount/index. Reads are range-limited views copied only on explicit request; no source is rewritten.

## Authorization and permissions

No authorization model applies. Every host access is read-only and confined to the explicitly resolved roots/mod input.

## Validation, error handling, and recovery

Reject absolute, drive-qualified, NUL, dot/traversal names; unsupported identifiers; unreasonable counts/name lengths; truncated names/tables; invalid offsets/sizes; addition overflow; oversized entries/archives; and case-only ambiguities. Diagnostics identify the logical item and non-private source labels. Recovery is fixing/removing the bad input or choosing a different explicit root/mod.

## Expected implementation surfaces

`include/zh/data/big_archive.h`, `include/zh/data/vfs.h`, `src/data/big_archive.cpp`, `src/data/vfs.cpp`, verification integration, `CMakeLists.txt`, and `tests/data/test_vfs.cpp`.

## Tests and commands

- Positive: slash/case normalization; each precedence layer; case-insensitive stable archive order; first-loaded wins; allowed duplicate resources; identical roots.
- Negative/boundary: malformed identifier/count, unterminated/NUL path table, traversal/absolute path, truncated header/table, offset/size/out-of-file and overflow, configured size/count limits, loose and archive-name case collisions.
- Run focused VFS tests then the `data`, `headless`, and `foundation` labels under `linux-gcc-debug`.

## Acceptance criteria

- Exact precedence and deterministic order are observable in tests and verification output.
- No invalid BIG entry is exposed, and no test depends on filesystem enumeration order.
- Existing M0–M3 behavior remains green.

## Commit boundary

Commit this plan artifact with the complete deterministic VFS/BIG behavior and synthetic tests as `delivery: M4 slice 02 add deterministic VFS`.
