# M22 08R0C transactional source terrain-prop frame evidence

Status: accepted; exact R0C behavior payload is this slice commit.
Transaction parent: `27b8c5ac022fefa1e4e4a7879f00d8b7fbc7722f`.
Only public generated inputs are used. Active08 source/test/ledger trial and the
unrelated renderer diagnostic remain unstaged. No factory or retail admission.

## Implemented boundary

The source HeightMap prop slot performs native cull/partition/shroud/lighting and
Render task generation. Triangles remain at the existing mesh/category flush,
with static/sorted work at the authored later flushes. Display owns the complete
props-only or combined terrain frame journal before Begin_Render, and exactly
one final present/CompleteFrame. Failure restores the checkpoint and keeps the
same immutable tree phase, when present, without C1/RNG/FX advancement.

Bounded graph preparation validates exact providers and all reachable motion
graphs before mutation. Embedded-sound SINGLE/DOUBLE and indeterminate external
COMBO graphs reject without sound dispatch or provider dereference. Typed state
capture includes animation/hierarchy/LOD, transforms/bounds, lighting, material
alpha/CRC and mapper temporal/RNG state. Resource preparation and native mesh
registration remain outside the frame. Consumed task/list nodes and refs are
retained until commit; abort restores exact identities/order/bytes, rather than
reconstructing from global state. Dynamic/static sorting buffers and skin scratch
are reserved outside the frame and restored exactly. Native DynamicSorting binding
is allowed only with both the admitted pending journal and mesh checkpoint.
Ordinary unsupported binding remains fail-closed.

Prop removal/reset/free/add preflight rejects both pending source-frame and
active-pass states before owner or resource mutation. The destructor asserts the
same idle invariant. The broader Edge retirement predicate is unchanged. Generated
controls prove unchanged owner/buffer/phase identity and deterministic idle retry
after journal close. Linux internal friendships/sidecars add no class fields,
virtuals, serialized state or Windows success-path change.

## Fixture classification and bounded workload

Native visibility rules are retained. The generated fixture uses the real
PartitionManager query and reveal/undo route; a small visual/logical-map adapter
asserts exact notification cell order and restores the real display before any
frame. SHROUDED suppression and FOGGED eligibility are separate controls. Provider
guards outlive visual teardown, preventing fixture cleanup from masking the
primary assertion. The fixture explicitly establishes source camera/world/view
bindings required by native sorting; it does not replace production draw paths.

Native category grouping is intentionally material/mapper-identity dependent.
Generated controls distinguish independent mapper instances from explicit shared
material coalescing. No grouping semantics were changed to satisfy a fixture.

The fresh equivalent operation sweep rejects exactly 1382 props-only and 1386
combined ordinals, then succeeds. Each failing instance proves same-owner retry.
Two isolated diagnostic-only GCC processes measured 137.385/137.275 seconds.
The approved wrapper-specific native timeout is 180 seconds per process with
TimeoutExpired retained and no workload/assertion reduction. Sanitizer runtime
must be measured separately before acceptance; the diagnostic ceiling is not
acceptance evidence. Earlier diagnostic runs are superseded by frozen-source gates.

## Reproducible gates and chronological checkpoints

Six complete builds: `cmake --build build/<preset> -j4`.
Exact ten-control focus, run serially on GCC/Clang debug and both sanitizers:

`ASAN_OPTIONS=detect_leaks=0 ctest --test-dir build/<preset> -R '^(original_w3d_prop_frame|original_w3d_prop_owner|original_w3d_prop_owner_identity|original_w3d_prop_owner_provider_removal|original_w3d_prop_shroud|original_w3d_terrain_map_frame|original_w3d_tree_draw|original_w3d_tree_decal|original_w3d_stage_transaction|original_w3d_source_reference)$' --output-on-failure -j1`.

Strict host LSan uses that same ten-control selection and exactly
`ASAN_OPTIONS=detect_leaks=1`, with host escalation and no strict UBSan override.
Generated validation-enabled physical proof on the four focused configurations:

`VK_INSTANCE_LAYERS=VK_LAYER_KHRONOS_validation python3 tools/run_validation_clean.py python3 tests/original_rendering/test_w3d_prop_frame.py --executable build/<preset>/zh_original_w3d_full_probe --asset-producer build/<preset>/original_w3d_cpu_graph_tests --source-root . --gpu`.

