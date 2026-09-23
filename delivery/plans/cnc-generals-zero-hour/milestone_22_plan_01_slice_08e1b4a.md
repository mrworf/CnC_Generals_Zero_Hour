# M22 08E1B4A: original generated named load-screen layout closure

## Goal

Parse one project-owned generated SinglePlayer layout through the original
`GameWindowManager::winCreateFromScript` path and expose its exact root and
progress child to original name-key lookup.  This makes
the source layout boundary independently observable before a
SinglePlayerLoadScreen owner can consume campaign, text, audio, or video state.

## Scope and constraints

Included source surfaces are `GameWindowManagerScript.cpp`, the minimal
GameWindowManager allocation/tree operations, NameKey generation, FileSystem
logical-file input, and the original progress gadget state needed by the
generated tree.  Static text requires the separate `TextData` contract and is
deliberately B4C.  The fixture owns only project-authored WND text and
never names or reads retail assets.  It does not introduce a load-screen mode
owner, Mission/CampaignManager, GameText, AudioEventRTS, mapped image, video,
renderer texture, or pixel result.

## Behavior and failures

The generated script is accepted only with a callback-free root and the fixed
progress child type.  Source name-key lookup proves child identity and source
gadget state mutation.  The existing generated missing-file and
foreign-callback controls remain fail-closed; malformed script handling and
duplicate/case semantics are source-parser behavior and are not rewritten by
this slice.  Layout destruction proves no linked windows remain before the
existing two-generation headless reset/removal transaction.

## Validation

Extend the existing source-headless CTest, which already owns the matching
generated missing-file/foreign-callback and two-generation lifecycle controls.
Run focused GCC/Clang first, then the mandated
final-tree six configuration non-GPU/non-LAN suites, host leak checks, Vulkan,
LAN, dependency-ledger/diff checks, and record evidence.  Commit only this
slice's plan, implementation/test/build wiring, index, and evidence.
