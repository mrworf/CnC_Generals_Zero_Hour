# M22 slice 04B — original mesh buffer/category ownership

Canonical original `MeshModelClass::Register_For_Rendering` calls canonical
`DX8MeshRendererClass::Register_Mesh_Type`, which builds original rigid,
sorting and skinned FVF/polygon/texture categories. Original vertex/index
buffer classes own CPU upload bytes and append-lock offsets in the same
source files; their D3D allocation and lock calls are isolated behind the
mutually exclusive CPU/native source configuration. The original
`StripOptimizerClass` and original polygon-category destructor participate
in link and lifecycle. The interim WW3D static NPatches defaults remain in
the preexisting CPU state file until 04C moves the full original `ww3d.cpp`
render/flush owner into the Linux target.

An owned W3D fixture loads a source-selected sorting mesh, switches its
original SORT flag off to exercise source-selected rigid FVF registration,
and loads a second skin geometry with an authored vertex-influence chunk.
The source creates polygon renderer/category memberships and original
vertex/index buffers. `Invalidate`, pending-delete cleanup and registration
retry release all buffers, clear model memberships, rebuild both models,
and finish at zero buffers on teardown. Direct owned buffer tests witness
byte-accurate append offsets and index values and reject stale engine refs,
out-of-range locks, NPatches and dynamic physical usages. Original
`DX8MeshRenderer::Flush` explicitly rejects the still-unavailable GPU edge.
Malformed material/texture loader controls from 04A remain in the graph.

This is CPU source-owned buffer/category behavior, **not** original ordered
mesh/WWShade pass execution, a recording frame or Vulkan rendering. The
temporarily typed physical render/flush methods must be replaced by 04C and
05; original WW3D static defaults must not coexist with `ww3d.cpp`.

Validation:

- Complete GCC/Clang Debug builds and 136/136 non-LAN tests for each;
  both existing local-UDP LAN tests passed separately outside the socket
  sandbox for each toolchain, making 138/138 each.
- GCC and Clang focused owned original graph, full-draw probe, ABI, runtime
  source identity and provider-removal controls pass. Original mesh model,
  vertex/index buffers, renderer, polygon and strip optimizer are required
  compile/link sources; removing each provider is rejected.
- GCC and Clang focused ASan+UBSan graph pass outside the ptrace-restricted
  sandbox. Dependency-ledger hashes and `git diff --check` pass.

No original-game symlink contents or private retail data were written. The
production target remains the canonical M20/M21 schema configuration;
the original full-behavior probe remains a separate unmixed executable.