Established native Vulkan controls are GCC display-owner/factory-bootstrap/
factory-map and Clang display-owner/factory-map. Serial LAN uses
`ctest --test-dir build/<preset> -L lan --output-on-failure -j1`.
Six serial canonical suites use `ASAN_OPTIONS=detect_leaks=0 ctest --test-dir
build/<preset> -LE 'gpu|lan|retail' --output-on-failure -j1 -V`.
Full logs require runtime/sanitizer/validation category audits, not exit alone.

Final focused target builds pass GCC and Clang debug. Exact focus passes
10/10 on GCC in 302.61 seconds and Clang in 315.87 seconds, with full category-clean
logs; the expanded prop-frame controls take 253.58 and 262.88 seconds respectively.
These initial frozen gates are superseded by the lighting correction below.
All corrected final-source gates are pending.
Dependency ledger schema/hash and complete worktree whitespace checks pass.
Acceptance and exact staging are not inferred from diagnostic or prior-slice gates.

## Lighting correction and refreeze

The first GCC sanitizer diagnostic completes the exact 1382/1386 sweeps and all
retry/resource assertions in 560.325 seconds but is rejected by the wrapper's
runtime-category audit. Diagnostic-only UBSan halt/stacktrace localizes in 0.941
seconds: `lightenvironment.h:139:9: runtime error: load of value 126, which is not
a valid value for type 'bool'`. Native full-member InputLightStruct/LightEnvironment
assignment from MeshClass::Set_Lighting_Environment reaches an uninitialized
inactive input-light slot in W3DPropBuffer::drawProps, not the checkpoint copier.
This is a real reached source lighting defect, not an allocator or classifier
false positive. No clean timing or acceptance is claimed from that run.

Approved neutral InputLight/OutputLight constructors initialize all vectors,
scalars and bools, including FillLight and local directional-light unused point
fields. Layout and active-light formulas remain unchanged; portable initialization
does not change successful Windows lighting semantics. The generated owner test
covers explicit nonzero backing, fresh copies, populated directional/point lights,
reset/copy and two generations. The existing reached late-fault generated route
must be sanitizer-clean before timing is used to set any wrapper bound. Initial
native/build gates are diagnostic after this correction; affected builds/focus
restart on corrected frozen hashes. The narrow initialization lesson is recorded
in AGENTS.md without private evidence.

All six corrected complete builds pass, with clean compiler-error audits. The
corrected exact native focus passes 10/10 on GCC (302.25 seconds; prop-frame
253.31 seconds) and Clang (314.03 seconds; prop-frame 261.47 seconds), with clean
full-log runtime/validation category audits. The explicit fresh/reset/copy owner
regression passes under both sanitizers with diagnostic-only UBSan halt/stacktrace
(GCC 0.86 seconds, Clang 0.59 seconds), without a runtime category. Corrected
full generated sanitizer route/timing, registered sanitizer acceptance and all
remaining final gates are still pending. No registered timeout change yet.

The corrected isolated GCC sanitizer diagnostic completes both unchanged
generated processes in 565.788/566.341 seconds. Each reports exactly 1382
props-only and 1386 combined rejected ordinals, successful same-owner retry and
zero-resource teardown. The wrapper exits cleanly; full category audit is clean.
Diagnostic-only UBSan halt/stacktrace remains enabled, so this is reached-owner
localization/timing evidence, not the registered focus or strict-LSan gate.
The temporary driver calls the existing public `run(..., timeout_seconds=600)`
parameter with otherwise unchanged executable/helper/generated inputs/assertions.
It is not committed. The corresponding isolated Clang sanitizer processes complete
in 492.544/492.894 seconds with the same exact ordinal counts, successful retry,
zero-resource teardown and clean full-category audit. These four timings are
diagnostic workload measurements, not registered acceptance gates.

The approved final test-only correction changes this wrapper's per-process bound
from 180 to 750 seconds (~32% headroom over 566.341 seconds). TimeoutExpired,
both external generations, all 1382/1386 sweeps, generated inputs and assertions
are unchanged; no production or CMake change and no full rebuild. The corrected
six complete builds and native exact10 results at the stricter 180-second bound
are retained as proportional evidence. The selected 42 executable/helper binary
hashes are frozen before the correction; the wrapper-only hash delta cannot alter
those bytes or their behavior assertions. Registered sanitizer exact10, strict10
both, generated physical four, all six fresh canonical suites and common fresh
minimal/Vulkan/LAN controls remain required on the updated wrapper.

