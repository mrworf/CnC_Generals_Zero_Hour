# M24 slice 02: transactional original restore

## Goal and observable outcome

Loading a valid original save restores the real map, objects, player/team, scripts and AI, then resumes equivalent source-owned checkpoints. Invalid or injected-failure loads preserve the exact pre-load state and cleanup.

## Scope / non-scope

Own source `XferLoad`, `GameState::loadGame`, `GameStateMap` extraction, original postprocess and live backup/rollback. Do not substitute `ZHSG`, bypass original callbacks or implement replay/M22.

## Dependencies, entry and state

Slice 01 save. Production load selects a validated XDG save. Preflight source block framing/version/reference/embedded-map bounds before reset; original Xfer backup preserves live state. On success original reset → xfer → postprocess publishes loaded state. On any failure, original reset/reload of backup restores prior map, object identities/state values, RNG and frame/checkpoint; transaction-owned scratch files are removed and flags/locks restored.

## Permissions, validation and errors

Local save names only; no retail writes. Reject truncated descriptor/body, unknown required version, duplicate/missing required block, corrupt map, invalid object reference, path escape and oversized lengths before publication. Failure after reset, mid-traversal and postprocess must leave no mixed state. Preserve an actionable noninteractive diagnostic.

## Expected surfaces

Original `XferLoad.cpp`, `GameState.cpp/.h`, `GameStateMap.cpp`, affected original snapshot loaders as source-evidenced, Linux scenario hook, focused tests/fixtures, ledger. Investigate block extents/reference semantics before edits.

Investigation finding: the first valid source load failed because the original save path appended a Win32 backslash to an XDG directory and lowercased the resulting absolute path. Raw map extraction wrote a literal-backslash Linux filename while the production VFS normalized backslashes to separators. The narrow native correction is a slash-terminated XDG save directory, slash-aware map-leaf extraction, and preservation of absolute host path case; the portable `Save\\` token and original block schema remain unchanged. The original invalid-load modal message also dereferenced absent headless window chrome, so bounded Linux diagnostics must precede rollback/error return.

Source traversal findings: GameLogic v10 omitted CRC-visible simulation RNG and prepending loaded objects reversed source update/CRC order. The source AI/pathfinder state was not in `SNAPSHOT_SAVELOAD` although it contributes to logic CRC. Linux v11 GameLogic now carries fixed-width random words, restores serialized object order, and includes a bounded source AI/pathfinder block. A corrupt TOC template reached unsafe partial-object teardown before rollback; preflight now validates source TOC and object IDs before reset. Extracted maps publish atomically; only transaction-owned scratch maps are cleaned, and a pre-existing user map collision is rejected before reset.

## Tests and commands

- Positive: round-trip after commands, compare original object/player/team/script/AI and CRC/checkpoints, resume frames; repeated load/reset/re-entry.
- Negative: truncate at block boundaries, unknown version, corrupt map, invalid reference, path escape, injected after-reset/traversal/postprocess failures; verify exact pre-load state/checkpoint and no stray XDG scratch file.
- Focused original-persistence tests, related original-simulation/lifecycle, Clang ASan/UBSan; four-preset/full suite at gate.

## Acceptance / commit

Every valid load restores source state; all failure classes preserve pre-load state and cleanup. Commit as `delivery: M24 slice 02 restore original saves transactionally`.
