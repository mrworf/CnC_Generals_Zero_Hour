# M22 08E1B4B: generated Campaign/Mission field owner

## Authority and boundary

`SinglePlayerLoadScreen::init` reads only the selected source
`CampaignManager::Mission` before the later GameText, static-gadget, video,
mapped-image, and audio edges.  The original owner chain is:

`CampaignManager::newCampaign` (case-normalized campaign replacement)
→ `Campaign::newMission` (case-normalized mission replacement)
→ `CampaignManager::setCampaignAndMission` (published current owner)
→ `CampaignManager::getCurrentMission`.

The source INI `Campaign`/`Mission` parser is deliberately outside this slice:
it loads file-backed campaign configuration and is not needed to establish the
owner/selection semantics consumed by the load screen.  This slice supplies
only project-owned in-memory descriptors and does not read retail data or
record campaign assets, labels, media, audio, paths, bytes, or hashes.

## Delivered behavior

Extend the existing headless original-provider executable with a two-generation
generated descriptor transaction.  Each generation creates a campaign and
mission through the original factory methods, assigns bounded synthetic
load-screen fields (objective/unit/location/movie/voice-length), publishes it
through the original case-insensitive selection route, and verifies exact
identity and field retention.  It also exercises source duplicate replacement,
unknown/empty selection failure, next-mission absence, re-selection, provider
removal, and unpublication after destruction.  The original memory-pool
allocator intentionally retains pool backing storage, so raw-allocation count
is not treated as object-ownership evidence.

There is no public API accepting foreign or stale `Mission*`; those conditions
are structurally rejected by the source owner model rather than fabricated as
a second owner.  GameText conversion, layout text application, audio dispatch,
and the `SinglePlayerLoadScreen` owner remain deferred to B4C/B4D/B4.

## Validation

- focused GCC and Clang headless source test plus the dedicated
  `CampaignManager.cpp` provider-removal relink test;
- fresh six-configuration non-GPU/non-LAN matrix;
- GCC and Clang leak-detection host tests, physical Vulkan, and serial LAN;
- dependency ledger, whitespace/diff checks, evidence/index update, and exact
  staging while preserving the unrelated renderer diagnostic.
