# M22 08E1B4C: generated GameText and static-gadget evidence

This slice composes the B4B-shaped generated mission label with the existing
project-owned `GameTextInterface` provider and the live original static-gadget
path:

`Mission label` → `GameTextInterface::fetch` → `GadgetStaticTextSetText` →
`TextData`/`DisplayString` → `WindowLayout::destroyWindows` → queued
`GWM_DESTROY` → `DisplayStringManager::unLink`.

The project-owned callback-free WND contains one named `STATICTEXT`.  The
two-generation witness verifies exact translated assignment, duplicate
replacement, empty clear, null-window no-op, malformed callback-layout
rejection inherited from B4A, provider removal, and no linked display strings
after the source destroy queue is drained.  The queue drain is required by the
original `GameWindowManager` lifecycle; immediate ownership release is not
claimed.  Foreign or stale `TextData` has no public source admission API and
is therefore structurally unavailable rather than simulated with an invented
owner.

Retail text tables, Campaign INI, mode-owned `SinglePlayerLoadScreen`, audio,
mapped-image/video composition, and pixels remain deferred.  All WND bytes
and labels are project-owned generated inputs; no retail path, name, hash, or
content is used.

- Focused GCC Debug and Clang Debug static/provider checks: **3/3** each;
  coupled source-identity and all three registered ledger checks: **6/6**
  each.
- Fresh final-tree non-GPU/non-LAN suites: **226/226** in GCC Debug, GCC
  Release, Clang Debug, Clang Release, GCC ASan/UBSan, and Clang ASan/UBSan.
  The first Clang sanitizer diagnostic had only 225 registered tests because
  its build tree predated this slice's CMake registration: comparison found
  only `original_static_text_provider_removal` missing.  It was not accepted;
  a forced CMake regeneration registered 226, the new test passed directly,
  and the fresh 226-test suite passed.
- Canonical sanitizer CTest runs use `detect_leaks=0` because sandbox ptrace
  prevents reliable process leak inspection.  The strict host GCC and Clang
  `detect_leaks=1` focused source/provider checks passed **3/3** each.  Host
  Vulkan validation passed **9/9** and serial LAN passed **4/4** in each of
  the six configured builds.
- Direct and registered dependency-ledger checks plus `git diff --check`
  pass.  The unrelated `tests/renderer/test_bgfx_device.cpp` diagnostic stays
  unstaged.
