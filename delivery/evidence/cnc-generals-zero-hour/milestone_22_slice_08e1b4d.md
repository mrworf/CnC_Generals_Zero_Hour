# M22 08E1B4D: generated AudioEventRTS provider evidence

This slice proves the prerequisite source value/provider boundary, not the
final `SinglePlayerLoadScreen` owner composition.  A generated current source
`Mission` carries `AudioEventRTS("GeneratedBriefing")` into the project-owned
`AudioManager` force-play provider.  A generated ambient event is source
copy-constructed, fails once without publication, retries to a normal handle,
rejects a special handle without mutation, and is removed to zero active
entries.  The sequence is repeated for two campaign generations; empty
briefing values are rejected.

The actual load-screen calls remain deferred to B4 because its ambient member
and handle are private and coupled to the complete WND, mapped-image, video,
display, `GameInfo`, and mode lifetime.  No audio INI, device, media, retail
content, or audio-fidelity claim is made.

- Focused GCC and Clang Debug source/identity/AudioEventRTS provider-removal:
  **3/3** each.
- Fresh final-tree non-GPU/non-LAN suites: **227/227** in GCC Debug, GCC
  Release, Clang Debug, Clang Release, GCC ASan/UBSan, and Clang ASan/UBSan.
  Canonical sanitizer CTest uses `detect_leaks=0` only for the sandbox ptrace
  limitation; strict host `detect_leaks=1` checks passed **3/3** in both GCC
  and Clang.
- Physical Vulkan validation passed **9/9**.  Serial LAN passed **4/4** in
  each of the six configured builds.  Registered/direct ledger checks and
  `git diff --check` pass.  The unrelated
  `tests/renderer/test_bgfx_device.cpp` diagnostic remains unstaged.
