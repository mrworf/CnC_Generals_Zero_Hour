# M22 08R0A terrain-prop graph and lifecycle evidence

Status: final-source acceptance complete; commit: this slice commit.
Transaction parent: `a8136b8a74122cf031d53b0fe5ca0078bcb61d12`.
Only public generated inputs are used. Retail corpus and active08 trial remain
read-only/private and unchanged; no prop frame/factory/retail admission is claimed.

## Implemented owner and generated controls

The Linux owner validates the reachable exact prototype graph before Create,
rejecting unsupported Aggregate/Ring/Sphere/Dazzle kinds, cycles, malformed bones,
hierarchy parents/indices/names/transforms and bounded counts. Admitted shared DAG
edges retain exact native ref multiplicity. Guarded HTree/HLOD/HModel/Collection
and DistLOD graph construction preserves native child/LOD/bone/container order,
conversion reversal and independent instance ownership, without native class
fields, virtuals, serialized layout or generic container changes. Windows success
branches remain unchanged.

W3DPropBuffer initialization no longer invokes the reversed memset. Type+instance
publication is offside and atomic, with exact 64-type/4000-slot bounds (including
tombstones), native weather/time model selection, finite zero/negative scales,
RotateZ→Scale→translation, translated type-sphere bounds and shroud invalidation.
Reverse cleanup releases instance refs before type refs and light/pass resources.
Logical release/recreation preserves owner identity; prop drawing remains closed.

One optional WW3D-free exact owner/token callback composes props-only lifecycle
preflight before drawable scanning and GameClient/GameLogic/GameEngine/resetAll/
shutdownAll mutations. Direct terrain/map operations also preflight before any
height-map VB/IB/backup/mapping mutation. Generated controls assert exact identities
and bytes on rejection, unchanged historical terrain draw, idle retry, provider
withdrawal and exact two-generation allocator teardown. Direct idle map free admits
subsequent visual teardown only for the exact post-free empty owner; active source
frame, stale generation, foreign provider and nonempty/map-mismatch remain closed.

## Failure classification and corrections before freeze

- Generated HLOD initially named a missing generated mesh, producing native
  missing-asset report storage. The fixture was corrected to reference its resident
  texture-free mesh; allocation expectations and production behavior were unchanged.
- A real ordering defect released HeightMap buffers before the inherited prop
  preflight. Moving that preflight to the top preserves exact VB/IB/backup bytes and
  Edge mappings. Fixture scope restoration also prevents secondary teardown from
  masking a primary rejection.
- Generated optional Collection snap-point coverage exposed a real definition
  teardown leak. Linux definition cleanup now releases its owned snap-point ref
  exactly once and clears deleted child-name count, preserving vector capacity.
  Snap/no-snap two generations, surviving siblings, provider removal and repeated
  public cleanup retain exact refs and allocator baselines.
- Exact post-free empty-state callback composition was corrected before freeze;
  no provider or active-frame guard was weakened.
- The first frozen GCC debug canonical returned 288/289. Its existing generated
  malformed-prop-pointer negative reached W3DPropBuffer::notifyShroudChanged
  before address validation. Read-only generated debugger localization proved
  an R0A-caused guard defect, not an environment or fixture failure. The failed
  log remains diagnostic evidence; later canonicals had not started. All prior
  complete-build/focus/strict/minimal/Vulkan/LAN results are superseded. The
  approved correction validates exact sidecar terrain/map/buffer/provider/
  generation/token identity before dereference and adds five rejection/retry
  cases to the registered terrain-visual-map witness. All required gates restart
  from zero on the corrected frozen source.

These diagnostic results are superseded by the final frozen source below. No
temporary markers, debugger mutation, private evidence or timeout changes remain.

## Reproducible acceptance commands

Six complete builds: `cmake --build build/<preset> -j4`, for GCC/Clang debug,
release and existing sanitizer configurations. Exact eleven-control focus:

`ASAN_OPTIONS=detect_leaks=0 ctest --test-dir build/<preset> -R '^(original_w3d_prop_owner|original_w3d_prop_owner_identity|original_w3d_prop_owner_provider_removal|original_w3d_clone_graph|original_w3d_clone_graph_identity|original_w3d_clone_graph_provider_removal|original_w3d_cpu_graph|original_w3d_house_color|original_w3d_house_color_identity|original_w3d_tree_module|original_w3d_terrain_visual_map)$' --output-on-failure -j1`.

