#include "PreRTS.h"
#include "W3DDevice/GameClient/W3DBufferManager.h"
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
	auto *points=static_cast<VertexFormatXYZ *>(vertex_lock.Get_Vertex_Array())+vertices->m_start;
	points[0]={0,0,0}; points[1]={1,0,0}; points[2]={0,1,0};
	DX8IndexBufferClass::WriteLockClass index_lock(indices->m_IB->m_DX8IndexBuffer);
	auto *triangles=index_lock.Get_Index_Array()+indices->m_start;
	triangles[0]=0; triangles[1]=1; triangles[2]=2;
}
}

extern "C" void zh_probe_volumetric_buffer()
{
	require(std::getenv("ZH_M22_VOLUME_BUFFER_PROFILE"), "original volume-buffer profile missing");
	zh::renderer::RecordingGpuDevice device;
	for (Int generation=0; generation!=2; ++generation) {
		zh::original_runtime::OriginalGpuEdge edge(device);
		W3DBufferManager manager;
		require(!TheW3DBufferManager, "original volume buffer provider was already published");
		TheW3DBufferManager=&manager;
		try {
			require(rejected([&]{ manager.getSlot(W3DBufferManager::MAX_FVF, 3); }),
				"original volume buffer accepted an unsupported vertex format");
			require(rejected([&]{ manager.getSlot(W3DBufferManager::VBM_FVF_XYZ, 0); }) &&
				rejected([&]{ manager.getSlot(MAX_IB_SIZES*MIN_SLOT_SIZE+1); }),
				"original volume buffer accepted an invalid slot extent");

			auto *vertices=manager.getSlot(W3DBufferManager::VBM_FVF_XYZ, 3);
			auto *indices=manager.getSlot(3);
			require(vertices && indices && vertices->m_size==MIN_SLOT_SIZE && indices->m_size==MIN_SLOT_SIZE,
				"original volume buffer did not preserve source slot rounding");
			write_triangle(vertices,indices);
			device.fail_next_buffer_create();
			require(rejected([&]{ edge.bind_vertex(vertices->m_VB->m_DX8VertexBuffer); }),
				"original volume buffer vertex create failure did not roll back");
			require(!device.resource_counts().total(), "original volume buffer create failure retained resources");
			(void)edge.bind_vertex(vertices->m_VB->m_DX8VertexBuffer);
			device.fail_buffer_upload_after(0);
			require(rejected([&]{ edge.bind_index(indices->m_IB->m_DX8IndexBuffer); }),
				"original volume buffer index upload failure did not surface");
			edge.release_source_buffers();
			(void)edge.bind_vertex(vertices->m_VB->m_DX8VertexBuffer);
			(void)edge.bind_index(indices->m_IB->m_DX8IndexBuffer);
			edge.release_source_buffers();

			W3DBufferManager foreign;
			auto *foreign_slot=foreign.getSlot(W3DBufferManager::VBM_FVF_XYZ, 3);
			require(rejected([&]{ manager.releaseSlot(foreign_slot); }),
				"original volume buffer accepted a foreign slot");
			foreign.releaseSlot(foreign_slot);
			manager.releaseSlot(vertices); manager.releaseSlot(indices);
			require(rejected([&]{ manager.releaseSlot(vertices); }) && rejected([&]{ manager.releaseSlot(indices); }),
				"original volume buffer accepted a duplicate slot release");
			manager.ReleaseResources();
			require(manager.ReAcquireResources(), "original volume buffer resource recreation failed");
			manager.freeAllBuffers();
			TheW3DBufferManager=NULL;
			require(!device.resource_counts().total(), "original volume buffer generation retained Recording resources");
		} catch (...) { TheW3DBufferManager=NULL; throw; }
		edge.release_source_buffers();
	}
	require(!TheW3DBufferManager && !device.resource_counts().total(),
		"original volume buffer teardown retained provider or resources");
	std::puts("original volume buffer: source-slots=1 retry=2 negatives=1 removal=1 generations=2 resources=0");
}
