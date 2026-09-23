# M22 08E1B4C: generated GameText and static-gadget text closure

## Source boundary and dependency order

`SinglePlayerLoadScreen::init` reads the B4B-published mission labels through
`GameTextInterface::fetch` and assigns the resulting `UnicodeString` to
source static gadgets with `GadgetStaticTextSetText`.  The B4A root/progress
tree cannot support this alone: `GameWindowManager::gogoGadgetStaticText`
copies parsed `TextData` and acquires a `DisplayString` from
`DisplayStringManager`; `GadgetStaticTextSystem` then owns its release on
`GWM_DESTROY`.

This closure is still bounded and does not require the concrete retail GameText
database: the original abstract interface is supplied by the existing
project-owned headless provider, while the actual source WND parser,
`TextData` allocation, static gadget system-message path, and destruction
remain live.  No source dependency warrants another split.

## Delivered transaction

Extend the generated B4A WND fixture with one callback-free named
`STATICTEXT`, configure a project-owned headless `DisplayStringManager`, and
run two generations of B4B-shaped synthetic mission label → `GameText::fetch`
→ source static-gadget assignment → source text retrieval → layout destruction.
Cover missing label (empty result), null window no-op, malformed callback
layout rejection, duplicate generated assignment replacement, provider
removal, reset/re-entry, and zero linked display-string ownership.  Foreign
or stale text data has no public source admission API and is rejected
structurally rather than emulated with a second owner.

Mission INI parsing, retail text data, audio, mapped-image assignment, video
composition, and `SinglePlayerLoadScreen` itself remain deferred.  All labels
and WND bytes are project-owned and generated.

## Validation

Focused GCC/Clang source and provider-removal checks, six final-tree broad
suites, strict sanitizer leak checks, Vulkan, LAN, ledger/diff, evidence, and
an independent exact-path commit are required.
