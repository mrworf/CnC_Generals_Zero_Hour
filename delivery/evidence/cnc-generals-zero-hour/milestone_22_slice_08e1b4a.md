# M22 08E1B4A: generated named layout evidence

This slice extends the existing generated source-headless lifecycle fixture
with a project-owned callback-free WND containing a root and a named
`PROGRESSBAR` child.  The actual original `GameWindowManager` parser consumes
the logical FileSystem input, source NameKey lookup finds the child, the
original progress gadget stores its source state, and layout destruction leaves
no linked windows.  Existing generated missing-file and foreign-callback
controls remain in the same source fixture.  The fixture names no retail
layout or asset and claims no visual or audio fidelity.

Static text requires its distinct `TextData` source contract and remains B4C;
Mission/CampaignManager, GameText, AudioEventRTS and the
`SinglePlayerLoadScreen` owner remain later slices.

- Focused source-headless checks: **1/1** in GCC Debug and Clang Debug.
- Fresh non-GPU/non-LAN final-tree suites: **224/224** in each of GCC Debug,
  GCC Release, Clang Debug, Clang Release, GCC ASan and Clang ASan.
- Host LSan `detect_leaks=1`: **3/3** in GCC ASan and **3/3** in Clang ASan.
  Physical Vulkan: **1/1**. Serial LAN: **4/4** in all six configurations.
- Direct and registered dependency-ledger checks and `git diff --check` pass.
  The unrelated `tests/renderer/test_bgfx_device.cpp` diagnostic remains
  unstaged.
