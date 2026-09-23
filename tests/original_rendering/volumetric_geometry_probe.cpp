#include "PreRTS.h"
#include "full_w3d/volume_geometry_cpu.h"
#include "original_gpu_edge.h"
#include "zh/renderer/recording_device.h"

#include <cstdio>
#include <cstdlib>
#include <stdexcept>

namespace {
void require(bool value, const char *message) { if (!value) throw std::runtime_error(message); }
template <typename F> bool rejected(F action) { try { action(); } catch (const std::runtime_error &) { return true; } return false; }
}

extern "C" void zh_probe_volumetric_geometry()
{
	require(std::getenv("ZH_M22_VOLUME_GEOMETRY_PROFILE"), "original volume-geometry profile missing");
	const Vector3 vertices[] = { Vector3(0,0,0), Vector3(1,0,0), Vector3(0,1,0) };
	const UnsignedShort silhouette[] = { 0,1, 1,2, 2,0 };
	const UnsignedShort degenerate[] = { 0,0, 1,2, 2,1 };
	const UnsignedShort malformed[] = { 0,1, 1,2, 2,3 };
	zh::renderer::RecordingGpuDevice device;
	for (Int generation = 0; generation != 2; ++generation) {
		zh::original_runtime::OriginalGpuEdge edge(device);
		W3DBufferManager manager;
		require(!TheW3DBufferManager, "original volume geometry provider was already published");
		TheW3DBufferManager = &manager;
		try {
			W3DBufferManager foreign;
			require(rejected([&] { zh::original_runtime::build_volume_geometry(manager, vertices, 3, degenerate, 6, Vector3(0,0,1), 1); }) &&
				rejected([&] { zh::original_runtime::build_volume_geometry(manager, vertices, 3, malformed, 6, Vector3(0,0,1), 1); }) &&
				rejected([&] { zh::original_runtime::build_volume_geometry(manager, vertices, 16385, silhouette, 6, Vector3(0,0,1), 1); }) &&
				rejected([&] { zh::original_runtime::build_volume_geometry(manager, vertices, 3, silhouette, 5, Vector3(0,0,1), 1); }) &&
				rejected([&] { zh::original_runtime::build_volume_geometry(manager, vertices, 3, silhouette, 6, Vector3(0,0,1), 0); }) &&
				rejected([&] { zh::original_runtime::build_volume_geometry(foreign, vertices, 3, silhouette, 6, Vector3(0,0,1), 1); }),
				"original volume geometry accepted malformed, degenerate, or overflow input");
			auto slots = zh::original_runtime::build_volume_geometry(manager, vertices, 3, silhouette, 6, Vector3(0,0,1), 1);
			require(slots.vertices && slots.indices && slots.vertex_count == 6 && slots.index_count == 18,
				"original volume geometry did not preserve source closed-strip topology");
			{
				DX8IndexBufferClass::WriteLockClass index_lock(slots.indices->m_IB->m_DX8IndexBuffer);
				auto *indices = index_lock.Get_Index_Array() + slots.indices->m_start;
				const UnsignedShort winding[] = { 0,1,2, 2,1,3, 2,3,4, 4,3,5, 4,5,0, 0,5,1 };
				for (Int i = 0; i != slots.index_count; ++i)
					require(indices[i] == winding[i], "original volume geometry winding drifted");
			}
			device.fail_next_buffer_create();
			require(rejected([&] { edge.bind_vertex(slots.vertices->m_VB->m_DX8VertexBuffer); }),
				"original volume geometry create failure did not roll back");
			require(!device.resource_counts().total(), "original volume geometry retained create failure resources");
			(void)edge.bind_vertex(slots.vertices->m_VB->m_DX8VertexBuffer);
			device.fail_buffer_upload_after(0);
			require(rejected([&] { edge.bind_index(slots.indices->m_IB->m_DX8IndexBuffer); }),
				"original volume geometry upload failure did not surface");
			edge.release_source_buffers();
			(void)edge.bind_vertex(slots.vertices->m_VB->m_DX8VertexBuffer);
			(void)edge.bind_index(slots.indices->m_IB->m_DX8IndexBuffer);
			edge.release_source_buffers();
			zh::original_runtime::release_volume_geometry(manager, slots);
			manager.ReleaseResources();
			require(manager.ReAcquireResources(), "original volume geometry resource recreation failed");
			manager.freeAllBuffers();
			TheW3DBufferManager = NULL;
			require(!device.resource_counts().total(), "original volume geometry generation retained resources");
		} catch (...) { TheW3DBufferManager = NULL; throw; }
		edge.release_source_buffers();
	}
	require(!TheW3DBufferManager && !device.resource_counts().total(),
		"original volume geometry teardown retained provider or resources");
	std::puts("original volume geometry: source-topology=1 retry=2 negatives=1 removal=1 generations=2 resources=0");
}
