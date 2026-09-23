# M22 slice 07FC: active map-water route

The CPU-only original route admits exactly one nonzero-extent
`WATER_TYPE_0_TRANSLUCENT` plane, published by `W3DTerrainVisual` in the
primary loaded-map RTS3D scene. It owns a four-vertex (96-byte), six-index
(12-byte) source quad and submits after terrain and the accepted terrain-track
flush. Disabled water remains a scene-owned no-draw member; direct no-water
`Render` remains a typed rejection. Clouds, reflection water, grid mutation,
shadows, particles and water pixels remain guarded.

The generated-map Recording probe verifies source buffer descriptors and draw
range, terrain-to-tracks-to-water ordering, disabled/cloud/zero-extent/
reflection/foreign-scene rejection, create/upload/draw rollback and retry,
two independent generations, and zero retained Recording resources.

Acceptance passed:

- GCC Debug, GCC Release, Clang Debug and Clang Release: exact non-GPU/non-LAN 203/203 each.
- GCC ASan+UBSan and Clang ASan+UBSan: exact non-GPU/non-LAN 203/203 each under CTest's required `LSAN_OPTIONS=detect_leaks=0` ptrace workaround.
- Host GCC ASan+UBSan map-frame/tracks/water/view focused suite with `detect_leaks=1`: 4/4 pass.
- Host Vulkan validation `original_w3d_view_scene_bgfx`: pass.
- Host serial LAN: 4/4 pass in all six configured builds.
- Dependency ledger and `git diff --check`: pass.

The sandbox CTest wrapper cannot collect LSan diagnostics under ptrace; host
focused leak detection above is the decisive leak result. Physical Vulkan and
UDP tests are separately host-run and are not counted as sandbox broad-suite
coverage.
