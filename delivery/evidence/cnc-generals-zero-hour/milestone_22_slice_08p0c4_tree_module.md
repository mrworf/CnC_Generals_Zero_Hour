# M22 slice 08P0C4: physical Tree module factory and unwind

Status: final-source acceptance complete; implementation payload commit boundary
`delivery: M22 08P0C4 admit physical W3DTreeDraw`.

Authority: [C4 plan](../../plans/cnc-generals-zero-hour/milestone_22_plan_01_slice_08p0c4.md).
Implementation parent/approved plan checkpoint:
`32c2ecb4dbe8422233cb19c76a83f6fad463172b`; accepted C3:
`b2a613b6b69b3bc1c33e4a3043381e4a5a84082f`.

## Delivered boundary under validation

Only the full-instance factory opens the source `W3DTreeDraw` proc. Schema and
headless remain data-only, the other eight unavailable providers remain closed,
and retail admission remains slice08. Factory, constructor and generated
consumer compare the same CPU class size/alignment and six owner-field offsets.
The positive route uses real ThingFactory construction and public Object
transforms, never manual module replacement or direct terrain insertion.

Linux Thing setters use a stack-only exact transform/cache rollback guard;
Object forwards to its Drawable before partition/trigger/status side effects.
Linux module/Drawable removal preflight protects audited destroy, reset and
construction-unwind routes before mutation. The optional subsystem reset hook
uses exact owner/generation identity, rejects overlap/stale removal, remains
absent-safe in minimal lifecycle linkage and unregisters before GameClient
teardown. Module teardown cancels the accepted phase and compacts exact tree
ID/type storage without candidate allocations; aliases, surviving type indices,
last-tree retirement and stale epochs are preserved.

## Registered generated controls

The module witness covers real factory selection, null/wrong data, missing
terrain, zero-XY defer, authored zero-scale rejection, first/repeated/moved
placement, provider-removal rollback, all four setters and nested slope setters,
16 construction boundaries and14 model/atlas/GPU/registry admission faults.
Every failed construction has an immediate list-head/resource/RNG witness and
clean retry. Active-frame controls cover18 constructor/deletion/reset routes,
preserving Object/Drawable/module/lookup/phase/resource/RNG identities; abort
permits idle destruction and terrain lifecycle retry. Shared aliases, distinct
type compaction, last-tree cleanup and stale-registry replacement run across two
internal generations and two process generations.

The physical C4 branch replaces the proven C2D/C3 direct-add path with a real
factory Object and public transform. It retains bounded nonblack terrain,
multiplicative projected coverage, unchanged outside pixels/tree geometry,
late present/commit failure, exact factory relationship, unchanged accepted
pixels/phase/RNG/native frame count and identical successful retry. No C4
positive path calls `tryAddTree`; the unchanged non-C4 control branch does.

Fixture-only corrections explicitly author `Scale = 1` (retaining the zero
negative) and clear fresh targets rather than requesting an uninitialized load.
Shipping admission and assertions were not weakened. Temporary localization
markers were removed before source freeze.

## Completed final-source gates

All six complete builds returned0. Clang sanitizer conservatively rebuilt907
units; no compiler error occurred. Exact focused commands are in the C4 plan.

| Final focused configuration | Result | Wall seconds |
| --- | --- | --- |
| GCC Debug |21/21;complete log clean|58.76|
| Clang Debug |21/21;complete log clean|66.04|
| GCC sanitizer |21/21;complete log clean|258.88|
| Clang sanitizer |21/21;complete log clean|227.59|

Asset-free full draw scenario passes all four, without a retail-archive argument;
its Tree-without-ready-terrain control remains rejected. Strict host LSan passes
21/21 both with exactly `ASAN_OPTIONS=detect_leaks=1`, no UBSan override:
GCC263.49s and Clang231.49s. Both complete logs are category-clean.

After all six links, generated module/decal/draw/program/nonuniform-shroud
physical controls pass all four configurations serially with Vulkan validation
and `ASAN_OPTIONS=detect_leaks=0` under host escalation. Established Vulkan
passes GCC3/3(5.07s) and Clang2/2(4.28s). Host LAN passes4/4 all six(1.71–1.83s),
serially. Complete logs contain no sanitizer/validation category.

Private gate logs use `/tmp/m22-c4-final-*`; focused, strict, Vulkan, LAN and
canonical complete CTest logs are preserved separately. Six serial canonical
suites started from zero with `ASAN_OPTIONS=detect_leaks=0 ctest --test-dir
build/<preset> -LE 'gpu|lan|retail' --output-on-failure -j1`.

| Final canonical configuration | Result | Wall seconds |
| --- | --- | --- |
| GCC Debug |280/280;complete log clean|328.78|
| Clang Debug |280/280;complete log clean|316.76|
| GCC Release |280/280;complete log clean|168.90|
| Clang Release |280/280;complete log clean|120.49|
| GCC sanitizer |280/280;complete log clean|1109.30|
| Clang sanitizer |280/280;complete log clean|904.26|

Serial canonical session16650 returned0. Every final complete log is free of
runtime-error, ASan, LSan, UBSan, VUID and validation-error categories. No gate
is waived or pending. Dependency ledger, owned diff and frozen hash checks pass. The
unrelated renderer diagnostic remains unstaged; the original private-input
symlink/content is never modified and fixture inputs are generated/read-only.

## Frozen executable and witness source

The29-path source set below is unchanged across the completed final gates.
Plan/index/ledger/evidence-only updates are outside this executable freeze.

