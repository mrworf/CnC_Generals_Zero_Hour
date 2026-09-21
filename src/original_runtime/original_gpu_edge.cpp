#include "original_gpu_edge.h"

#include "dx8vertexbuffer.h"
#include "dx8indexbuffer.h"
#include "dx8wrapper.h"
#include "ww3dformat.h"
#include "texture.h"
#include "missingtexture.h"

#include <algorithm>
#include <atomic>
#include <stdexcept>
#include <string>

namespace zh::original_runtime {
namespace {
thread_local OriginalGpuEdge* active_edge;
std::atomic<std::uint64_t> next_generation{1};

renderer::TextureFormat translate_format(WW3DFormat source)
{
    switch (source) {
    case WW3D_FORMAT_DXT1: return renderer::TextureFormat::bc1;
    case WW3D_FORMAT_DXT2:
    case WW3D_FORMAT_DXT3: return renderer::TextureFormat::bc2;
    case WW3D_FORMAT_DXT4:
    case WW3D_FORMAT_DXT5: return renderer::TextureFormat::bc3;
    case WW3D_FORMAT_A8R8G8B8:
    case WW3D_FORMAT_X8R8G8B8: return renderer::TextureFormat::bgra8;
    default: throw std::runtime_error("original texture format has no physical GPU mapping");
    }
}
}

OriginalGpuEdge::OriginalGpuEdge(renderer::GpuDevice& device)
    : device_(device), previous_(active_edge), generation_(next_generation.fetch_add(1))
{
    if (previous_) throw std::runtime_error("nested original GPU translation session");
    active_edge = this;
}

OriginalGpuEdge::~OriginalGpuEdge()
{
    while (!textures_.empty()) {
        auto it=textures_.begin();
        TextureBaseClass* source=it->first;
        if (!it->second.shared_missing) device_.destroy(it->second.handle);
        textures_.erase(it);
        source->Invalidate();
    }
    for (auto& entry : indices_) { device_.destroy(entry.second); entry.first->Release_Ref(); }
    for (auto& entry : vertices_) { device_.destroy(entry.second); entry.first->Release_Ref(); }
    if (missing_texture_) device_.destroy(missing_texture_);
    active_edge = previous_;
}

OriginalGpuEdge& OriginalGpuEdge::required()
{
    if (!active_edge) throw std::runtime_error("original physical call requires a GPU translation session");
    return *active_edge;
}

bool OriginalGpuEdge::supports_texture_format(WW3DFormat format) const noexcept
{
    try { return device_.supports_texture_format(translate_format(format),renderer::TextureDimension::texture_2d,true,false); }
    catch (...) { return false; }
}

renderer::TextureHandle OriginalGpuEdge::create_texture(WW3DFormat format, unsigned width,
    unsigned height, unsigned& mips)
{
    const auto translated=translate_format(format);
    if (!device_.supports_texture_format(translated,renderer::TextureDimension::texture_2d,true,false))
        throw std::runtime_error("original texture format is unsupported by physical device");
    if (!width || !height || width>16384 || height>16384)
        throw std::runtime_error("original texture extent is invalid at physical edge");
    if (!mips) {
        unsigned w=width, h=height;
        mips=1;
        while (w>1 || h>1) { w=std::max(1U,w/2); h=std::max(1U,h/2); ++mips; }
    }
    renderer::TextureDesc desc;
    desc.width=width; desc.height=height; desc.mip_levels=mips;
    desc.dimension=renderer::TextureDimension::texture_2d; desc.format=translated;
    auto handle=device_.create_texture(desc,"original TextureLoader");
    if (!handle) throw std::runtime_error("original texture creation failed: "+device_.last_error());
    return handle;
}

void OriginalGpuEdge::upload_texture(renderer::TextureHandle texture, unsigned level,
    unsigned width, unsigned height, unsigned pitch, const void* bytes, std::size_t size)
{
    renderer::TextureUploadDesc desc{texture,width,height,pitch,size,level};
    const auto result=device_.upload_texture(desc,bytes);
    if (!result) throw std::runtime_error("original texture mip upload failed: "+device_.last_error());
}

void OriginalGpuEdge::discard_texture(renderer::TextureHandle handle) noexcept
{
    if (handle) device_.destroy(handle);
}

void OriginalGpuEdge::publish_texture(TextureBaseClass* source, renderer::TextureHandle texture, bool shared_missing)
{
    if (!source || !texture || textures_.count(source))
        throw std::runtime_error("original texture physical publication is invalid");
    textures_.emplace(source,TextureOwnership{texture,generation_,shared_missing});
}

renderer::TextureHandle OriginalGpuEdge::missing_texture()
{
    if (!missing_texture_) missing_texture_=MissingTexture::_Create_Gpu_Missing_Texture();
    return missing_texture_;
}

bool OriginalGpuEdge::is_missing_texture(const TextureBaseClass* source) noexcept
{
    if (!active_edge) return false;
    const auto it=active_edge->textures_.find(const_cast<TextureBaseClass*>(source));
    return it!=active_edge->textures_.end() && it->second.shared_missing;
}

renderer::TextureHandle OriginalGpuEdge::texture_handle(const TextureBaseClass* source) const
{
    const auto it=textures_.find(const_cast<TextureBaseClass*>(source));
    if (it==textures_.end() || it->second.generation!=generation_)
        throw std::runtime_error("original texture owner is not resident in active device generation");
    return it->second.handle;
}

void OriginalGpuEdge::release_texture_if_owned(TextureBaseClass* source) noexcept
{
    if (!active_edge) return;
    const auto it=active_edge->textures_.find(source);
    if (it==active_edge->textures_.end()) return;
    if (!it->second.shared_missing) active_edge->device_.destroy(it->second.handle);
    active_edge->textures_.erase(it);
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