Registered final sanitizer exact10 passes 10/10 on GCC in 1358.29 seconds
(prop-frame 1130.93) and Clang in 1193.90 seconds (prop-frame 986.09), with
`ASAN_OPTIONS=detect_leaks=0`, no UBSan override and clean complete verbose-log
category audits. All 42 selected executable/helper hashes are rechecked exactly
unchanged. Strict host exact10 on both, generated physical four, minimal8 native
both, established native Vulkan3/2, serial LAN4 all six and fresh canonical291
all six are now queued serially. The queue stops on any exit/category; no strict,
physical or canonical result is inferred from the completed sanitizer focus.

Strict host LSan exact10 passes 10/10 on GCC in 1357.99 seconds (prop-frame
1128.78) and Clang in 1189.99 seconds (prop-frame 981.41), with exactly
`ASAN_OPTIONS=detect_leaks=1`, no UBSan override and clean complete verbose-log
category audits. Generated physical Vulkan on GCC has started; all four
physical/common/canonical results remain pending at this checkpoint.

The generated GCC and Clang native `--gpu` physical commands pass with the complete
Recording operation sweep, both external processes and all internal real-Vulkan
props-only/combined source-order, unchanged prior-pixel rollback and identical
retry assertions. Validation is enabled through the established host route;
the full category audits are clean. Both sanitizer physical commands also pass
with the same complete workload/assertions and clean full category audits.
No raw pixel image or private input is retained in this evidence.

Fresh minimal/headless selections pass 8/8 on GCC in 8.47 seconds and Clang in
8.30 seconds, proving full-draw helpers do not expand the lifecycle/minimal link.
Established validation-enabled Vulkan controls pass GCC 3/3 in 5.28 seconds
(display-owner/factory-bootstrap/factory-map) and Clang 2/2 in 4.19 seconds
(display-owner/factory-map). Serial host LAN passes 4/4 in all six configurations:
GCC debug 1.71 seconds, Clang debug 1.72, GCC release 1.72, Clang release 1.72,
GCC sanitizer 1.82 and Clang sanitizer 1.79. All complete logs are category-clean.
Six fresh canonical suites are now running serially with the exact documented
291-control selection; no completed canonical result is inferred yet.

The first fresh canonical suite passes GCC debug 291/291 in 592.77 seconds,
with a clean complete verbose-log category audit. Clang debug is the next live
serial suite; the other five canonical results are not inferred.

Clang debug also passes 291/291 in 585.22 seconds, with a clean complete
verbose-log category audit. GCC release is now running. All 49 frozen executable/
test inputs, 42 selected binary/helper hashes and ten preserved nonoverlapping
active08/renderer paths are rechecked byte-identical. The index remains empty;
the dependency ledger and worktree whitespace checks pass.

GCC release passes the fresh canonical selection 291/291 in 273.68 seconds,
with a clean complete verbose-log category audit. Clang release follows serially;
no result is inferred for the three remaining configurations.

Clang release passes 291/291 in 224.51 seconds, with a clean complete verbose-log
category audit. All four native canonical configurations are complete. GCC
sanitizer and then Clang sanitizer remain in the serial queue; their results
are not inferred from focused or strict gates.

GCC sanitizer passes the fresh canonical selection 291/291 in 2284.10 seconds
(prop-frame 1128.94), with ASan leak detection disabled, no UBSan override and
a clean complete verbose-log category audit. The prop runtime agrees with
the prior registered focus/strict measurements. Clang sanitizer is the final
live serial canonical suite; five of six configurations are now complete.

Clang sanitizer passes 291/291 in 1915.08 seconds (prop-frame 979.52), with
ASan leak detection disabled, no UBSan override and a clean complete verbose-log
category audit. The serial host queue completes successfully. Pending statements
above describe historical checkpoints, not outstanding acceptance work.

## Final acceptance and exact transaction audit

All six corrected complete builds pass. The approved wrapper-only timeout delta
reuses their unchanged bytes and corrected native exact10 results; registered
sanitizer exact10, strict host LSan exact10 both, generated physical four, fresh
minimal8 native both, established Vulkan3/2 and LAN4 all six pass as recorded
above. All six fresh canonical suites pass 291/291 with full category-clean logs:

