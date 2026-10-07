# Slice 01 — recoverable clean baseline

## Goal and outcome

Restore the explicitly selected original-source baseline while preserving old
history/dirty work and establish one authoritative upstream-only specification.

## Scope and state transitions

Old active main/isolate → verified source snapshots/Git bundle → retired builds
and isolate → exact baseline reset → new policy/plan/findings. Original retail
links remain in place; their targets are never followed. Source code remains
identical to the original baseline in this slice.

## Dependencies and authority

The user explicitly approved the baseline SHA, full milestone replacement and
implementation. This authorizes the otherwise destructive reset after recovery.
No external messages, remote writes, gameplay changes or vendor changes apply.

## Surfaces

Recovery metadata and archives (ignored), local Git HEAD/worktree metadata,
`.gitignore`, `AGENTS.md`, documentation, reset evidence, and this plan.

## Validation and recovery

Positive: both archives match source hashes; Git bundle verifies; tracked source
matches baseline; asset symlink hashes remain unchanged; new docs are coherent.
Negative: snapshot code aborts on special files, changed inventories/status or
hash mismatches before reset. No production behavior changes, so no game tests
are created/run. Inspect documentation diff and original subtree equality.

Recovery uses bundle plus worktree snapshots in a separate directory. All moved
old artifacts remain available. No broad recursive cleanup is permitted.

## Acceptance and commit

Complete after reset evidence, source identity, and the new specification are
verified. One commit: `restart: restore upstream-only port specification`.
The commit containing this file is the non-self-referential slice receipt.
Milestone packet generation and renderer implementation remain separate work.
