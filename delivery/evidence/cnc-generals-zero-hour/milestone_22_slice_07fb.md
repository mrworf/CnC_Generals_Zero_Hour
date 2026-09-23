# M22 slice 07FB: active terrain-track update and flush

The bounded CPU-only source route owns exactly one active
`TerrainTracksRenderObjClass` in the primary loaded-map RTS3D scene. It creates
the source index/vertex buffers after the map texture is published, queues the
module before `HeightMapRenderObjClass::Render` flushes it, and tears all
resources down before a second generation.

The dedicated Recording probe verifies 84-byte index and 384-byte vertex
buffers, the indexed six-element active range, terrain-before-track order,
singleton/capacity rejection, map-less pending ownership, disabled and expired
no-draw controls, injected buffer-create/upload/draw rollback and retry, two
generations, and zero retained resources. It does not claim retail track input
or physical pixels.

Acceptance passed:

- GCC Debug: 7/7 coupled terrain lifecycle tests; exact non-GPU/non-LAN 202/202.
- GCC Release: exact non-GPU/non-LAN 202/202.
- Clang Debug: 10/10 coupled terrain, identity, provider-removal and ledger tests; exact non-GPU/non-LAN 202/202.
- Clang Release: exact non-GPU/non-LAN 202/202.
- GCC and Clang ASan+UBSan: exact non-GPU/non-LAN 202/202 each under CTest's required `LSAN_OPTIONS=detect_leaks=0` ptrace workaround.
- Host GCC ASan+UBSan 07FB probe with `detect_leaks=1`: pass.
- Host Vulkan validation `original_w3d_view_scene_bgfx`: pass.
- Host serial LAN: 4/4 pass in all six configured builds.
- `python3 tools/check_original_dependency_ledger.py --root . --ledger docs/original-runtime-dependency-ledger.tsv`: pass.
- `git diff --check`: pass.

The sandbox diagnostic full suite reports physical Vulkan and POSIX UDP failures
because it cannot initialize the host GPU or create sockets. Those are not
accepted as broad coverage: the exact non-GPU/non-LAN matrix and the host GPU,
LSan and serial-LAN gates above are the acceptance evidence. The direct
in-sandbox `detect_leaks=1` child aborts before probe code with LSan's documented
ptrace limitation, so it is not presented as a product leak result.