| Configuration | Canonical count | Seconds |
|---|---:|---:|
| GCC debug | 291/291 | 592.77 |
| Clang debug | 291/291 | 585.22 |
| GCC release | 291/291 | 273.68 |
| Clang release | 291/291 | 224.51 |
| GCC ASan+UBSan | 291/291 | 2284.10 |
| Clang ASan+UBSan | 291/291 | 1915.08 |

The final identity audit rechecks all 49 frozen executable/test inputs and all
42 selected executable/helper/compile-link metadata files exactly unchanged.
The ten nonoverlapping active08/renderer paths are byte-identical and unstaged.
Only R0C's two dispatch hunks enter the shared engine file. The staged ledger
starts from HEAD and replaces only its 23 owned source rows (including two new
owners), with the engine digest derived from its exact indexed C-only bytes;
the preserved active08 ledger rows remain unstaged. Source ownership, native
success semantics, ABI/layout, exception/noexcept boundaries and public fault
seam closure are reviewed. Ledger schema/hash and worktree/staged whitespace
checks are required to pass before the exact commit. No private inputs, images,
raw logs or retail admission are part of this slice.

Final staged audit passes: exactly 54 owned paths, only two added engine lines,
exact owned ledger candidate and every indexed ledger source digest verified.
The indexed C-only engine SHA256 is
`c23d634ab79f682e6cfde182b57b6aaf22d035927eebd6e3efa29884b44e6ecb`;
the frozen worktree composition hash below intentionally includes preserved
active08 trial hunks. The unstaged remainder is exactly the twelve preserved
trial/renderer paths. Both staged and worktree whitespace checks pass; no
renderer diagnostic or private retail artifact is staged.

## Frozen public executable/test inputs

This is the exact frozen executable composition, including the preserved public
active08 hunks in the shared Linux engine file. C's exact commit isolates only
its two generated-probe dispatch hunks there; preservation of the remaining trial
hunks does not change the tested worktree bytes. Ten nonoverlapping trial/renderer
paths remain byte-identical to the transaction-start inventory and unstaged.
Plan/evidence status updates are not executable input changes.

