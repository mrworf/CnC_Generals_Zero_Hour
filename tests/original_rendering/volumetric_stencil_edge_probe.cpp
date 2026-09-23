#include "PreRTS.h"
#include "W3DDevice/GameClient/W3DBufferManager.h"
#include "WW3D2/dx8fvf.h"
#include "original_gpu_edge.h"
#include "zh/platform/bgfx_device.h"
#include "zh/renderer/recording_device.h"

#include <cstdio>
#include <cstdlib>
#include <stdexcept>
#include <string>

namespace {
void require(bool value, const char *message) { if (!value) throw std::runtime_error(message); }
template <typename F> bool rejected(F action) { try { action(); } catch (const std::runtime_error &) { return true; } return false; }

void write_triangle(W3DBufferManager::W3DVertexBufferSlot *vertices,
    W3DBufferManager::W3DIndexBufferSlot *indices)
{
    DX8VertexBufferClass::WriteLockClass vertex_lock(vertices->m_VB->m_DX8VertexBuffer);
    auto *points=static_cast<VertexFormatXYZ *>(vertex_lock.Get_Vertex_Array())+vertices->m_start;
    points[0]={-0.5F,-0.5F,0}; points[1]={0.5F,-0.5F,0}; points[2]={0,0.5F,0};
    DX8IndexBufferClass::WriteLockClass index_lock(indices->m_IB->m_DX8IndexBuffer);
    auto *triangles=index_lock.Get_Index_Array()+indices->m_start;
    triangles[0]=0; triangles[1]=1; triangles[2]=2;
}

void run_physical_generation()
{
    zh::renderer::BgfxOptions options;
    options.shader_root=ZH_BGFX_SHADER_DIR;
    zh::renderer::BgfxGpuDevice device(options);
    zh::original_runtime::OriginalGpuEdge edge(device);
    W3DBufferManager manager;
    require(!TheW3DBufferManager, "physical volume-stencil provider was already published");
    TheW3DBufferManager=&manager;
    auto *vertices=manager.getSlot(W3DBufferManager::VBM_FVF_XYZ,3);
    auto *indices=manager.getSlot(3);
    write_triangle(vertices,indices);
    zh::renderer::TextureDesc target; target.width=16; target.height=16; target.render_target=true; target.sampled=false;
    const auto color=device.create_texture(target,"physical volume edge color");
    target.format=zh::renderer::TextureFormat::depth24_stencil8;
    const auto depth=device.create_texture(target,"physical volume edge depth");
    try {
        require(color && depth, "physical volume-stencil targets were not created");
        edge.bind_frame_targets(color,depth,16,16);
        edge.begin_source_frame(true,true,0,0,0,1);
        edge.draw_volume_stencil(vertices->m_VB->m_DX8VertexBuffer,
            indices->m_IB->m_DX8IndexBuffer,0,3,0,3,0x80);
        edge.end_source_frame(false);
        edge.release_source_buffers(); edge.release_volume_stencil();
        manager.releaseSlot(vertices); manager.releaseSlot(indices); manager.freeAllBuffers();
        TheW3DBufferManager=NULL;
        device.destroy(depth); device.destroy(color);
        require(device.wait_idle(), "physical volume-stencil device did not become idle");
        require(device.live_resource_count()==0, "physical volume-stencil bridge retained resources");
    } catch (...) {
        TheW3DBufferManager=NULL;
        throw;
    }
}
}

