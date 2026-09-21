#include "original_gpu_edge.h"

#include "dx8vertexbuffer.h"
#include "dx8indexbuffer.h"
#include "dx8wrapper.h"
#include "ww3dformat.h"

#include <stdexcept>
#include <string>

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

bool OriginalGpuEdge::supports_texture_format(WW3DFormat format) const noexcept
{
    renderer::TextureFormat translated;
    switch (format) {
    case WW3D_FORMAT_DXT1: translated=renderer::TextureFormat::bc1; break;
    case WW3D_FORMAT_DXT2:
    case WW3D_FORMAT_DXT3: translated=renderer::TextureFormat::bc2; break;
    case WW3D_FORMAT_DXT4:
    case WW3D_FORMAT_DXT5: translated=renderer::TextureFormat::bc3; break;
    case WW3D_FORMAT_A8R8G8B8:
    case WW3D_FORMAT_X8R8G8B8: translated=renderer::TextureFormat::bgra8; break;
    default: return false;
    }
    return device_.supports_texture_format(translated,renderer::TextureDimension::texture_2d,true,false);
}

[[noreturn]] void OriginalGpuEdge::texture_creation_unavailable(WW3DFormat format,
    unsigned width, unsigned height, unsigned mips, unsigned reduction)
{
    device_.record_marker("original TextureLoader selected format=" + std::to_string(format) +
        " width=" + std::to_string(width) + " height=" + std::to_string(height) +
        " mips=" + std::to_string(mips) + " reduction=" + std::to_string(reduction));
    throw std::runtime_error("original texture creation requires a GPU device translation (M22 05B2B)");
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
