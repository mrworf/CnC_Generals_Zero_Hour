# M22 08E1B4B: generated Campaign/Mission descriptor evidence

This slice exercises the actual original `CampaignManager` owner chain with
project-owned in-memory descriptors only:

`newCampaign` → `newMission` → `setCampaignAndMission` →
`getCurrentMission`.

The generated two-generation witness retains the bounded map, movie,
objective, unit, location, and voice-length fields used by the later
single-player load-screen owner; verifies case-normalized campaign and
source-lowercase mission selection; rejects empty/unknown selection; stops a
missing next-mission traversal; replaces duplicate mission and campaign owners;
and unpublishes `TheCampaignManager` after each generation.  A dedicated
negative relink proves `CampaignManager.cpp` is live.  The original memory
pool deliberately retains backing slabs, so zero ownership is asserted at the
owner/publication boundary rather than by raw allocator count.

The file-backed Campaign INI parser, GameText/gadget text, audio registration,
and `SinglePlayerLoadScreen` composition remain deferred.  No retail input,
asset, path, hash, media, pixel, or audio-fidelity claim is made.

- Focused GCC Debug and Clang Debug source/provider-removal gates: **2/2**
  each.
- Fresh final-tree non-GPU/non-LAN suites: **225/225** in GCC Debug, GCC
  Release, Clang Debug, Clang Release, GCC ASan/UBSan, and Clang ASan/UBSan.
- Strict `detect_leaks=1` source-host check: **1/1** in each sanitizer
  toolchain.  Physical Vulkan validation: **1/1**.  Serial LAN: **4/4** in
  each of six configurations.
- Direct and registered dependency-ledger checks plus `git diff --check`
  pass.  The unrelated `tests/renderer/test_bgfx_device.cpp` diagnostic stays
  unstaged.
