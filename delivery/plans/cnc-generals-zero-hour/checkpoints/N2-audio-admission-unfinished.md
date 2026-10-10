# N2 audio admission worktree checkpoint

The user requested working-tree cleanup. Nine pending audio source files were
an incomplete, unvalidated implementation, not a delivered behavior slice.
They are preserved verbatim in `N2-audio-admission-unfinished.patch`. Active
source was restored to parent `91340a607823c8053717fd6b58f9d64372eaab60`.
The existing slice03 audio contract remains the governing plan. This checkpoint
changes no executable behavior and claims no acceptance.

## Resume boundaries

- Read original-engine format notes and slice03 audio findings first.
- Review the patch before applying; it is a draft, not a build dependency.
- Complete the SoundManager declaration/definition change, initialize all four
  sample counters, and add the new source to the original-runtime build graph.
- Integrate admission and guarded event/request ownership across GameAudio,
  GameMusic and GameSounds together, including rejection, queue draining and
  paired manager startup. The draft alone does not fix those production paths.
- Preserve legitimate rejection handle/RNG semantics, ancestor rollback and
  caller lifetime. Test queue payload transfer and whole-owner cleanup.
- Use actual parsed generated audio roots and original managers, not fabricated
  playback success. Validate focused and related families with GCC and Clang
  ownership sanitizers before committing executable behavior.

## Verification and provenance

The preserved patch passed reverse applicability against the dirty source and
forward applicability against the restored source. Tracked audio source matches
the parent after cleanup. No test was run on the incomplete draft. No assets,
vendor libraries, recovery files or milestone acceptance/status were changed.
N2 slice03 remains in progress.

Cleanup commit locator:
`git log -1 --format=%H -- delivery/plans/cnc-generals-zero-hour/checkpoints/N2-audio-admission-unfinished.md`.