Strict host LSan uses that exact focus with `ASAN_OPTIONS=detect_leaks=1` and no
UBSan override. Established validation-enabled native Vulkan controls are GCC
display-owner/factory-bootstrap/factory-map and Clang display-owner/factory-map.
Serial LAN uses `ctest --test-dir build/<preset> -L lan --output-on-failure -j1`.
Six serial canonicals use `ASAN_OPTIONS=detect_leaks=0 ctest --test-dir
build/<preset> -LE 'gpu|lan|retail' --output-on-failure -j1 -V`.
Host escalation is required for strict LSan, Vulkan and LAN. Full logs are audited
for sanitizer/runtime/validation categories, not exit status alone.

## Final-source gate results

Executable source refrozen after the sidecar correction. The corrected targeted
GCC subset passes 3/3, including the formerly crashing scene-attachment negative
and expanded terrain-visual-map control (60.39s). These are diagnostic until the
complete six-build relinks and required final gates finish. All earlier matrix
results are superseded. Dependency ledger and owned diff checks pass.
Corrected complete builds pass all six presets. Post-relink focus passes 11/11
on GCC debug (6.45s), Clang debug (6.62s), GCC sanitizer (13.95s) and Clang
sanitizer (12.18s). Strict host LSan passes 11/11 on GCC (14.99s) and Clang
(12.82s), without runtime/sanitizer categories. Minimal/headless controls pass
8/8 on both native toolchains. Validation-enabled established Vulkan passes
GCC 3/3 (5.22s) and Clang 2/2 (4.18s); serial LAN passes 4/4 on every preset.
Each full log passes the category audit. Corrected serial canonicals pass
289/289 on GCC debug (337.15s), Clang debug (325.98s), GCC release (177.61s),
Clang release (128.42s), GCC sanitizer (1144.13s) and Clang sanitizer (952.78s).
The formerly crashing scene-attachment negative passes in all six full suites,
including GCC sanitizer (261.29s) and Clang sanitizer (225.77s) active generated
work. Final frozen hashes, dependency ledger and owned diff check pass. The
unrelated active08 trial and renderer diagnostic remain unstaged.

## Frozen public owned source/test hashes

