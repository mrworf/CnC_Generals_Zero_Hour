# M22 08R0B exact shroud material-pass evidence

Status: complete; corrected final-source acceptance and exact staged audit pass.
Transaction parent: `bd0ee97a`.
Only public generated inputs are used. Active08 trial and unrelated renderer
diagnostic remain unchanged/unstaged. No prop frame, factory or retail admission
is claimed.

## Implemented owner and generated controls

Idle stage-zero and accepted tree stage-one preparation use the existing bounded
selected-stage transaction for resident texture/filter/transform only. They do
not select a material, shader, blend or depth state. Complete ST_SHROUD_TEXTURE
Install/UnInstall rejects outside an active admitted source-frame journal before
mutation. The complete pass validates exact published current shroud content and
device generation, then applies the authored PRELIT_DIFFUSE sprite state, EQUAL
depth, camera-space COUNT2 transform and source-selected multiplicative blend.
The private bounded preset selector preserves DEBUG/INTERNAL fog alpha selection;
generated controls exercise both authored preset identities without changing the
release build profile. Windows code, class fields/vtables and serialized state
are unchanged.

The frame-only generated fixture uses the accepted Edge/DX8 journal plus the same
private shader-manager checkpoint used by W3DDisplay. Five program creation/
uniform upload failures and late present rejection restore exact source state,
resource identity and command baseline; clean retry commits. The generated
terrain-to-shroud sequence mirrors native HeightMap cache invalidation/application.
It asserts exact blend/depth/UV state and actual VertexUniform/FragmentUniform
buffer sizes, stage output index, texture identity and pipeline bindings through
the accepted exact-FVF source-state program ABI. No prop scheduling is enabled.

The physical mode claims a generated hidden SDL window and sampleable offscreen
target, draws an opaque depth prepass and the same shroud geometry, proves bounded
nonuniform multiplicative darkening, rejects a late native submission without
touching accepted pixels/live resources or advancing a native frame, then retries
to identical pixels. Native frame counters are checked before diagnostic readback,
which legitimately advances frames. Two generations release shroud texture/
sampler ownership, preserve unrelated Edge program/filter ownership until Edge
teardown, and finish with total zero resources.

## Diagnostic classification before freeze

- The first compile required a missing CPU-only terrain-visual include.
- Physical presentation initially lacked a claimed SDL window and sampleable
  target; the fixture now uses the established physical presentation preconditions
  and releases window resources before total-zero device verification.
- The expanded source-order fixture initially omitted native HeightMap's
  ShaderClass invalidation/application before the additional pass. Read-only
  source comparison established the missing fixture ordering; no renderer cache
  behavior was changed.
- Successful program preparation retains two Edge-owned uniform buffers, two
  shaders, one pipeline and the terrain stage-one filter until Edge teardown.
  Shroud release assertions now distinguish those exact independent owners from
  its texture/sampler; final total-zero verification remains mandatory.
- The physical rollback frame counter was initially sampled after diagnostic
  readback, which itself submits frames. The counter check now occurs immediately
  after abort, before the unchanged pixel readback assertion.

All earlier results are diagnostic after the final fixture changes. Temporary
markers and output-debug selectors were removed before the frozen set below.

## Reproducible acceptance commands

Six complete builds: `cmake --build build/<preset> -j4`.
Exact eight-control focus:

`ASAN_OPTIONS=detect_leaks=0 ctest --test-dir build/<preset> -R '^(original_w3d_prop_shroud|original_w3d_shroud_data|original_w3d_terrain_shroud_projection|original_w3d_tree_program|original_w3d_stage_transaction|original_w3d_source_reference|original_w3d_texture_decisions|original_w3d_material_abi_isolation)$' --output-on-failure -j1`.

The new prop-shroud CTest ID and the retained C2B projection ID invoke the same
expanded generated-map/process probe; the selection contains eight IDs, not
eight independent fixture owners. Strict host LSan uses exactly that focus with
`ASAN_OPTIONS=detect_leaks=1`, no UBSan override.

Approved timeout-only correction control on all six configurations:
`ASAN_OPTIONS=detect_leaks=0 ctest --test-dir build/<preset> -R '^original_w3d_tree_draw$' --output-on-failure -j1 -V`.

Physical on GCC/Clang debug and both sanitizers:
`VK_INSTANCE_LAYERS=VK_LAYER_KHRONOS_validation python3 tools/run_validation_clean.py python3 tests/original_rendering/test_w3d_terrain_shroud_projection.py --source-root . --executable build/<preset>/zh_original_w3d_full_probe --gpu`.

Established native Vulkan controls are GCC display-owner/factory-bootstrap/
factory-map and Clang display-owner/factory-map. Serial LAN uses
`ctest --test-dir build/<preset> -L lan --output-on-failure -j1`.
Six serial canonicals use `ASAN_OPTIONS=detect_leaks=0 ctest --test-dir
build/<preset> -LE 'gpu|lan|retail' --output-on-failure -j1 -V`.
Host escalation is required for strict LSan, Vulkan and LAN. Audit full logs for
runtime/sanitizer/validation categories, not exit alone.

## Final-source gates

