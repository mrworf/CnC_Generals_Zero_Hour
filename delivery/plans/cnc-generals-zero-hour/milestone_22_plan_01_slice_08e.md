# M22 slice 08E: original load-screen progress lifecycle

## Goal

Close the first original consumer reached after 08D: the optional
`GameLogic::updateLoadProgress(LOAD_PROGRESS_POST_PARTICLE_INI_LOAD)` call
after reset bookkeeping and before `loadMapINI`/map loading.  The selector may
admit only the exact source `LoadScreen` returned by the existing game-mode
factory; absence remains the original no-op.

## Scope and guards

- Retain original `LoadScreen` selection and ownership.  Add no retail assets,
  GUI layout, rendering, map load, or permissive fallback.
- Bound an exact progress update/re-entry lifecycle for the selected owner.
  Default, foreign, stale, duplicate, unsupported type and uninitialized
  owner paths fail closed; legitimately absent owner remains the source no-op.
- Generated fixtures prove update ordering, failure rollback/retry, reset and
  two-generation provider removal/zero ownership.  The read-only retail audit
  emits only an aggregate post-progress advance marker and verifies rollback.

## Dependencies and acceptance

08E depends on 08E1 and is a prerequisite of 08.  It must use generated
fixtures for detailed behavior, preserve retail confidentiality, then complete
the proportional matrix, leak/Vulkan/LAN checks, ledger/evidence/index, and
one independent commit.  Subsequent map-INI or map-load consumers remain
separate dependency work.
