# M22 original WW3D CPU closure (slice 01)

The explicit original-source lists in `CMakeLists.txt` are the executable inventory. `zh_w3d` now compiles 45 original `WW3D2` translation units plus `src/original_runtime/ww3d_cpu_state.cpp`; `zh_wwsupport` additionally compiles the needed original WWLib, WWMath, and WWSaveLoad files. The remainder of the original WW3D2 directory is not assumed reachable. The asset-manager registration and the mesh/HLOD/hierarchy/animation test fixture justify the transitive graph; link-map identity ensures the fixture actually links the original providers.

| Reached producer family | Canonical source and observed behavior |
| --- | --- |
| Manager and prototypes | `assetmgr.cpp`, `proto.cpp`; load and register chunks, reject duplicate names, clone render objects, release prototypes. |
| Geometry and materials | `mesh.cpp`, `meshmdl.cpp`, `meshmdlio.cpp`, `meshgeometry.cpp`, `meshmatdesc.cpp`, `matinfo.cpp`, `vertmaterial.cpp`, `shader.cpp`, `mapper.cpp`; triangle vertices/indices, pass shader, vertex material, texture-stage references. |
| Composition and animation | `hlod.cpp`, `htree.cpp`, `htreemgr.cpp`, `hanim.cpp`, `hanimmgr.cpp`, `hrawanim.cpp`, `hcanim.cpp`, `hmorphanim.cpp`; HLOD bone/subobject link and hierarchy/raw animation. |
| Texture | Original `texture.cpp` retains the single `Load_Texture` implementation; CPU-only metadata/device handling is compiled into this original translation unit through `texture_cpu.inc`, not another parser or provider. `TextureClass::Apply` fails explicitly until the slice-03 device translation. |
| Other registered/linked subclasses and math | `hmdldef.cpp`, `collect.cpp`, `boxrobj.cpp`, `distlod.cpp`, `agg_def.cpp`, `nullrobj.cpp`, `dazzle.cpp`, `ringobj.cpp`, `sphereobj.cpp`, `assetstatus.cpp`, `w3dexclusionlist.cpp`, `w3d_util.cpp`, `aabtree.cpp`, `lightenvironment.cpp`, `aabtreebuilder.cpp`, `animobj.cpp`, `composite.cpp`, `snappts.cpp`, `pivot.cpp`, `visrasterizer.cpp`, `coltest.cpp`, `predlod.cpp`, `camera.cpp` plus original support/math. These are link dependencies of the registered render-object graph, not an assertion that the test visits all concrete render variants. Slice 02 must visit the campaign/skirmish concrete classes. |

The owned in-memory W3D fixture uses the original chunk writer and RAM file, then the original asset manager, creation, inspection and reverse release paths. Positive controls assert HLOD/hierarchy/model, animation, geometry, shader, vertex material and texture-filter identity. Negative controls reject duplicate prototypes, oversized mesh counts/texture names, unknown chunks and device use before translation. The original asset manager can publish earlier valid chunks before reporting a later error; this authored intermediate behavior is retained. Slice 05 owns required retail scenario failure unwind and corrected retry.

No original-game symlink files are modified or copied into tests. This is a CPU source-identity gate, not a rendered retail scene or Vulkan acceptance claim.

M22 08P0C2B0A adds a bounded owner for acquired source texture references,
distinct from native handle reference ownership. Finish/cancel transfers exact
units without callbacks; ordinary exact-generation drain commits idle device
cleanup before detaching terminal metadata and releasing pooled objects.
Retryable failures preserve the queue and publication. Receiver-pinned texture
invalidation drains enrolled pins before ordinary mutation. Mandatory shutdown
failure terminates with a fixed cleanup category; native device loss remains
process-fatal, not a normal teardown or stale-generation retry. Source-stage
atomic application and shroud semantics remain dependency-next.

M22 08P0C2B0B composes that owner with a bounded resident selected-stage
transaction. Membership and complete filter admission precede allocation,
retention, access-time changes and device admission. Selected maps, exact key
presence, preallocated transform nodes, texture refs, access times and pending
edge/filter/sampler/revision state roll back without ordinary fallible setters.
Selected-only application leaves pending shader/material and unselected state
unchanged. Reset, source removal and destruction cancel in the live generation.
Finish transfers pins to A; ordinary drain owns terminal retirement and retry.
Only grouped strictly-surviving units bypass the device cleanup journal, without
callbacks or allocation; equality and mixed terminal groups stay transactional.
This capability does not apply shroud semantics, produce a frame, admit retail
assets or open the tree factory.
