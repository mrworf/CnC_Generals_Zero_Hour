#pragma once

// Portable CPU extraction of the side-wall construction in
// W3DVolumetricShadow::constructVolumeVB.  It deliberately has no shadow
// owner, render task, stencil state, or device API.

#include "W3DDevice/GameClient/W3DBufferManager.h"
#include "vector3.h"

namespace zh::original_runtime {

struct VolumeGeometrySlots {
  W3DBufferManager::W3DVertexBufferSlot *vertices = nullptr;
  W3DBufferManager::W3DIndexBufferSlot *indices = nullptr;
  Int vertex_count = 0;
  Int index_count = 0;
};

// `silhouette` is an even-length sequence of directed source vertex pairs.
// The routine performs the original connected-strip reordering and emits two
// triangles per pair with the original side-wall winding.
VolumeGeometrySlots build_volume_geometry(
    W3DBufferManager &manager, const Vector3 *vertices, Int vertex_count,
    const UnsignedShort *silhouette, Int silhouette_count,
    const Vector3 &light_object, Real extrusion_distance);

void release_volume_geometry(W3DBufferManager &manager, VolumeGeometrySlots &slots);

} // namespace zh::original_runtime
