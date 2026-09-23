# M22 08E1B4: generated single-player layout/mission/text/audio ownership

## Source revalidation and dependency order

`SinglePlayerLoadScreen::init` is not a single owner boundary.  Its source
sequence first invokes `GameWindowManager::winCreateFromScript` for the named
layout, obtains the progress/objective/unit/location nodes through the original
name-key lookup, reads the current `CampaignManager::Mission`, translates its
labels via `GameText`, uses the already accepted B3 video route, assigns
mapped-image references, and finally registers an ambient `AudioEventRTS`.
`moveWindows` separately dispatches the briefing voice.  The concrete WND
parser also depends on FileSystem, the full GameWindow allocation/gadget
factory, and script callbacks; it must not be hidden behind a fake retail
layout.

This is therefore split before implementation:

1. **08E1B4A** — original named WND parser and generated bounded
   SinglePlayer layout-tree contract.  It owns only project-owned generated
   script bytes, the source FileSystem/layout parse, NameKey identity, root and
   exact required basic gadget nodes.  It has no mission, text, audio, video,
   mapped-image assignment, mode owner, retail layout, or pixels.
2. **08E1B4B** — original generated CampaignManager/Mission descriptor
   ownership for the bounded label/movie/voice fields used by this screen.
3. **08E1B4C** — generated GameText lookup and source static/progress gadget
   text application for the bounded mission labels.
4. **08E1B4D** — generated AudioEventRTS ambient/briefing registration,
   removal, and failure/re-entry lifecycle.
5. **08E1B4** — original `SinglePlayerLoadScreen` owner composition using
   accepted B1–B3 and B4A–B4D, with a generated profile only.

No retail layout, media, names, bytes, audio fidelity, raw Direct3D, or pixel
claim is admitted by these slices.  Every generated asset is project-owned.

## 08E1B4B: generated Campaign/Mission descriptor ownership

The current executable slice uses the source `CampaignManager`/
`Campaign`/`Mission` factory and current-selection transaction with only
project-owned in-memory descriptor values.  It deliberately excludes the
file-backed INI parse edge, GameText, static gadget data, audio, and the
`SinglePlayerLoadScreen` mode owner.  The detailed acceptance contract is
persisted in `milestone_22_plan_01_slice_08e1b4b.md`.

## Completed 08E1B4A

The first slice validates only a generated root/progress child through the
actual original parse and NameKey lookup edge.  The existing missing-file and
foreign-callback fixture controls remain applicable; parser malformed and
duplicate semantics are preserved for later bounded validation rather than
rewritten.  Its destruction joins the existing two-generation reset/removal
proof with no linked windows.  It will use no source `SinglePlayerLoadScreen`
owner until the Mission, text, and audio prerequisites are present.