```text
aa03a6417cb9f415c82fbe053ee06776ee8520e9fb552ce8c6b2f7168236e713  CMakeLists.txt
7eeccdd331862e2cebf81b27c7d4442d610e2c626ad656cc497ed367ee6264d6  GeneralsMD/Code/GameEngine/Include/Common/SubsystemInterface.h
95b550db99f278edeea527985a31061ce76e190a2fb05e6fba7efebf6e42f145  GeneralsMD/Code/GameEngine/Source/Common/GameEngine.cpp
30f6d1de898b53d21718ab231c601601d5ada5b4943d137e6fac3a863d01d902  GeneralsMD/Code/GameEngine/Source/Common/System/SubsystemInterface.cpp
9805c3b5a75d5ef99e36b6fac7e779d526dcda76766bc1da0b1f6ff9c5810007  GeneralsMD/Code/GameEngine/Source/GameClient/GameClient.cpp
84771f51361c218eb17acddc4c5818b6d5a23f6d03c2b8371b162d8e69e48153  GeneralsMD/Code/GameEngine/Source/GameLogic/System/GameLogic.cpp
35feb9df9e5eec4eabeabe4d3facce79ed57bcc0f7dbcb37f00ec42d602c6e3f  GeneralsMD/Code/GameEngineDevice/Include/W3DDevice/GameClient/BaseHeightMap.h
e2da55f13a34d874501d9464fb421dfd9fc6ec0f6c4d3bfd7709bbd016205004  GeneralsMD/Code/GameEngineDevice/Include/W3DDevice/GameClient/HeightMap.h
c94440f0a2770dd9135a5b9a4c2d6cad034661010760d9a508fec6a4c3fe1ab6  GeneralsMD/Code/GameEngineDevice/Include/W3DDevice/GameClient/W3DPropBuffer.h
213b3b938e206ee6ba09075d15fd789bf1d32d236f496a792ea739b8a8669a9f  GeneralsMD/Code/GameEngineDevice/Include/W3DDevice/GameClient/W3DTerrainVisual.h
c02fd574a60f6c66358e444b1afb86f7337a2b587b57c355835592f2a854c000  GeneralsMD/Code/GameEngineDevice/Source/W3DDevice/GameClient/BaseHeightMap.cpp
b70e1be76c37608d35fd1be7077be000e9af8140debeafad76526c8d1d195996  GeneralsMD/Code/GameEngineDevice/Source/W3DDevice/GameClient/HeightMap.cpp
fd6031ab3107b3bf296199eb42548a1f9c5f437ae340fc3f2cd907dabe2aea31  GeneralsMD/Code/GameEngineDevice/Source/W3DDevice/GameClient/W3DPropBuffer.cpp
4504cc64fec01de1348f5cdaaad678936f9351f2475e399078a081b1ad607465  GeneralsMD/Code/GameEngineDevice/Source/W3DDevice/GameClient/W3DTerrainVisual.cpp
49a3910270b90a50f999d5a55348a549b513c0203c449049433e76e77f6ca202  GeneralsMD/Code/Libraries/Source/WWVegas/WW3D2/animobj.cpp
faf95ffca0cdff11b080701b4cb69528045e201acde51780742c4b8025b227e1  GeneralsMD/Code/Libraries/Source/WWVegas/WW3D2/assetmgr.cpp
9f965f16e59537324bf8906d469caa3ec83429cc41d000dafced090abf2b6cff  GeneralsMD/Code/Libraries/Source/WWVegas/WW3D2/collect.cpp
89f3bbdf1f7848a25e69c4f6b2a88f1ed89135fd9695e685ba71c837c9b09260  GeneralsMD/Code/Libraries/Source/WWVegas/WW3D2/collect.h
df291430f5eeafb564289021876dc8da9aae31707404adc2ff719a872e68ba55  GeneralsMD/Code/Libraries/Source/WWVegas/WW3D2/distlod.cpp
72f939b4efce8c06a4bc21372c2fdc87142fb256a009ce9e2a08735b6f8a9290  GeneralsMD/Code/Libraries/Source/WWVegas/WW3D2/distlod.h
4c321c3032c5f9a540c6c886b1c10edf3ca59a15801aea0376969ef4d922de93  GeneralsMD/Code/Libraries/Source/WWVegas/WW3D2/hlod.cpp
2d187e6272c05753b29bea21c72d996a05232e5896835a0152554a745c41f315  GeneralsMD/Code/Libraries/Source/WWVegas/WW3D2/hlod.h
73130989eed6309ed3150f91b952975994f7fb7f635695539db5e068dcfbdad8  GeneralsMD/Code/Libraries/Source/WWVegas/WW3D2/hmdldef.H
acdcb4ce63cf694b98e4534fd1272ee98685007c6560cf6a0b256022436cbada  GeneralsMD/Code/Libraries/Source/WWVegas/WW3D2/htree.cpp
e8b624d6dafa9bb030af6dc89a7068cafbe1187dae5a86b1acf6274ba6ab4092  GeneralsMD/Code/Libraries/Source/WWVegas/WW3D2/htree.h
16080255546243f105b79b18eba2dcca577be2e97620cb7c28ee10322242c9b8  GeneralsMD/Code/Libraries/Source/WWVegas/WW3D2/proto.cpp
b7eb87c348a12c29d4fbe41d003d537d1a60067f9a9b3f212a90fc2b231755d6  src/original_runtime/full_w3d/CMakeLists.txt
68c47e93a0839208e74badc9d23f17ecc11c519f1e2d5ae99afd84942f993ab3  src/original_runtime/original_gpu_edge.h
031e84c1a17555848563270e359e97bb194e139c75a4748881fa373399d6137e  tests/original_rendering/terrain_visual_map_probe.cpp
0e9ca30f1be374d7f7988c44901f987e4b3bc7704b1a25ebf8f8958e1b50e46e  tests/original_rendering/test_w3d_terrain_visual_map.py
a3f44c4ae75240c5e3236f0fdec5190e718b805608c663890b72002df026efcd  GeneralsMD/Code/Libraries/Source/WWVegas/WW3D2/prop_graph.cpp
f1c3fe8cfca5348e0107bab5f508cb3f339068f2c0675ad23d99389c04c347b6  GeneralsMD/Code/Libraries/Source/WWVegas/WW3D2/prop_graph.h
64155c9fe80007fa64d280f6595dda1804dd7bb5c4c39cb9e3ca6e66de3028be  tests/original_rendering/test_prop_owner.cpp
```
