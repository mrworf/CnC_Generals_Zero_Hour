# M22 08E1B2: generated mouse provider lifecycle

Close the bounded original `Mouse` singleton cursor-tooltip state used by
`SinglePlayerLoadScreen::update`: it clears the tooltip immediately before
the B4-owned progress/text controls and base E1A presentation update.
Generated headless owners prove the empty-tooltip/default-delay state,
published singleton identity, reset/re-entry, provider removal and zero
ownership across two generations.  The source has no foreign/stale identity
tag at this singleton edge; nonempty tooltip presentation requires the later
DisplayString/layout closure, and the exact full update ordering is therefore
an E1B4 composition gate rather than fabricated here.  No layout, video,
retail or pixels.  Depends on 08E1A; required by E1B.
