# M22 slice 07FE: bounded active smudge route

The generated effect profile now selects the original particle provider and
renders exactly one original `SmudgeSet` containing one five-vertex smudge
through the original derived manager.  It uses the fixed twelve source
indices, terrain texture and alpha material after terrain, tracks and water.
Unsupported particle variants and oversize/malformed batches fail closed.
The owned source buffers retire individually from the active edge, leaving
terrain, tracks and water providers intact.  Reused source sets reset their
used count before retry/re-entry.

Final-tree acceptance:

- Fresh non-GPU/non-LAN CTest: 206/206 in GCC Debug (107.09s), GCC Release,
  Clang Debug, Clang Release, GCC ASan+UBSan, and Clang ASan+UBSan.  The two
  broad sanitizer runs used `LSAN_OPTIONS=detect_leaks=0` only for the known
  ptrace limitation.
- Host focused `detect_leaks=1`: GCC ASan+UBSan provider and coupled
  terrain-water 2/2; equivalent Clang ASan+UBSan `/tmp` executable provider
  and terrain-water probes passed 2/2.
- Host Vulkan `original_w3d_view_scene_bgfx`: pass.  Serial LAN acceptance:
  4/4 in all six configured builds.
- GCC Debug identity/provider/ledger group: 4/4; direct dependency-ledger
  validation and `git diff --check`: pass.
