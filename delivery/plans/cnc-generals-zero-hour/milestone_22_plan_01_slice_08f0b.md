# M22 plan 01 slice 08F0B: original display smart asset-purge handoff

## Goal and observable outcome

The live Linux `W3DDisplay::doSmartAssetPurgeAndPreload` method performs the
original map-INI handoff through source-owned asset management.  A generated
map override can invoke it with an absent or project-owned usage list, and the
display/asset owners remain valid across reset and a fresh generation.

## Scope and non-scope

Implement the source method's optional text-list read and
`Free_Assets_With_Exclusion_List` call with the original list semantics.
Keep the original early return for absent asset manager or empty name when
there is no active display owner.  Fail closed on a broken published owner or
required file-system provider; do not return successful placeholder behavior.  Use only
generated assets and usage files for executable tests.  This is a prerequisite
to 08F0's selector transition, not permission to construct a terrain or scene.

Exclude retail provider traversal, retail names/paths/bytes/hashes, visual or
audio fidelity, map terrain load, and post-parser construction.

## Dependencies and order

08F1 supplies the generated `loadMapINI` parser/provider contract.  This
slice closes the display handoff it reaches before 08F0 admits the live
post-08D transition.  Keep accepted M21/M24 headless behavior and default
display guards intact.

## Entry point, state, authorization and recovery

`GameLogic::loadMapINI` calls the display method even when the optional
`AssetUsage.txt` file is absent.  The display reads tokens from the file when
present, skips source-defined comment tokens, and passes the resulting list to
the original asset manager.  With no file, it passes an empty exclusion list.
The source owns read-only inputs; XDG process writes remain isolated.  No user
authorization is involved.  An absent optional file yields the original empty
list.  Broken published owners and provider failures must leave the asset
manager, display aliases and file handles recoverable for reset/retry and
teardown.

## Implementation surfaces

- `GeneralsMD/Code/GameEngineDevice/Source/W3DDevice/GameClient/W3DDisplay.cpp`
  and only directly required source owner adapters.
- Generated display/asset-manager fixture and focused positive/negative test.
- This plan, governing index, dependency ledger if source imports change, and
  a sanitized evidence artifact.

## Positive and negative validation

Prove absent optional file triggers source purge with an empty exclusion list;
present owned file preserves exactly its listed assets and removes others.
Test comments, absent optional file, missing or inconsistent owners, provider
failure, reset/retry, two fresh
generations, and zero residual owners/resources.  Run focused GCC/Clang and
sanitizer checks, source identity/provider/ledger gates, then the repository's
required six-config non-GPU/non-LAN matrix, strict host leaks, proportional
public Vulkan, serial LAN, and `git diff --check` before commit.

## Acceptance and commit boundary

The native source method completes with observed owner transitions; all
negative cases fail closed and teardown is clean.  Commit this complete
behavior and its plan/evidence in one reviewable `delivery: M22 08F0B close
display smart purge` commit.  Then resume 08F0; do not combine the commits.

## Execution record

Implemented only the CPU-branch original display method: source token scan,
comment filtering, optional-file empty-list purge, and native asset-manager
handoff. A published but incomplete display owner fails before any purge;
null/empty names and the never-initialized display retain the source return.
The generated two-root fixture tests exact exclusion behavior, provider/owner
negative controls, reload, reset, and two clean generations. The result is
recorded in [08F0B evidence](../../evidence/cnc-generals-zero-hour/milestone_22_slice_08f0b.md).
08F0 remains pending and owns the separate selector transition.
