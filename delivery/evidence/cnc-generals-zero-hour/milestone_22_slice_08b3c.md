# M22 slice 08B3C — bounded original volumetric-shadow owner

The explicit `ZH_M22_VOLUME_SHADOW_PROFILE` admits one generated, published
original `W3DModelDraw` `SHADOW_VOLUME` owner.  Its map-owned original buffer
provider builds the accepted CPU side-wall slots after map load, and
`RTS3DScene` schedules the original `DoShadows(true)` phase after
terrain/tracks and before water.  The owner delegates only its public
three-pass D24S8 source-buffer submission to 08B3C0; it makes no retail,
resize, raw-D3D/private-Vulkan, or pixel claim.

Focused proof covers absent profile, projection/none, duplicate and detached
owners, deferred provider acquire, resource/draw failure then retry, disabled
no-repeat, source ordering, reverse owner removal, and two generations with
zero Recording resources.  The default typed volume rejection remains intact.

Acceptance completed on the final tree:

- GCC Debug, GCC Release, Clang Debug, Clang Release: 212/212 non-GPU/non-LAN.
- GCC ASan/UBSan and Clang ASan/UBSan: 212/212 with sandbox
  `ASAN_OPTIONS=detect_leaks=0` only.
- Host leak-enabled Clang focused route: 3/3 with `detect_leaks=1`.
- Host public Vulkan C0 bridge check: 1/1.
- Host serial LAN: 4/4 in each of six configurations.
- Dependency ledger validator and its three registered consumers pass.

The only unrelated worktree file remains `tests/renderer/test_bgfx_device.cpp`.