```text
dbdbdd0e13ee43cfb187ed9f513ffe9dd19b24e9f480bbee68527004f8f1283e  CMakeLists.txt
ab54819c6de0ffae057b6fcfd3e04e1579b328d9fea1ecfbf77be9d4c2d0888b  GeneralsMD/Code/GameEngineDevice/Include/W3DDevice/GameClient/BaseHeightMap.h
4c88e2a2104dce0a742373fac9ce02ab18f4f2c50d25add8ad8e1a962d6106df  GeneralsMD/Code/GameEngineDevice/Include/W3DDevice/GameClient/W3DPropBuffer.h
5166d3e66f98812acc748a563670dd658984925ab29890b27406383049e6c15a  GeneralsMD/Code/GameEngineDevice/Source/W3DDevice/GameClient/BaseHeightMap.cpp
21598c24a3c9ed717834cc89738a60924d15b46d31199aba60ec6987f39ddd1e  GeneralsMD/Code/GameEngineDevice/Source/W3DDevice/GameClient/HeightMap.cpp
2ebac03596faaa2a284ed5949b797715d93517430486e41efa18f348113c5085  GeneralsMD/Code/GameEngineDevice/Source/W3DDevice/GameClient/W3DDisplay.cpp
f73825e8ea443e2fa142690e20dd3d8bf892daf52ac8ac1b0f4e4280c7489d32  GeneralsMD/Code/GameEngineDevice/Source/W3DDevice/GameClient/W3DPropBuffer.cpp
2fba3f1c2e18e8cb010681efc0af3eb673c4dbbf7f5d16640a4b6a9e1149f313  GeneralsMD/Code/Libraries/Source/WWVegas/WW3D2/animobj.h
718f754b834aae6d1f08ffd51446ac3d9fb092ad620f8f79080f0cbe42d004ff  GeneralsMD/Code/Libraries/Source/WWVegas/WW3D2/assetmgr.h
c56e54bf9590fe4adbfd2c13fa3d61e37166e1944c22550ef644ab4292725b00  GeneralsMD/Code/Libraries/Source/WWVegas/WW3D2/collect.h
4fbcf2af764a541de5566569b1777c8903eba4ca249f433ea871d3e564c23192  GeneralsMD/Code/Libraries/Source/WWVegas/WW3D2/composite.h
219bd61c3d0f1d8a865725c4c623dd7359f0cc5d23c02b9b89d65cc72b0c5eff  GeneralsMD/Code/Libraries/Source/WWVegas/WW3D2/dx8indexbuffer.cpp
1d52bc068c02f1c431c459751bfb07c148c8be43ca9b24b94fb478b74e797e7c  GeneralsMD/Code/Libraries/Source/WWVegas/WW3D2/dx8indexbuffer.h
0dedb3cf0bfce9f05db7f53e0362b925e4aa46a5642159b0a4754296b82a41af  GeneralsMD/Code/Libraries/Source/WWVegas/WW3D2/dx8polygonrenderer.cpp
d1f0c2bc9ee5697cb7f936894cd4b88747ad2bdd626f305f48c8b9730efd69ae  GeneralsMD/Code/Libraries/Source/WWVegas/WW3D2/dx8renderer.cpp
f9c88e1da1545070133570a074cc8278fc15325c3b3d69fe280594413e5109ba  GeneralsMD/Code/Libraries/Source/WWVegas/WW3D2/dx8renderer.h
5abc5237c759c1a3b9b79694247820154b12f4d106e1011db1da72cef026e4cc  GeneralsMD/Code/Libraries/Source/WWVegas/WW3D2/dx8vertexbuffer.cpp
394b31d2a1e98328728b2363420b165d2e7d5a000232612bc2e23a5d2e4d0bb6  GeneralsMD/Code/Libraries/Source/WWVegas/WW3D2/dx8wrapper.cpp
ace338679d64b01cde54e9375918e6720d4119d7511bad8b0436d7d77fbf70da  GeneralsMD/Code/Libraries/Source/WWVegas/WW3D2/hlod.h
04fed294e3ba8135e7f9ad15e79f98701829c1f39bb197928aab402636ff7e39  GeneralsMD/Code/Libraries/Source/WWVegas/WW3D2/htree.h
8af9755f62625ac5240308434b6fb61320003ebfabeddb00bca7538fe20eac03  GeneralsMD/Code/Libraries/Source/WWVegas/WW3D2/mapper.cpp
ba55505b5f9ff2a2fd71849fe53f4a6a3a9bd17e6313a3274ed17b4303180ed4  GeneralsMD/Code/Libraries/Source/WWVegas/WW3D2/mapper.h
7042f9e04b7aab371334ab43145b5fff0238e507b65cb361c71c054896a51376  GeneralsMD/Code/Libraries/Source/WWVegas/WW3D2/mesh.cpp
cbdc6a2119ca5840ecf05238f906ce95d17d28fe12518d6803391f25e79af8bd  GeneralsMD/Code/Libraries/Source/WWVegas/WW3D2/mesh.h
f45aa9fc440762ccb5bdd27f65e9df81325f2a26bf1795caf8125051035747fd  GeneralsMD/Code/Libraries/Source/WWVegas/WW3D2/meshmdl.h
389f676318cb14749a7a1329dc6a3609736a5cd31b997f1b1a62bde74fe275f8  GeneralsMD/Code/Libraries/Source/WWVegas/WW3D2/rendobj.h
430fe40cfebbb44cd42255c4f1884790680138dbdfaa21463f4ffb66d5f94e4f  GeneralsMD/Code/Libraries/Source/WWVegas/WW3D2/sortingrenderer.cpp
879b3e8b6d2fc5b748f63256a0f369c6975d43e613c1a49f523acc6df4a0caf7  GeneralsMD/Code/Libraries/Source/WWVegas/WW3D2/sortingrenderer.h
48a8e560898082c8333fd6a7fdd8bce5e864acf5a1b1219f640bba383660a7c9  GeneralsMD/Code/Libraries/Source/WWVegas/WW3D2/static_sort_list.cpp
7c9d607151e1bb115af73b818edf06b4c921a2cf99dc25a0327d8af865cf4534  GeneralsMD/Code/Libraries/Source/WWVegas/WW3D2/static_sort_list.h
4ce2aa6d407355bd42865846bace4b4ee105b58c8d79e5493f2ea8a285a28196  GeneralsMD/Code/Libraries/Source/WWVegas/WW3D2/texture.h
56cd5f9e4a44f8cf506693d3bd1fa3a26cba096377d71fd9d534e571edabf09a  GeneralsMD/Code/Libraries/Source/WWVegas/WW3D2/vertmaterial.cpp
a2022d612ee047e18df9bc5b5091e93243cfd4549a2c4ce8dbe166178632c145  GeneralsMD/Code/Libraries/Source/WWVegas/WW3D2/vertmaterial.h
82260733d3952a3bbd73e113a6c20acf8e1283d1eccfe0931bee071dda1341b2  GeneralsMD/Code/Libraries/Source/WWVegas/WW3D2/ww3d.cpp
1838e11fcddaa680b39f6e8af7712d8debf598b0bed73446a8dd3a1e3a7704a3  GeneralsMD/Code/Libraries/Source/WWVegas/WW3D2/ww3d.h
08848055a26d9003ff5c5cd0103b8245a70c63e27a43ca2f79541de6d1618202  GeneralsMD/Code/Libraries/Source/WWVegas/WWLib/Vector.H
64bd6da2b576b063f7ffda3a06ae98a1c72f79de9f78d78399082e5cda6fcbd8  GeneralsMD/Code/Libraries/Source/WWVegas/WWLib/multilist.h
dff637ac9984f772421b3e5bf26479ef0ffc671388cae8690f952279d3fa9cbb  GeneralsMD/Code/Libraries/Source/WWVegas/WWLib/simplevec.h
214e7c047fb6d5c95e63fa07fb5c8b5ec320433e7e398661e19664474c0b8b2d  src/original_runtime/linux_game_engine.cpp
228bac980a550ac7fe6aa0a001f8026b4d76a7a62bdae9e404851979bc082798  src/original_runtime/original_gpu_edge.cpp
404fe58c98e14b05c2f5e5e7d402f0125ebea1fbb0ff93fdbba1a3a826b4952b  src/original_runtime/original_gpu_edge.h
d58f7cd86336044d19fe81845b70c1203749dc3f0bd4e36ee1c15b6e6b4901cc  tests/original_rendering/generated_model_packet.inc
3ffadd6252f8ba7d80a80b95dce040255c74961f8b6d9daa390a7cfbf1ccd73e  tests/original_rendering/test_prop_owner.cpp
e35cfc7b7d7b61d3aba5eed2dd5b9330355333788340bf8b3fd4d1ae8eb32ee8  tests/original_rendering/test_w3d_cpu_graph.cpp
0991d1b47e52e101e44873fc27f2791c5e8e8812564c24901c7025c1f131b8cd  GeneralsMD/Code/Libraries/Source/WWVegas/WW3D2/prop_frame.cpp
f358a2a1289309abe3789ce8a03a96ed2ec674ce4de0cf90d4e22824df21ae49  GeneralsMD/Code/Libraries/Source/WWVegas/WW3D2/prop_frame.h
4954bf1d508fff06e51b6ad0eeca80399dd2fcf76fa8944e88ddf5761ddeb2a9  tests/original_rendering/prop_frame_probe.cpp
b1b2092f6c681b7933a0f210e647710ea0fea01f74e0e2219f62b97d60dd59df  tests/original_rendering/test_w3d_prop_frame.py
31ea475c36a1812589da3c3783b8024ee0fbb5a1e3512ad1a436befa57faef64  GeneralsMD/Code/Libraries/Source/WWVegas/WW3D2/lightenvironment.h
```

