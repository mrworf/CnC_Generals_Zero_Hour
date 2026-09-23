# M22 08E1B3A: original generic video-stream registry evidence

The portable original `VideoPlayer.cpp` registry path is exercised with a
project-owned generated `VideoStream`: descriptor lookup and duplicate
replacement, unknown/duplicate-open rejection, update failure then
reset/retry, original close/list removal, original destructor publication
removal, and zero streams across two generations. Bink decode and W3D texture
buffers remain outside this slice.

- Focused GCC and Clang provider plus source-removal checks: **2/2** each.
- Fresh non-GPU/non-LAN suites: **219/219** in GCC Debug/Release, Clang
  Debug/Release, GCC ASan+UBSan and Clang ASan+UBSan. Sandbox broad sanitizer
  runs use `detect_leaks=0` only for ptrace limitations.
- Host leak-enabled focus: **2/2** GCC and **2/2** Clang; Vulkan probe **1/1**;
  serial loopback LAN **4/4** in all six builds.
- Direct and registered ledger checks and `git diff --check` pass. The
  unrelated renderer diagnostic remains unstaged.
