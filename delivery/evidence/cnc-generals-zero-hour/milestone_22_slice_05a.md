# M22 slice 05A: indexed-draw contract and original buffer physical edge

Original `DX8RigidFVFCategoryContainer::Render` now calls a scoped
`OriginalGpuEdge` at its first physical vertex/index buffer operations.
The original `MeshModelClass` and category choose and fill buffers;
the adapter only uploads their exact bytes to public `GpuDevice`, retains
the original buffers while handles exist and destroys handles at scope
teardown. The following original texture/material call remains a typed
unavailable edge; no completed pass, generated proxy or retail frame is
claimed.

The public draw descriptor now retains original 16-bit index width,
first-index and signed base-vertex fields, with unchanged 32-bit/zero
defaults. Recording and SDL_GPU reject out-of-bounds index ranges and
unsupported formats. An owned M14 component scene exercised actual
SDL_GPU Vulkan 16-bit indexed drawing at nonzero first/base with
`VK_INSTANCE_LAYERS=VK_LAYER_KHRONOS_validation`; validation-clean
acceptance passed 1/1. This component witness does not replace M22's
later original retail rendering/visual acceptance.

The owned W3D source witness proves no-session failure, two original
source-issued buffer uploads, typed later texture edge and zero recorder
resources. Direct original DX8 buffer witnesses compare every uploaded
vertex/index byte to original-owned storage, prove stable repeated bind
and source reference lifetime, and reject nested/no session and unsupported
sorting. Injected failures at both create and both upload positions return
all original/device resources to zero. Identity and provider-removal
require original DX8 category and scoped translator at compile/link
boundaries. No copied mesh/material selection or Direct3D dependency.

GCC and Clang Debug full builds, 138/138 asset-free non-UDP tests each,
both preexisting local-UDP tests per compiler outside socket sandbox,
focused GCC and Clang ASan+UBSan source and failure fixtures, source
identity/provider removal, dependency-ledger freshness and diff check
passed. Original image decode/texture/material/shader entry is mandatory
05B; full interleaved passes/GameClient/retail/Vulkan are 06–09.
