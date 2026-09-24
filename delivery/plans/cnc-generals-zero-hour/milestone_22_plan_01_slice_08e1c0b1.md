# M22 08E1C0B1: generated persona and MapCache metadata providers

Use the original `INI::load` dispatch to populate a generated,
project-owned `ChallengeGenerals` and `MapCache` pair. Prove persona lookup by
campaign, general name, and player-template name plus a generated map metadata
entry with player count and start waypoint. Reject missing lookups and malformed
input, update a same-name map entry, release all source providers across two
generations, and prove source archive extraction/removal.

`getMapPreviewImage` is explicitly excluded: it copies preview files through
GameState/GlobalData/filesystem and mapped-image ownership. Source
`positionStartSpots`/`updateMapStartSpots` additionally mutate live GUI gadget
windows. Those coupled resource/GUI lifecycles are C0B2. No layout owner,
retail data, network service, media, or pixels are included here.

Acceptance: focused GCC/Clang/sanitizer probes, six portable full suites,
strict host LSan, physical Vulkan, serial LAN, provider identity/removal,
evidence and one isolated commit.