All six complete builds pass on the frozen set. The exact focus passes 8/8 on
GCC/Clang debug and both sanitizers; strict host LSan passes 8/8 on both
sanitizers. The generated validation-enabled physical proof passes all four
focused configurations. Established native Vulkan controls pass GCC 3/3 and
Clang 2/2. Full logs for these final gates contain no runtime/sanitizer/validation
error category. Serial LAN passes 4/4 in all six configurations, with clean
category audit. Both serial native debug canonical suites pass 290/290,
with clean full-log category audit (GCC 619.87 seconds; Clang 583.43 seconds).
Both release suites pass 290/290, category-clean (GCC 334.96 seconds; Clang
232.39 seconds). GCC sanitizer passes 290/290 in 1827.70 seconds; full log has
no ASan/UBSan/runtime/validation error category. The final Clang sanitizer
canonical suite completes 289/290 in 1477.19 seconds, with a real generated tree-draw process
timeout at its unchanged 60-second wrapper bound. No sanitizer finding preceded
the timeout. This run is not acceptance. The preserved accepted R0A Clang
sanitizer log records 85.62 seconds for both tree-draw processes and 225.77 seconds
for terrain-scene; current GCC sanitizer records 113.94 and 344.91 seconds.
Those broader timings suggest host variance but do not establish causality.
An exact unchanged-command isolated rerun also times out at 60.44 seconds.
The failed full log contains no ASan/UBSan/runtime/validation error category;
all other 289 controls pass. An argument-free host process snapshot observed
concurrent high-CPU Java/linker work; no external process was changed or stopped.
Approved diagnosis-only runs use a temporary generated driver to call the exact
unchanged executable/helper/inputs/assertions with a 240-second process ceiling.
Two runs complete all assertions with zero error categories: 59.32/62.25 seconds
and 61.05/64.35 seconds. The repeat's two processes both report 368 rejected
operation boundaries. At diagnosis the wrapper/probe, CPU HeightMap and Edge.cpp
were unchanged from the accepted parent; CPU HeightMap selects terrain shaders,
and the tree sweep/phase/Edge does not call the new complete shroud-pass branch or
idle setShroudTex. No operation graph growth was found. The diagnostic driver is
not committed and these runs are not acceptance. The approved wrapper-only
correction changes 60 to 90 seconds per process, retaining TimeoutExpired,
all behavior assertions, both generations and the unchanged 368-boundary sweep.
No production or CMake bytes change; the nine previously frozen owned hashes
remain identical. The corrected exact tree-draw control runs on all six
configurations, followed by a complete rerun of only the failed Clang sanitizer
canonical. The five clean complete canonical suites and unchanged six-build,
eight-focus, strict, generated-physical, established Vulkan and LAN gates above
are explicitly reused proportionally: their binaries, selected inputs and
behavior assertions are unchanged. The only final test-byte delta is below.
The corrected exact tree-draw CTest control passes 1/1 on all six configurations
with complete category-clean logs (GCC sanitizer 87.29 seconds; Clang sanitizer
83.78 seconds for both generated processes). The complete final Clang sanitizer
canonical rerun passes 290/290 in 979.55 seconds, with a clean full-log category
audit; its corrected tree-draw control passes in 83.38 seconds. This completes
all six canonical configurations under the approved proportional provenance.
The earlier 289/290 and isolated 60-second timeout
logs remain superseded diagnostic evidence, not accepted gates.
Final ten owned public hashes, preserved trial/renderer hashes, dependency-ledger
hash/schema validation and complete worktree whitespace check pass. Exact staged
audit passes for 14 owned paths, six owned ledger updates plus one new helper row,
and all ten indexed source/test hashes. Unrelated paths and all four active08
ledger rows stay unstaged, including the unchanged renderer diagnostic.
No acceptance inferred from earlier diagnostic results.

## Frozen public owned source/test hashes

```text
2216adb77a63a04401909cc36f72027f715785723d1130b36a60f37ee3a4189b  CMakeLists.txt
a9b4452cce3156744b8f08a9b6858d4421a9e18cd74c774033cd7ac2df45d7ac  GeneralsMD/Code/GameEngineDevice/Include/W3DDevice/GameClient/W3DShaderManager.h
0824dd6082044aa56b2b2301288535d166c96ee23250a2f34ea655d4d1f53468  GeneralsMD/Code/GameEngineDevice/Source/W3DDevice/GameClient/W3DShroud.cpp
498ca98616afaac13be11f7a47fcd94d2bf5ae7e554c33f02756ab5234b20e1d  GeneralsMD/Code/Libraries/Source/WWVegas/WW3D2/dx8wrapper.h
404cea9f373ea30f34fed41ae45e8ad7f7b784e8cc926dc847e913dfbeb9123f  src/original_runtime/original_gpu_edge.h
9495a2ab2dffd1ef6024423c92edb88b90e5a5940ce03d8d26948f85babd6c5a  src/original_runtime/shroud_material_preset_cpu.h
004f39ba20edf73e0333e88fff9ead65de48fe1fef057b6fc25ae7ed1d70da96  src/original_runtime/w3d_shader_manager_cpu.inc
53d1d5ce94151b039ca0ef7f99c71718e678866a3ae7b650022eab33760fcc05  tests/original_rendering/terrain_shroud_projection_probe.cpp
7cbc8a6df3c0a4689ed612c5145f2fc755b4a6fdb775478faac80412db583f4c  tests/original_rendering/test_w3d_terrain_shroud_projection.py
9680a9a37c492e9d5cb989d388944dd03d249667ad8cbfc1136ca5598c721a79  tests/original_rendering/test_w3d_tree_draw.py
```

The tree-draw wrapper's accepted-parent hash was
`a49da11c27f0fb4181bb2c9c57dd1b7569b056676bc9b40a3ba7c809e9f29136`;
its new hash records only timeout/comment correction, no workload/assertion edit.
