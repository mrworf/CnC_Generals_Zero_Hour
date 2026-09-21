#include "zh/renderer/contract.h"

#include <cmath>
#include <cstdint>
#include <cstring>
#include <limits>
#include <tuple>

namespace zh::renderer {
namespace {

ValidationResult failure(std::string message) { return {false, std::move(message)}; }

UInt32 float_bits(float value) noexcept
{
    UInt32 bits = 0;
    std::memcpy(&bits, &value, sizeof(bits));
    return bits;
}

void hash_value(UInt64& hash, UInt64 value) noexcept
{
    constexpr UInt64 offset = 1469598103934665603ULL;
    constexpr UInt64 prime = 1099511628211ULL;
    if (hash == 0) hash = offset;
    for (unsigned shift = 0; shift < 64; shift += 8) {
        hash ^= (value >> shift) & 0xffULL;
        hash *= prime;
    }
}

bool is_depth(TextureFormat format) noexcept
{
    return format == TextureFormat::depth16 || format == TextureFormat::depth24_stencil8 || format == TextureFormat::depth32;
}

bool blend_equal(const BlendState& a, const BlendState& b) noexcept
{
    return std::tie(a.enabled, a.source_color, a.destination_color, a.color_operation,
               a.source_alpha, a.destination_alpha, a.alpha_operation, a.color_write_mask)
        == std::tie(b.enabled, b.source_color, b.destination_color, b.color_operation,
               b.source_alpha, b.destination_alpha, b.alpha_operation, b.color_write_mask);
}

bool depth_equal(const DepthStencilState& a, const DepthStencilState& b) noexcept
{
    return std::tie(a.depth_test, a.depth_write, a.depth_compare, a.stencil_test,
               a.stencil_read_mask, a.stencil_write_mask)
        == std::tie(b.depth_test, b.depth_write, b.depth_compare, b.stencil_test,
               b.stencil_read_mask, b.stencil_write_mask);
}

bool raster_equal(const RasterState& a, const RasterState& b) noexcept
{
    return std::tie(a.cull, a.fill, a.front_face) == std::tie(b.cull, b.fill, b.front_face)
        && float_bits(a.depth_bias) == float_bits(b.depth_bias);
}

UInt32 vertex_element_size(VertexElementFormat format) noexcept
{
    switch (format) {
    case VertexElementFormat::float1: return 4;
    case VertexElementFormat::float2: return 8;
    case VertexElementFormat::float3: return 12;
    case VertexElementFormat::float4: return 16;
    case VertexElementFormat::ubyte4_norm: return 4;
    }
    return 0;
}

ValidationResult validate_original_fvf(const OriginalFvfLayout& layout)
{
    if (layout.stride == 0 || layout.stride > 2048 || layout.stride % 4 != 0)
        return failure("original FVF stride must be aligned and in range 4..2048");
    if (layout.attribute_count == 0 || layout.attribute_count > RendererLimits::vertex_attributes)
        return failure("original FVF attribute count exceeds device limit");
    if (layout.attributes[0].location != 0 || layout.attributes[0].format != VertexElementFormat::float3
        || layout.attributes[0].offset != 0)
        return failure("original FVF must begin with canonical XYZ position");
    std::array<bool, RendererLimits::vertex_attributes> locations{};
    for (UInt32 i = 0; i < layout.attribute_count; ++i) {
        const auto& attr = layout.attributes[i];
        const auto size = vertex_element_size(attr.format);
        if (attr.location >= RendererLimits::vertex_attributes || locations[attr.location] || !size
            || attr.offset % 4 != 0 || attr.offset > layout.stride || size > layout.stride - attr.offset)
            return failure("original FVF attribute has unsupported format, duplicate location or out-of-stride offset");
        locations[attr.location] = true;
        for (UInt32 j = 0; j < i; ++j) {
            const auto& other = layout.attributes[j];
            if (attr.offset < other.offset + vertex_element_size(other.format)
                && other.offset < attr.offset + size)
                return failure("original FVF attributes overlap");
        }
    }
    return {};
}

ValidationResult validate_bindings(const StageBindings& bindings, std::string_view stage)
{
    if (bindings.uniform_count > RendererLimits::uniform_buffers_per_stage)
        return failure(std::string(stage) + " uniform buffer count exceeds limit 4");
    if (bindings.texture_count > RendererLimits::sampled_textures_per_stage)
        return failure(std::string(stage) + " sampled texture count exceeds limit 16");
    for (UInt32 index = 0; index < bindings.uniform_count; ++index) {
        const auto& binding = bindings.uniforms[index];
        if (!binding.buffer || binding.size == 0)
            return failure(std::string(stage) + " uniform binding is empty");
        if (binding.offset > RendererLimits::maximum_upload_bytes || binding.size > RendererLimits::maximum_upload_bytes - binding.offset)
            return failure(std::string(stage) + " uniform binding range exceeds resource limit");
    }
    for (UInt32 index = 0; index < bindings.texture_count; ++index) {
        if (!bindings.textures[index] || !bindings.samplers[index])
            return failure(std::string(stage) + " texture binding requires texture and sampler");
    }
    return {};
}

} // namespace

PipelineKey::PipelineKey(const PipelineDesc& desc) noexcept : descriptor_(desc)
{
    hash_value(hash_, desc.vertex_shader.value());
    hash_value(hash_, desc.fragment_shader.value());
    hash_value(hash_, static_cast<UInt8>(desc.vertex_layout));
    if (desc.vertex_layout == VertexLayout::original_fvf) {
        hash_value(hash_, desc.original_fvf.stride);
        hash_value(hash_, desc.original_fvf.attribute_count);
        for (UInt32 i = 0; i < desc.original_fvf.attribute_count && i < RendererLimits::vertex_attributes; ++i) {
            const auto& attr = desc.original_fvf.attributes[i];
            hash_value(hash_, attr.location);
            hash_value(hash_, static_cast<UInt8>(attr.format));
            hash_value(hash_, attr.offset);
        }
    }
    hash_value(hash_, static_cast<UInt8>(desc.topology));
    hash_value(hash_, desc.blend.enabled);
    hash_value(hash_, static_cast<UInt8>(desc.blend.source_color));
    hash_value(hash_, static_cast<UInt8>(desc.blend.destination_color));
    hash_value(hash_, static_cast<UInt8>(desc.blend.color_operation));
    hash_value(hash_, static_cast<UInt8>(desc.blend.source_alpha));
    hash_value(hash_, static_cast<UInt8>(desc.blend.destination_alpha));
    hash_value(hash_, static_cast<UInt8>(desc.blend.alpha_operation));
    hash_value(hash_, desc.blend.color_write_mask);
    hash_value(hash_, desc.depth_stencil.depth_test);
    hash_value(hash_, desc.depth_stencil.depth_write);
    hash_value(hash_, static_cast<UInt8>(desc.depth_stencil.depth_compare));
    hash_value(hash_, desc.depth_stencil.stencil_test);
    hash_value(hash_, desc.depth_stencil.stencil_read_mask);
    hash_value(hash_, desc.depth_stencil.stencil_write_mask);
    hash_value(hash_, static_cast<UInt8>(desc.raster.cull));
    hash_value(hash_, static_cast<UInt8>(desc.raster.fill));
    hash_value(hash_, static_cast<UInt8>(desc.raster.front_face));
    hash_value(hash_, float_bits(desc.raster.depth_bias));
    hash_value(hash_, static_cast<UInt8>(desc.color_format));
    hash_value(hash_, static_cast<UInt8>(desc.depth_format));
    hash_value(hash_, desc.uses_point_size);
    hash_value(hash_, desc.premultiplied_alpha);
    hash_value(hash_, desc.fog_enabled);
}

bool operator==(const PipelineKey& left, const PipelineKey& right) noexcept
{
    const auto& a = left.descriptor_;
    const auto& b = right.descriptor_;
    return a.vertex_shader == b.vertex_shader && a.fragment_shader == b.fragment_shader
        && a.vertex_layout == b.vertex_layout
        && (a.vertex_layout != VertexLayout::original_fvf || a.original_fvf == b.original_fvf)
        && a.topology == b.topology
        && blend_equal(a.blend, b.blend) && depth_equal(a.depth_stencil, b.depth_stencil)
        && raster_equal(a.raster, b.raster) && a.color_format == b.color_format
        && a.depth_format == b.depth_format && a.uses_point_size == b.uses_point_size
        && a.premultiplied_alpha == b.premultiplied_alpha && a.fog_enabled == b.fog_enabled;
}

ValidationResult validate(const BufferDesc& desc)
{
    if (desc.size == 0) return failure("buffer size must be nonzero");
    if (desc.size > RendererLimits::maximum_upload_bytes) return failure("buffer size exceeds resource limit");
    return {};
}

ValidationResult validate(const TextureDesc& desc)
{
    if (desc.width == 0 || desc.height == 0 || desc.depth_or_layers == 0 || desc.mip_levels == 0)
        return failure("texture dimensions and mip count must be nonzero");
    if (desc.dimension == TextureDimension::cube && desc.depth_or_layers != 6)
        return failure("cube texture requires exactly six layers");
    if (desc.dimension == TextureDimension::texture_2d && desc.depth_or_layers != 1)
        return failure("2D texture requires one layer");
    if (is_depth(desc.format) && desc.sampled && desc.render_target == false)
        return failure("sampled depth texture must be declared as a render target");
    return {};
}

ValidationResult validate(const SamplerDesc& desc)
{
    if (desc.maximum_anisotropy == 0 || desc.maximum_anisotropy > 16)
        return failure("sampler anisotropy must be in range 1..16");
    if (!std::isfinite(desc.maximum_lod) || desc.maximum_lod < 0.0F)
        return failure("sampler maximum LOD must be finite and nonnegative");
    return {};
}

ValidationResult validate(const ShaderDesc& desc)
{
    if (desc.name.empty()) return failure("shader name must not be empty");
    if (desc.uniform_buffers > RendererLimits::uniform_buffers_per_stage)
        return failure("shader uniform buffer count exceeds limit 4");
    if (desc.samplers > RendererLimits::sampled_textures_per_stage)
        return failure("shader sampler count exceeds limit 16");
    return {};
}

ValidationResult validate(const PipelineDesc& desc)
{
    if (!desc.vertex_shader || !desc.fragment_shader) return failure("pipeline requires vertex and fragment shaders");
    if (is_depth(desc.color_format)) return failure("pipeline color format cannot be a depth format");
    if (!is_depth(desc.depth_format)) return failure("pipeline depth format must be a depth format");
    if (desc.topology == PrimitiveTopology::point_list && !desc.uses_point_size)
        return failure("point-list pipeline requires explicit point-size shader behavior");
    if ((desc.blend.color_write_mask & 0xf0U) != 0) return failure("pipeline color mask uses undefined channels");
    if (!std::isfinite(desc.raster.depth_bias)) return failure("pipeline depth bias must be finite");
    if (desc.vertex_layout == VertexLayout::original_fvf) {
        if (auto result = validate_original_fvf(desc.original_fvf); !result) return result;
    } else if (static_cast<UInt8>(desc.vertex_layout) > static_cast<UInt8>(VertexLayout::original_fvf))
        return failure("unsupported vertex layout");
    return {};
}

ValidationResult validate(const RenderPassDesc& desc)
{
    if (desc.width == 0 || desc.height == 0) return failure("render pass extent must be nonzero");
    if (desc.color_target_count == 0 || desc.color_target_count > RendererLimits::color_targets)
        return failure("render pass color target count must be in range 1..4");
    for (UInt32 index = 0; index < desc.color_target_count; ++index)
        if (!desc.color_targets[index]) return failure("render pass color target is invalid");
    if (!desc.depth_target) return failure("render pass requires a depth target");
    return {};
}

ValidationResult validate(const UploadDesc& desc)
{
    if (!desc.destination) return failure("upload destination is invalid");
    if (desc.size == 0) return failure("upload size must be nonzero");
    if (desc.size > RendererLimits::maximum_upload_bytes) return failure("upload size exceeds resource limit");
    if (desc.offset > desc.destination_size || desc.size > desc.destination_size - desc.offset)
        return failure("upload range exceeds destination buffer");
    return {};
}

ValidationResult validate(const DrawDesc& desc, const PipelineDesc& pipeline)
{
    if (!desc.pipeline || !desc.vertex_buffer) return failure("draw requires pipeline and vertex buffer");
    if (desc.vertex_or_index_count == 0) return failure("draw count must be nonzero");
    if (desc.index_element_size != IndexElementSize::uint16 && desc.index_element_size != IndexElementSize::uint32)
        return failure("unsupported draw index element size");
    if (!desc.index_buffer && (desc.first_index != 0 || desc.base_vertex != 0 ||
        desc.index_element_size != IndexElementSize::uint32))
        return failure("non-indexed draw cannot specify index format or offsets");
    if (desc.index_buffer && desc.first_index > (std::numeric_limits<UInt32>::max)() - desc.vertex_or_index_count)
        return failure("indexed draw range overflows");
    if (pipeline.topology == PrimitiveTopology::point_list && (!(desc.point_size > 0.0F) || !std::isfinite(desc.point_size)))
        return failure("point-list draw requires a finite positive point size");
    if (auto result = validate_bindings(desc.vertex_bindings, "vertex"); !result) return result;
    return validate_bindings(desc.fragment_bindings, "fragment");
}

ValidationResult validate_original_fvf_indexed_vertices(const DrawDesc& draw, const PipelineDesc& pipeline,
    UInt64 vertex_bytes, const UInt8* index_bytes, UInt64 index_size)
{
    if (pipeline.vertex_layout != VertexLayout::original_fvf) return {};
    const UInt64 stride = pipeline.original_fvf.stride;
    if (!stride || vertex_bytes < stride) return failure("original FVF vertex buffer is shorter than stride");
    if (!draw.index_buffer) {
        if (draw.vertex_or_index_count > vertex_bytes / stride)
            return failure("original FVF draw exceeds vertex buffer");
        return {};
    }
    const UInt64 width = static_cast<UInt8>(draw.index_element_size);
    if (!index_bytes || (width != 2 && width != 4)
        || static_cast<UInt64>(draw.first_index) + draw.vertex_or_index_count > index_size / width)
        return failure("original FVF index range exceeds index buffer");
    for (UInt64 i = draw.first_index; i < static_cast<UInt64>(draw.first_index) + draw.vertex_or_index_count; ++i) {
        UInt32 index = 0;
        std::memcpy(&index, index_bytes + i * width, static_cast<std::size_t>(width));
        const auto vertex = static_cast<std::int64_t>(index) + draw.base_vertex;
        if (vertex < 0 || static_cast<UInt64>(vertex) >= vertex_bytes / stride)
            return failure("original FVF indexed vertex exceeds source vertex buffer");
    }
    return {};
}

bool requires_bc_fallback(TextureFormat format, bool backend_supports_format) noexcept
{
    const bool compressed = format == TextureFormat::bc1 || format == TextureFormat::bc2 || format == TextureFormat::bc3;
    return compressed && !backend_supports_format;
}

ResizeState::ResizeState(UInt32 width, UInt32 height)
    : width_(width), height_(height), requested_width_(width), requested_height_(height),
      phase_(width == 0 || height == 0 ? ResizePhase::suspended : ResizePhase::stable)
{
}

ValidationResult ResizeState::request(UInt32 width, UInt32 height)
{
    if (phase_ == ResizePhase::recreating) return failure("cannot request resize while recreation is active");
    requested_width_ = width;
    requested_height_ = height;
    phase_ = (width == 0 || height == 0) ? ResizePhase::suspended : ResizePhase::requested;
    return {};
}

ValidationResult ResizeState::begin_recreation()
{
    if (phase_ != ResizePhase::requested) return failure("resize recreation requires a pending nonzero extent");
    phase_ = ResizePhase::recreating;
    return {};
}

ValidationResult ResizeState::complete_recreation(bool succeeded)
{
    if (phase_ != ResizePhase::recreating) return failure("resize completion requires active recreation");
    if (!succeeded) {
        phase_ = ResizePhase::requested;
        return failure("resize recreation failed and remains pending");
    }
    width_ = requested_width_;
    height_ = requested_height_;
    ++generation_;
    phase_ = ResizePhase::stable;
    return {};
}

} // namespace zh::renderer