extern "C" void zh_probe_volumetric_stencil_edge()
{
    require(std::getenv("ZH_M22_VOLUME_STENCIL_EDGE_PROFILE"), "original volume-stencil edge profile missing");
    zh::renderer::RecordingGpuDevice device;
    for (Int generation=0;generation!=2;++generation) {
        zh::original_runtime::OriginalGpuEdge edge(device);
        W3DBufferManager manager, foreign;
        require(!TheW3DBufferManager, "original volume-stencil provider was already published");
        TheW3DBufferManager=&manager;
        auto *vertices=manager.getSlot(W3DBufferManager::VBM_FVF_XYZ,3);
        auto *indices=manager.getSlot(3);
        auto *unsupported_vertices=manager.getSlot(W3DBufferManager::VBM_FVF_XYZDUV,3);
        auto *foreign_vertices=foreign.getSlot(W3DBufferManager::VBM_FVF_XYZ,3);
        auto *foreign_indices=foreign.getSlot(3);
        write_triangle(vertices,indices); write_triangle(foreign_vertices,foreign_indices);
        zh::renderer::TextureDesc target; target.width=16; target.height=16; target.render_target=true; target.sampled=false;
        auto color=device.create_texture(target,"volume edge color"); target.format=zh::renderer::TextureFormat::depth24_stencil8;
        auto depth=device.create_texture(target,"volume edge depth");
        try {
            edge.bind_frame_targets(color,depth,16,16);
            require(rejected([&] { edge.draw_volume_stencil(vertices->m_VB->m_DX8VertexBuffer,
                indices->m_IB->m_DX8IndexBuffer,0,3,0,3,0x80); }), "volume edge accepted an inactive frame");
            edge.begin_source_frame(true,true,0,0,0,1);
            require(rejected([&] { edge.draw_volume_stencil(unsupported_vertices->m_VB->m_DX8VertexBuffer,
                indices->m_IB->m_DX8IndexBuffer,0,3,0,3,0x80); }), "volume edge accepted unsupported source layout");
            require(rejected([&] { edge.draw_volume_stencil(foreign_vertices->m_VB->m_DX8VertexBuffer,
                foreign_indices->m_IB->m_DX8IndexBuffer,0,3,0,3,0x80); }), "volume edge accepted foreign provider buffers");
            TheW3DBufferManager=NULL;
            require(rejected([&] { edge.draw_volume_stencil(vertices->m_VB->m_DX8VertexBuffer,
                indices->m_IB->m_DX8IndexBuffer,0,3,0,3,0x80); }), "volume edge accepted an absent provider");
            TheW3DBufferManager=&manager;
            device.set_texture_format_supported(zh::renderer::TextureFormat::depth24_stencil8,false);
            require(rejected([&] { edge.draw_volume_stencil(vertices->m_VB->m_DX8VertexBuffer,
                indices->m_IB->m_DX8IndexBuffer,0,3,0,3,0x80); }), "volume edge accepted unsupported D24S8");
            device.set_texture_format_supported(zh::renderer::TextureFormat::depth24_stencil8,true);
            device.fail_next_shader_create();
            require(rejected([&] { edge.draw_volume_stencil(vertices->m_VB->m_DX8VertexBuffer,
                indices->m_IB->m_DX8IndexBuffer,0,3,0,3,0x80); }), "volume edge shader failure missing");
            device.fail_next_pipeline_create();
            require(rejected([&] { edge.draw_volume_stencil(vertices->m_VB->m_DX8VertexBuffer,
                indices->m_IB->m_DX8IndexBuffer,0,3,0,3,0x80); }), "volume edge pipeline failure missing");
            device.fail_next_buffer_create();
            require(rejected([&] { edge.draw_volume_stencil(vertices->m_VB->m_DX8VertexBuffer,
                indices->m_IB->m_DX8IndexBuffer,0,3,0,3,0x80); }), "volume edge buffer-create failure missing");
            edge.release_volume_stencil();
            device.fail_buffer_upload_after(0);
            require(rejected([&] { edge.draw_volume_stencil(vertices->m_VB->m_DX8VertexBuffer,
                indices->m_IB->m_DX8IndexBuffer,0,3,0,3,0x80); }), "volume edge upload failure missing");
            device.fail_next_draw();
            require(rejected([&] { edge.draw_volume_stencil(vertices->m_VB->m_DX8VertexBuffer,
                indices->m_IB->m_DX8IndexBuffer,0,3,0,3,0x80); }), "volume edge draw failure missing");
            const auto start=device.snapshot();
            edge.draw_volume_stencil(vertices->m_VB->m_DX8VertexBuffer,indices->m_IB->m_DX8IndexBuffer,0,3,0,3,0x80);
            const auto trace=device.snapshot().substr(start.size());
            const auto increment=trace.find("OriginalGpuEdge::volume_stencil increment");
            const auto decrement=trace.find("OriginalGpuEdge::volume_stencil decrement");
            const auto composite=trace.find("OriginalGpuEdge::volume_stencil composite");
            require(increment!=std::string::npos && decrement>increment && composite>decrement,
                "volume edge public three-pass order drifted");
            edge.end_source_frame(false);
            edge.release_source_buffers(); edge.release_volume_stencil();
            manager.releaseSlot(vertices); manager.releaseSlot(indices); manager.releaseSlot(unsupported_vertices);
            foreign.releaseSlot(foreign_vertices); foreign.releaseSlot(foreign_indices);
            manager.freeAllBuffers(); foreign.freeAllBuffers(); TheW3DBufferManager=NULL;
            device.destroy(depth); device.destroy(color);
            require(!device.resource_counts().total(), "volume edge generation retained resources");
        } catch (...) {
            TheW3DBufferManager=NULL;
            throw;
        }
    }
    require(!TheW3DBufferManager && !device.resource_counts().total(), "volume edge provider removal leaked resources");
    if (std::getenv("ZH_M22_VOLUME_STENCIL_EDGE_PHYSICAL")) {
        for (Int generation=0;generation!=2;++generation) run_physical_generation();
        std::puts("original volume stencil edge: physical-dispatch=1 generations=2 resources=0");
    }
    std::puts("original volume stencil edge: public-dispatch=1 retries=5 foreign=1 generations=2 resources=0");
}
