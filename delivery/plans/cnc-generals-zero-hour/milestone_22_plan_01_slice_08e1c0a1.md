# M22 08E1C0A1: generated GameInfo/GameSlot lifecycle

Exercise the original in-memory GameInfo/GameSlot constructors, slot install,
human/AI/open state, local-slot selection, invalid lookup and two-generation
reset/removal. Map selection requires the MapCache provider and remains C0B;
no player-template/settings parser or multiplayer owner route is included.

`GameInfo::setSlot` is deliberately non-owning: callers first publish a live
slot with `setSlotPointer`, after which the original source copies values into
that owner. The generated probe proves that contract, rejects invalid slot
lookups, clears both occupied slots, unpublishes `TheGameInfo`/GameText/global
data after each generation, and uses a link-removal control for `GameInfo.cpp`.

Acceptance: focused GCC/Clang and sanitizer provider/removal probes; fresh
portable non-GPU/non-LAN suite in six configurations; strict host LSan in both
sanitizers; host Vulkan and serial LAN six-build checks. Commit this slice with
the C0 dependency ordering and its evidence only.
