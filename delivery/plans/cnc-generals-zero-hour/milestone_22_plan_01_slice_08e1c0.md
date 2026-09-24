# M22 08E1C0: generated multiplayer load-screen data/provider closure

## Revalidated dependency boundary

The original `MultiPlayerLoadScreen::init(GameInfo*)` cannot be exercised from
the accepted E1A/E1B providers alone. Before the owner route it dereferences a
local `GameSlot` and `PlayerTemplateStore`, maps multiplayer house colours,
resolves ChallengeGenerals/persona and text/image values, creates the full
MAX_SLOTS named layout, performs map-cache/preview and map-start helpers, and
initializes GameLogic timeout state. Its update route separately needs either
Network progress publication or GameLogic/GameInfo progress processing.

This prerequisite is dependency-split: **08E1C0A** supplies GameInfo/slots/
templates/settings; **08E1C0B** supplies persona/map helpers after A; only
then may 08E1C compose the owner. The prerequisite must first provide one generated, project-owned provider
transaction for those exact source values, with occupied/empty/AI slots,
template/persona fallback, color/image fallback, map-preview absent/present,
network/logic selection, two-generation removal and zero ownership. It must
not claim retail maps, lobby/network service, media, pixels, or audio fidelity.
Only after that proof can 08E1C compose the actual owner without fabricated
direct calls or null dependencies.

## Aggregate transaction

This final C0 aggregate adds no production source. Its generated witness orders
accepted C0A GameInfo/GameSlot and parser-backed settings before accepted C0B
persona/MapCache metadata, preview, and source start-marker helpers; each must
report its two-generation zero-provider result. Missing or altered witnesses
fail deterministically, while component slices retain direct negative, retry,
removal, and teardown controls. It proves no owner composition, retail
content, network service, media, pixel, or audio claim.
