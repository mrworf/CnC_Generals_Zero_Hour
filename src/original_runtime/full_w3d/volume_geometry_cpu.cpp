#include "PreRTS.h"
#include "full_w3d/volume_geometry_cpu.h"

#include "WW3D2/dx8fvf.h"

#include <stdexcept>
#include <vector>

namespace zh::original_runtime {
namespace {
constexpr Int kSourceMaxVertices = 16384;

void reject(bool condition, const char *message)
{
  if (condition) throw std::runtime_error(message);
}

VertexFormatXYZ extrude(const Vector3 &point, const Vector3 &light, Real distance)
{
  Vector3 result = point - light;
  result *= distance;
  result += point;
  return {result.X, result.Y, result.Z};
}
}

VolumeGeometrySlots build_volume_geometry(
    W3DBufferManager &manager, const Vector3 *vertices, Int vertex_count,
    const UnsignedShort *silhouette, Int silhouette_count,
    const Vector3 &light_object, Real extrusion_distance)
{
  reject(TheW3DBufferManager != &manager,
         "original volume geometry buffer provider is foreign or absent");
  reject(!vertices || !silhouette, "original volume geometry input missing");
  reject(vertex_count < 3 || vertex_count > kSourceMaxVertices,
         "original volume geometry vertex cardinality unsupported");
  reject(silhouette_count < 6 || (silhouette_count & 1) ||
             silhouette_count > kSourceMaxVertices * 2,
         "original volume geometry silhouette cardinality unsupported");
  reject(extrusion_distance <= 0, "original volume geometry extrusion unsupported");

  std::vector<UnsignedShort> edges(silhouette, silhouette + silhouette_count);
  for (Int i = 0; i < silhouette_count; ++i) {
    reject(edges[i] >= static_cast<UnsignedShort>(vertex_count),
           "original volume geometry silhouette index out of bounds");
  }
  for (Int i = 0; i < silhouette_count; i += 2) {
    reject(edges[i] == edges[i + 1], "original volume geometry degenerate edge");
  }

  // This is the count-and-reorder pass from constructVolumeVB.  It keeps the
  // original directed pair topology and connected-strip cache ordering.
  Int output_vertices = 2;
  Int output_triangles = 0;
  UnsignedShort strip_start = edges[0];
  for (Int i = 0; i < silhouette_count; i += 2) {
    const UnsignedShort end = edges[i + 1];
    Int next = i + 2;
    for (; next < silhouette_count; next += 2) {
      if (edges[next] == end) {
        const UnsignedShort first = edges[next];
        edges[next] = edges[i + 2];
        edges[i + 2] = first;
        const UnsignedShort second = edges[next + 1];
        edges[next + 1] = edges[i + 3];
        edges[i + 3] = second;
        break;
      }
    }
    if (next >= silhouette_count) {
      if (end != strip_start) output_vertices += 2;
      output_triangles += 2;
      if (i + 2 >= silhouette_count) break;
      strip_start = edges[i + 2];
      output_vertices += 2;
    } else {
      output_vertices += 2;
      output_triangles += 2;
    }
  }
  reject(output_vertices > kSourceMaxVertices || output_triangles > kSourceMaxVertices,
         "original volume geometry output overflow");

  VolumeGeometrySlots slots;
  try {
    slots.vertices = manager.getSlot(W3DBufferManager::VBM_FVF_XYZ, output_vertices);
    slots.indices = manager.getSlot(output_triangles * 3);
    reject(!slots.vertices || !slots.indices,
           "original volume geometry source slots unavailable");

    DX8VertexBufferClass::AppendLockClass vertex_lock(
        slots.vertices->m_VB->m_DX8VertexBuffer, slots.vertices->m_start, output_vertices);
    DX8IndexBufferClass::AppendLockClass index_lock(
        slots.indices->m_IB->m_DX8IndexBuffer, slots.indices->m_start, output_triangles * 3);
    auto *out_vertices = static_cast<VertexFormatXYZ *>(vertex_lock.Get_Vertex_Array());
    auto *out_indices = index_lock.Get_Index_Array();
    reject(!out_vertices || !out_indices, "original volume geometry source slot lock failed");

    Int v = 0;
    Int t = 0;
    Int last_vertex = 0;
    Int last_extrusion = 1;
    Int strip_vertex = 0;
    strip_start = edges[0];
    out_vertices[v++] = {vertices[strip_start].X, vertices[strip_start].Y, vertices[strip_start].Z};
    out_vertices[v++] = extrude(vertices[strip_start], light_object, extrusion_distance);
    for (Int i = 0; i < silhouette_count; i += 2) {
      const UnsignedShort end = edges[i + 1];
      const bool continues = i + 2 < silhouette_count && edges[i + 2] == end;
      const Int edge_vertex = (end == strip_start && !continues) ? strip_vertex : v;
      if (edge_vertex == v) {
        out_vertices[v++] = {vertices[end].X, vertices[end].Y, vertices[end].Z};
      }
      const Int edge_extrusion = (end == strip_start && !continues) ? strip_vertex + 1 : v;
      if (edge_extrusion == v) out_vertices[v++] = extrude(vertices[end], light_object, extrusion_distance);
      out_indices[t++] = static_cast<UnsignedShort>(last_vertex);
      out_indices[t++] = static_cast<UnsignedShort>(last_extrusion);
      out_indices[t++] = static_cast<UnsignedShort>(edge_vertex);
      out_indices[t++] = static_cast<UnsignedShort>(edge_vertex);
      out_indices[t++] = static_cast<UnsignedShort>(last_extrusion);
      out_indices[t++] = static_cast<UnsignedShort>(edge_extrusion);
      last_vertex = edge_vertex;
      last_extrusion = edge_extrusion;
      if (!continues && i + 2 < silhouette_count) {
        strip_start = edges[i + 2];
        strip_vertex = v;
        out_vertices[v++] = {vertices[strip_start].X, vertices[strip_start].Y, vertices[strip_start].Z};
        out_vertices[v++] = extrude(vertices[strip_start], light_object, extrusion_distance);
        last_vertex = strip_vertex;
        last_extrusion = strip_vertex + 1;
      }
    }
    reject(v != output_vertices || t != output_triangles * 3,
           "original volume geometry topology count mismatch");
    slots.vertex_count = v;
    slots.index_count = t;
    return slots;
  } catch (...) {
    release_volume_geometry(manager, slots);
    throw;
  }
}

void release_volume_geometry(W3DBufferManager &manager, VolumeGeometrySlots &slots)
{
  if (slots.indices) manager.releaseSlot(slots.indices);
  if (slots.vertices) manager.releaseSlot(slots.vertices);
  slots = {};
}

} // namespace zh::original_runtime