Selected executable/helper and compile-link metadata hashes before the wrapper-only timeout correction
(all six complete build configurations, exact10 command manifest; 42 files):

```text
a24265c9a67ba711c155c9a1ebe985116c4f104d56ac8e0dcebf5effc6a6c3ee  /home/ha/projects/CnC_Generals_Zero_Hour/build/linux-gcc-debug/original_w3d_prop_owner_tests
c93528241bd57e6a429496043bd68df778bcea1f334829c5a7bf318f7b206b36  /home/ha/projects/CnC_Generals_Zero_Hour/build/linux-gcc-debug/compile_commands.json
59d2813e97555a503aa17ae42d097a00db9678a0a247061712f7311027145ab5  /home/ha/projects/CnC_Generals_Zero_Hour/build/linux-gcc-debug/original-w3d-prop-owner.link.map
8a5a4dfc3194bd19041f9bb3dcc706e541656b0924fe0dbb34b601b436052478  /home/ha/projects/CnC_Generals_Zero_Hour/build/linux-gcc-debug/original_w3d_cpu_graph_tests
71ed7f197e2915dcc80f33e8c64cc3abc9538d639ba3fb6b3e3734b890137e1d  /home/ha/projects/CnC_Generals_Zero_Hour/build/linux-gcc-debug/zh_original_w3d_full_probe
17b2110a208ad6c62370baecf6a98356e07dd68b02f9f3646b7aeeec3ade385a  /home/ha/projects/CnC_Generals_Zero_Hour/build/linux-gcc-debug/original_w3d_source_reference_tests
0d86b4d1f2ba22f7814db85a815ac276aea335a35562918637af44c08bcca0a8  /home/ha/projects/CnC_Generals_Zero_Hour/build/linux-gcc-debug/original_w3d_stage_transaction_tests
066928a815b42ef18464c7f9f4de0e64091b260862b478ebe47bd833f7335b04  /home/ha/projects/CnC_Generals_Zero_Hour/build/linux-clang-debug/original_w3d_prop_owner_tests
a3bc9ccd8d98bb94f2f398dcaff67eb11ee70ea96bba41a6130bc75642ddc6c2  /home/ha/projects/CnC_Generals_Zero_Hour/build/linux-clang-debug/compile_commands.json
48d082d2d71f7755b6ac0400c8e9a935d236fed650899542fbda83c7a61d290f  /home/ha/projects/CnC_Generals_Zero_Hour/build/linux-clang-debug/original-w3d-prop-owner.link.map
c3b8a6ec5b073f817656633be235e9b0ad87534040ee0551cb93b159274ce7d7  /home/ha/projects/CnC_Generals_Zero_Hour/build/linux-clang-debug/original_w3d_cpu_graph_tests
9f09ebad5954c9205f41343b63c7ae241d6883c9097b15aab51e4626820f1293  /home/ha/projects/CnC_Generals_Zero_Hour/build/linux-clang-debug/zh_original_w3d_full_probe
818c72e27d890816e29b5f404780ca39f003833d55391abe3c732731d4bde86b  /home/ha/projects/CnC_Generals_Zero_Hour/build/linux-clang-debug/original_w3d_source_reference_tests
f54fd5e10e23c4fd386f112344c5dde7d64451d94f3d79ba6aea801ae53165b0  /home/ha/projects/CnC_Generals_Zero_Hour/build/linux-clang-debug/original_w3d_stage_transaction_tests
e93a31616e0846bf8e73d9fa0df19e0e2fc5ecb18e4711acfb3a9dcc38413945  /home/ha/projects/CnC_Generals_Zero_Hour/build/linux-gcc-release/original_w3d_prop_owner_tests
759db94d96dacf1c58dcd118e0b366383c444413c2fcbe02725537710f6545a6  /home/ha/projects/CnC_Generals_Zero_Hour/build/linux-gcc-release/compile_commands.json
a5d1c3deadd098ac36cc2224f4bb94eed84196c8c6d601d5203c5651593c2d96  /home/ha/projects/CnC_Generals_Zero_Hour/build/linux-gcc-release/original-w3d-prop-owner.link.map
e687aaca7630b58f4ed21836e7ea5a973540ad908e867f549b32f7bb7b7ee551  /home/ha/projects/CnC_Generals_Zero_Hour/build/linux-gcc-release/original_w3d_cpu_graph_tests
95454865a6948aabfca6548a4cd6d3c222477a4498472267fca2048077c8a61a  /home/ha/projects/CnC_Generals_Zero_Hour/build/linux-gcc-release/zh_original_w3d_full_probe
97325c87ddea5c4181f48037fbba1badc5899654491c32ffc1e3be3671e0367d  /home/ha/projects/CnC_Generals_Zero_Hour/build/linux-gcc-release/original_w3d_source_reference_tests
648a26c726770c7a1276ed2238ad08c3568ae3e1426b601469d6e792a2e850c2  /home/ha/projects/CnC_Generals_Zero_Hour/build/linux-gcc-release/original_w3d_stage_transaction_tests
bcdd6245261fe73153ec3c997641f080614f23fcd3d8cd562f41a88bc5ca3200  /home/ha/projects/CnC_Generals_Zero_Hour/build/linux-clang-release/original_w3d_prop_owner_tests
4a77a7df7aad4a689bd6f253e12442933002c01ef8bd6158d4d06f7ff8e7d11f  /home/ha/projects/CnC_Generals_Zero_Hour/build/linux-clang-release/compile_commands.json
fe809e42b3dbf5f3edbd4c88613848727716023f9fad52d09cf5b24890dce6b8  /home/ha/projects/CnC_Generals_Zero_Hour/build/linux-clang-release/original-w3d-prop-owner.link.map
ebbf3251d5bd232ba4925a648a12342107b57090b9860c6b13385cd1431c7f3a  /home/ha/projects/CnC_Generals_Zero_Hour/build/linux-clang-release/original_w3d_cpu_graph_tests
9164a69801c2ad70de6ed17b053674343acc3e50a7ab978359307e09568e61e1  /home/ha/projects/CnC_Generals_Zero_Hour/build/linux-clang-release/zh_original_w3d_full_probe
5f1cb4d982df481d71370b55c1b64485029bded79a4f26c807e8455e19232089  /home/ha/projects/CnC_Generals_Zero_Hour/build/linux-clang-release/original_w3d_source_reference_tests
2ace285821edd050a65cc81286e7d0afe4f00a1e95cfc721abc4f17eb60cbbfd  /home/ha/projects/CnC_Generals_Zero_Hour/build/linux-clang-release/original_w3d_stage_transaction_tests
5d3093ce8879951e3c1678216645c81ea400ca9bf30dbd40c8f4f435c9d2bbdc  /home/ha/projects/CnC_Generals_Zero_Hour/build/linux-gcc-sanitized/original_w3d_prop_owner_tests
9ce040f6b0018c8776889d2b523c9c2311b0c9039475f1b44fe1edf1d019b99f  /home/ha/projects/CnC_Generals_Zero_Hour/build/linux-gcc-sanitized/compile_commands.json
1303e2f243af37aa9f22e7f0e678c05e30eb9fa874f30dec94745d8a3b47c581  /home/ha/projects/CnC_Generals_Zero_Hour/build/linux-gcc-sanitized/original-w3d-prop-owner.link.map
a9a37ba2d310e30ab757472ae0aed6e8136e85d46c7c6a1811e44ee98382df02  /home/ha/projects/CnC_Generals_Zero_Hour/build/linux-gcc-sanitized/original_w3d_cpu_graph_tests
774a54eef587e4dd86aa31f54bcf56df552993ac085370f01dfab3b0d19219fe  /home/ha/projects/CnC_Generals_Zero_Hour/build/linux-gcc-sanitized/zh_original_w3d_full_probe
4050948e369120f118cff2d8d4a3f4942a6acfdb3e28e4c077ac722574d2a842  /home/ha/projects/CnC_Generals_Zero_Hour/build/linux-gcc-sanitized/original_w3d_source_reference_tests
d62cea85e2e8a8e11c86ee863ea6315227d959cb412ebbe5db7c4b19e13edb3c  /home/ha/projects/CnC_Generals_Zero_Hour/build/linux-gcc-sanitized/original_w3d_stage_transaction_tests
ee5954b8162dad0f958482b07e65297fa3c6f1f1bf77b6d9ff0f3f49279832ca  /home/ha/projects/CnC_Generals_Zero_Hour/build/linux-clang-sanitized/original_w3d_prop_owner_tests
be3de2900f80032e9d597c3d72463fa4c52ac5ca7406474fd35d00c3e386227b  /home/ha/projects/CnC_Generals_Zero_Hour/build/linux-clang-sanitized/compile_commands.json
a01d91d93d10a21186f5a928905fd0ea915093b77a08532dd4c398e3bb2f0c45  /home/ha/projects/CnC_Generals_Zero_Hour/build/linux-clang-sanitized/original-w3d-prop-owner.link.map
cbdffdd5718db106e0eda730c4a5fb4c838439165571f6591878c91e9738d001  /home/ha/projects/CnC_Generals_Zero_Hour/build/linux-clang-sanitized/original_w3d_cpu_graph_tests
a091f772e32033aafb57273eab08b2328bc7f9ecc65312f802099eb24a20371e  /home/ha/projects/CnC_Generals_Zero_Hour/build/linux-clang-sanitized/zh_original_w3d_full_probe
327b6fa6215379b8d4b8f601f689e3709fdce69af9fe6d524241b01908b2df10  /home/ha/projects/CnC_Generals_Zero_Hour/build/linux-clang-sanitized/original_w3d_source_reference_tests
c0ee30818c9546a9ad069f139f1d414dc1a062dbd8e1bceee020fa528b07ab8a  /home/ha/projects/CnC_Generals_Zero_Hour/build/linux-clang-sanitized/original_w3d_stage_transaction_tests
```
