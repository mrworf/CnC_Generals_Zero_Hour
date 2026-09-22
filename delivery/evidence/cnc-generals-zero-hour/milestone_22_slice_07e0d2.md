# M22 slice 07E0D2 evidence: original tactical camera position

The canonical Linux `W3DView::get3DCameraPosition` now returns the owned 3D camera position through the source-compatible static `Coord3D` reference. It rejects a view whose camera has not been initialized. Source controls verify `(0,0,1)`, a move to `(2,3,4)`, reference identity, and return to the initial camera without changing the physical frame. Native Windows behavior and class layout are unchanged; camera movement and audio behavior remain outside this slice.

Five fully rebuilt non-GPU suites pass 194/194 each: GCC Debug, GCC Release, Clang Release, GCC ASan+UBSan and Clang ASan+UBSan, including leak-capable source controls. GCC Debug and Clang Release each pass 30/30 fresh-process host Vulkan view-scene repetitions (BGRA8/RGBA8, two extents, four generations, two display lifetimes). Source ledger/provider checks and `git diff --check` pass.

The pending no-bib teardown and production-order factory bootstrap were isolated and were not staged in this slice. The unrelated renderer diagnostic and retail symlink content were untouched.
