#include "PreRTS.h"
#include "full_w3d/volume_geometry_cpu.h"
#include "WW3D2/dx8fvf.h"
#include "original_gpu_edge.h"
#include "zh/renderer/recording_device.h"

#include <cstdio>
#include <cstdlib>
#include <stdexcept>

namespace {
void require(bool value, const char *message) { if (!value) throw std::runtime_error(message); }
template <typename F> bool rejected(F action) { try { action(); } catch (const std::runtime_error &) { return true; } return false; }

void write_triangle(W3DBufferManager::W3DVertexBufferSlot *vertices,
	W3DBufferManager::W3DIndexBufferSlot *indices)
{
	DX8VertexBufferClass::WriteLockClass vertex_lock(vertices->m_VB->m_DX8VertexBuffer);
	auto *points = static_cast<VertexFormatXYZ *>(vertex_lock.Get_Vertex_Array()) + vertices->m_start;
	points[0] = {0,0,0}; points[1] = {1,0,0}; points[2] = {0,1,0};
	DX8IndexBufferClass::WriteLockClass index_lock(indices->m_IB->m_DX8IndexBuffer);
	auto *triangles = index_lock.Get_Index_Array() + indices->m_start;
	triangles[0] = 0; triangles[1] = 1; triangles[2] = 2;
}
}

extern "C" void zh_probe_volumetric_cpu_closure()
{
	require(std::getenv("ZH_M22_VOLUME_CPU_CLOSURE_PROFILE"), "original volume CPU closure profile missing");
	const Vector3 points[] = { Vector3(0,0,0), Vector3(1,0,0), Vector3(0,1,0) };
	const UnsignedShort silhouette[] = { 0,1, 1,2, 2,0 };
	zh::renderer::RecordingGpuDevice device;
	for (Int generation = 0; generation != 2; ++generation) {
		zh::original_runtime::OriginalGpuEdge edge(device);
		W3DBufferManager manager;
		require(!TheW3DBufferManager, "original volume CPU closure provider was already published");
		TheW3DBufferManager = &manager;
		try {
			auto *base_vertices = manager.getSlot(W3DBufferManager::VBM_FVF_XYZ, 3);
			auto *base_indices = manager.getSlot(3);
			write_triangle(base_vertices, base_indices);
			auto volume = zh::original_runtime::build_volume_geometry(
				manager, points, 3, silhouette, 6, Vector3(0,0,1), 1);
			require(volume.vertex_count == 6 && volume.index_count == 18,
				"original volume CPU closure did not compose source slots and geometry");
			device.fail_next_buffer_create();
			require(rejected([&] { edge.bind_vertex(volume.vertices->m_VB->m_DX8VertexBuffer); }),
				"original volume CPU closure create failure did not roll back");
			require(!device.resource_counts().total(), "original volume CPU closure retained create failure resources");
			(void)edge.bind_vertex(volume.vertices->m_VB->m_DX8VertexBuffer);
			(void)edge.bind_index(volume.indices->m_IB->m_DX8IndexBuffer);
			(void)edge.bind_vertex(base_vertices->m_VB->m_DX8VertexBuffer);
			(void)edge.bind_index(base_indices->m_IB->m_DX8IndexBuffer);
			edge.release_source_buffers();
			device.fail_buffer_upload_after(0);
			require(rejected([&] { edge.bind_index(volume.indices->m_IB->m_DX8IndexBuffer); }),
				"original volume CPU closure upload failure did not roll back");
			edge.release_source_buffers();
			require(!device.resource_counts().total(), "original volume CPU closure retained upload failure resources");
			(void)edge.bind_vertex(volume.vertices->m_VB->m_DX8VertexBuffer);
			(void)edge.bind_index(volume.indices->m_IB->m_DX8IndexBuffer);
			edge.release_source_buffers();
			zh::original_runtime::release_volume_geometry(manager, volume);
			manager.releaseSlot(base_vertices); manager.releaseSlot(base_indices);
			manager.ReleaseResources();
			require(manager.ReAcquireResources(), "original volume CPU closure resource recreation failed");
			manager.freeAllBuffers();
			TheW3DBufferManager = NULL;
			require(!device.resource_counts().total(), "original volume CPU closure generation retained resources");
		} catch (...) { TheW3DBufferManager = NULL; throw; }
		edge.release_source_buffers();
	}
	require(!TheW3DBufferManager && !device.resource_counts().total(),
		"original volume CPU closure teardown retained provider or resources");
	std::puts("original volume CPU closure: slots=1 geometry=1 retry=2 removal=1 generations=2 resources=0");
}