```text
49724a173cf3ae0a63417f6e43e334aaa49c7aaeef3821d490ec148a5f9bca19  CMakeLists.txt
1b69ce5da53ffec64b3971d782076929bec2d916b1c14b6e0d1bfb3dfea820bf  GeneralsMD/Code/GameEngine/Include/Common/DrawModule.h
ba5b53769a713f140e2d73e6a810f52dd97453b5788babb80cac018be16b3be4  GeneralsMD/Code/GameEngine/Include/Common/SubsystemInterface.h
c45234a3267dd48e76fac06a5848205b264792251926d6301b9b053beabe515b  GeneralsMD/Code/GameEngine/Include/Common/Thing.h
4da62802496a35bd539d6a7423ac7fae0591c60b16d13c45dcf37e8c3edf08ec  GeneralsMD/Code/GameEngine/Include/GameClient/Drawable.h
e9247a1d4fb0b10e0ed5a6ec5431aa857ec559ad6c78362b3c610314c8c655c7  GeneralsMD/Code/GameEngine/Include/GameClient/GameClient.h
a3dabb5729689df567ceca278bf137ab989d9a26455f1239a6ee06bbf8c4d33d  GeneralsMD/Code/GameEngine/Include/GameLogic/GameLogic.h
67ee57c2f1c8c4c277742480cf64ba03d8a9b2ab821ae991f0c642dd07ff9b68  GeneralsMD/Code/GameEngine/Include/GameLogic/Object.h
4d5544df5d76ff6fb75c6f6e67fc15a4c0c4076fe3d8ea8483f3864e6348da9c  GeneralsMD/Code/GameEngine/Source/Common/GameEngine.cpp
fbf10500667f0d6c46fc4f4f65879099d895e8836cb15107a82adbd5a5a1979c  GeneralsMD/Code/GameEngine/Source/Common/System/SubsystemInterface.cpp
e68cffe4dc59935ca0dd45fa3b85a4d989feedd420019e66407a47f85a0ec9fd  GeneralsMD/Code/GameEngine/Source/Common/Thing/Thing.cpp
fc13ee5e999ba442818fdb5b0a6d535f79d21266f64b88ba9b26d5c2a9266d85  GeneralsMD/Code/GameEngine/Source/GameClient/Drawable.cpp
1935541c84e3933e5529baeb1a862dc299e1a046118fad21360c6625d4d78a97  GeneralsMD/Code/GameEngine/Source/GameClient/GameClient.cpp
dff83ae6d0b79f1db4f4233aa1d1d2aa4a80ae09d0bc54c98a929085c4fbeba5  GeneralsMD/Code/GameEngine/Source/GameLogic/Object/Object.cpp
bbf7e0bab8faab7f878c73e5f13d4df0f268b7746996f136245c6f44471aecc6  GeneralsMD/Code/GameEngine/Source/GameLogic/System/GameLogic.cpp
c1e341cbf10aacf60a503c873e42e1c4d44f33a0b9c50a63a7ef6fcde6b25f78  GeneralsMD/Code/GameEngineDevice/Include/W3DDevice/GameClient/BaseHeightMap.h
8979bbcd514af2bf03d61db17f1316b1720c9a9fd070e05d88af5c2ffeb26951  GeneralsMD/Code/GameEngineDevice/Include/W3DDevice/GameClient/Module/W3DTreeDraw.h
4f706864dd0219e59485b726e9f3ff2f9b9179fe36557e32928437ac50515de5  GeneralsMD/Code/GameEngineDevice/Source/W3DDevice/Common/Thing/W3DModuleFactory.cpp
76d3668fd557cb9536bfe6b276b89b8cd4e6e88ece3cd41f9458da8085fe9558  GeneralsMD/Code/GameEngineDevice/Source/W3DDevice/GameClient/BaseHeightMap.cpp
ab8119257d01257c662358a92c09c191a2acc246ed445ed1c1f3cd06c52f5ad8  GeneralsMD/Code/GameEngineDevice/Source/W3DDevice/GameClient/Drawable/Draw/W3DTreeDraw.cpp
e45da1a6f588fe1758bd64047e3abedeabbb9c7ad3824b1a1e2b0f3270bc37d5  GeneralsMD/Code/GameEngineDevice/Source/W3DDevice/GameClient/W3DTerrainVisual.cpp
7843590f572c92e8b3a38fe9f4085b353933e86616aeb2fc0ab9d4a9b547468d  src/original_runtime/full_w3d/CMakeLists.txt
10ea4796f144e7291560f82319099f608c98add69fe8720a0837291d1cc699aa  src/original_runtime/linux_game_engine.cpp
b9929d23815c57d2a64b9f505dc760251bf9292baafecd5d31e75db07d096a7c  tests/original_lifecycle/test_subsystem_lifecycle.cpp
c061b84ab830c2ff21570d3c3facd4e47ecb15e0e6021bbd35c15ad55a455093  tests/original_rendering/terrain_tree_preparation_probe.cpp
a788a6351e3017d1f24a58e549963acc37b3133f4e25fb10fa1c89e0f86f1a40  tests/original_rendering/test_w3d_full_draw_scenario.py
bab3c101238ecdcf45abde4757342a4678a9f5e1cf86a9e3941aadf69993b009  tests/original_rendering/test_w3d_terrain_source_bitmap.py
0fe412f89d0bba5449b1f1bb5fcee1d3d0527360b5e184345b39a3e4a3cdb872  tests/original_rendering/tree_module_probe.cpp
c6c156914846db4d6e0889f4f7e24921e1cdcbf626fcb79e6e1ebda520037a78  tests/original_rendering/test_w3d_tree_module.py
```
