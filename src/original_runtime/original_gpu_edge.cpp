#include "original_gpu_edge.h"

#include "dx8vertexbuffer.h"
#include "dx8indexbuffer.h"
#include "dx8wrapper.h"

#include <stdexcept>

namespace zh::original_runtime {
namespace {
thread_local OriginalGpuEdge* active_edge;
}

OriginalGpuEdge::OriginalGpuEdge(renderer::GpuDevice& device)
    : device_(device), previous_(active_edge)
{
    if (previous_) throw std::runtime_error("nested original GPU translation session");
    active_edge = this;
}

OriginalGpuEdge::~OriginalGpuEdge()
{
    active_edge = previous_;
    for (auto& entry : indices_) { device_.destroy(entry.second); entry.first->Release_Ref(); }
    for (auto& entry : vertices_) { device_.destroy(entry.second); entry.first->Release_Ref(); }
}

OriginalGpuEdge& OriginalGpuEdge::required()
{
    if (!active_edge) throw std::runtime_error("original physical call requires a GPU translation session");
    return *active_edge;
}

renderer::BufferHandle OriginalGpuEdge::bind_vertex(const VertexBufferClass* source)
{
    if (!source || source->Type() != BUFFER_TYPE_DX8)
        throw std::runtime_error("original sorting/dynamic vertex physical route is not translated");
    const auto* original = static_cast<const DX8VertexBufferClass*>(source);
    const auto* bytes = original->Get_CPU_Vertex_Buffer();
    const renderer::UInt64 size = static_cast<renderer::UInt64>(source->Get_Vertex_Count()) *
        source->FVF_Info().Get_FVF_Size();
    if (!bytes || !size) throw std::runtime_error("original vertex buffer has no upload bytes");
    auto it = vertices_.find(source);
    if (it == vertices_.end()) {
        auto handle = device_.create_buffer({size, renderer::BufferUsage::vertex, true}, "original WW3D vertex buffer");
        if (!handle) throw std::runtime_error("original vertex buffer device creation failed");
        try { it = vertices_.emplace(source, handle).first; source->Add_Ref(); }
        catch (...) { device_.destroy(handle); throw; }
    }
    if (auto result = device_.upload({it->second, size, 0, size}, bytes); !result)
        throw std::runtime_error("original vertex upload failed: " + result.error);
    return it->second;
}

renderer::BufferHandle OriginalGpuEdge::bind_index(const IndexBufferClass* source)
{
    if (!source || source->Type() != BUFFER_TYPE_DX8)
        throw std::runtime_error("original sorting/dynamic index physical route is not translated");
    const auto* original = static_cast<const DX8IndexBufferClass*>(source);
    const auto* bytes = original->Get_CPU_Index_Buffer();
    const renderer::UInt64 size = static_cast<renderer::UInt64>(source->Get_Index_Count()) * sizeof(*bytes);
    if (!bytes || !size) throw std::runtime_error("original index buffer has no upload bytes");
    auto it = indices_.find(source);
    if (it == indices_.end()) {
        auto handle = device_.create_buffer({size, renderer::BufferUsage::index, true}, "original WW3D 16-bit index buffer");
        if (!handle) throw std::runtime_error("original index buffer device creation failed");
        try { it = indices_.emplace(source, handle).first; source->Add_Ref(); }
        catch (...) { device_.destroy(handle); throw; }
    }
    if (auto result = device_.upload({it->second, size, 0, size}, bytes); !result)
        throw std::runtime_error("original index upload failed: " + result.error);
    return it->second;
}

} // namespace zh::original_runtime
