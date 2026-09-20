#pragma once

#include "zh/foundation/types.h"

#include <array>
#include <cstddef>
#include <functional>
#include <string>
#include <string_view>

namespace zh::renderer {

using foundation::UInt8;
using foundation::UInt16;
using foundation::UInt32;
using foundation::UInt64;

template <typename Tag>
class Handle {
public:
    constexpr Handle() = default;
    explicit constexpr Handle(UInt32 value) : value_(value) {}
    constexpr UInt32 value() const noexcept { return value_; }
    constexpr explicit operator bool() const noexcept { return value_ != 0; }
    friend constexpr bool operator==(Handle left, Handle right) noexcept { return left.value_ == right.value_; }
    friend constexpr bool operator!=(Handle left, Handle right) noexcept { return !(left == right); }
private:
    UInt32 value_ = 0;
};

struct BufferTag;
struct TextureTag;
struct SamplerTag;
struct ShaderTag;
struct PipelineTag;
using BufferHandle = Handle<BufferTag>;
using TextureHandle = Handle<TextureTag>;
using SamplerHandle = Handle<SamplerTag>;
using ShaderHandle = Handle<ShaderTag>;
using PipelineHandle = Handle<PipelineTag>;

enum class BufferUsage : UInt8 { vertex, index, uniform, transfer };
enum class TextureDimension : UInt8 { texture_2d, cube, texture_3d };
enum class TextureFormat : UInt8 { rgba8, bgra8, bc1, bc2, bc3, depth16, depth24_stencil8, depth32 };
enum class Filter : UInt8 { nearest, linear };
enum class AddressMode : UInt8 { repeat, mirrored_repeat, clamp_edge, clamp_border };
enum class ShaderStage : UInt8 { vertex, fragment };
enum class PrimitiveTopology : UInt8 { point_list, triangle_list, triangle_strip, triangle_fan };
enum class VertexLayout : UInt8 { position_color_uv, world_mesh, terrain, water, point_sprite, wwshade };
enum class CompareOp : UInt8 { never, less, equal, less_equal, greater, not_equal, greater_equal, always };
enum class BlendFactor : UInt8 { zero, one, src_color, inv_src_color, src_alpha, inv_src_alpha, dst_color, inv_dst_color, dst_alpha, inv_dst_alpha, src_alpha_saturate };
enum class BlendOp : UInt8 { add, subtract, reverse_subtract, minimum, maximum };
enum class CullMode : UInt8 { none, clockwise, counter_clockwise };
enum class FillMode : UInt8 { solid, wireframe, point };
enum class Winding : UInt8 { clockwise, counter_clockwise };
enum class TextureOrigin : UInt8 { top_left };
enum class CoordinateHandedness : UInt8 { left_handed };

struct RendererLimits {
    static constexpr UInt32 uniform_buffers_per_stage = 4;
    static constexpr UInt32 sampled_textures_per_stage = 16;
    static constexpr UInt32 color_targets = 4;
    static constexpr UInt32 vertex_attributes = 16;
    static constexpr UInt64 maximum_upload_bytes = 64ULL * 1024ULL * 1024ULL;
};

struct BufferDesc {
    UInt64 size = 0;
    BufferUsage usage = BufferUsage::vertex;
    bool dynamic = false;
};

struct TextureDesc {
    UInt32 width = 0;
    UInt32 height = 0;
    UInt32 depth_or_layers = 1;
    UInt32 mip_levels = 1;
    TextureDimension dimension = TextureDimension::texture_2d;
    TextureFormat format = TextureFormat::rgba8;
    bool render_target = false;
    bool sampled = true;
};

struct SamplerDesc {
    Filter min_filter = Filter::linear;
    Filter mag_filter = Filter::linear;
    Filter mip_filter = Filter::linear;
    AddressMode address_u = AddressMode::repeat;
    AddressMode address_v = AddressMode::repeat;
    AddressMode address_w = AddressMode::repeat;
    UInt8 maximum_anisotropy = 1;
};

struct ShaderDesc {
    ShaderStage stage = ShaderStage::vertex;
    std::string_view name;
    UInt32 uniform_buffers = 0;
    UInt32 samplers = 0;
};

struct BlendState {
    bool enabled = false;
    BlendFactor source_color = BlendFactor::one;
    BlendFactor destination_color = BlendFactor::zero;
    BlendOp color_operation = BlendOp::add;
    BlendFactor source_alpha = BlendFactor::one;
    BlendFactor destination_alpha = BlendFactor::zero;
    BlendOp alpha_operation = BlendOp::add;
    UInt8 color_write_mask = 0x0f;
};

struct DepthStencilState {
    bool depth_test = true;
    bool depth_write = true;
    CompareOp depth_compare = CompareOp::less_equal;
    bool stencil_test = false;
    UInt8 stencil_read_mask = 0xff;
    UInt8 stencil_write_mask = 0xff;
};

struct RasterState {
    CullMode cull = CullMode::counter_clockwise;
    FillMode fill = FillMode::solid;
    Winding front_face = Winding::clockwise;
    float depth_bias = 0.0F;
};

struct PipelineDesc {
    ShaderHandle vertex_shader;
    ShaderHandle fragment_shader;
    VertexLayout vertex_layout = VertexLayout::world_mesh;
    PrimitiveTopology topology = PrimitiveTopology::triangle_list;
    BlendState blend;
    DepthStencilState depth_stencil;
    RasterState raster;
    TextureFormat color_format = TextureFormat::rgba8;
    TextureFormat depth_format = TextureFormat::depth24_stencil8;
    bool uses_point_size = false;
    bool premultiplied_alpha = false;
    bool fog_enabled = false;
};

class PipelineKey {
public:
    explicit PipelineKey(const PipelineDesc& desc) noexcept;
    const PipelineDesc& descriptor() const noexcept { return descriptor_; }
    UInt64 stable_hash() const noexcept { return hash_; }
    friend bool operator==(const PipelineKey& left, const PipelineKey& right) noexcept;
    friend bool operator!=(const PipelineKey& left, const PipelineKey& right) noexcept { return !(left == right); }
private:
    PipelineDesc descriptor_;
    UInt64 hash_ = 0;
};

struct UniformBinding {
    BufferHandle buffer;
    UInt64 offset = 0;
    UInt64 size = 0;
};

struct StageBindings {
    std::array<UniformBinding, RendererLimits::uniform_buffers_per_stage> uniforms{};
    UInt32 uniform_count = 0;
    std::array<TextureHandle, RendererLimits::sampled_textures_per_stage> textures{};
    std::array<SamplerHandle, RendererLimits::sampled_textures_per_stage> samplers{};
    UInt32 texture_count = 0;
};

struct RenderPassDesc {
    std::array<TextureHandle, RendererLimits::color_targets> color_targets{};
    UInt32 color_target_count = 0;
    TextureHandle depth_target;
    UInt32 width = 0;
    UInt32 height = 0;
};

struct UploadDesc {
    BufferHandle destination;
    UInt64 destination_size = 0;
    UInt64 offset = 0;
    UInt64 size = 0;
};

struct DrawDesc {
    PipelineHandle pipeline;
    BufferHandle vertex_buffer;
    BufferHandle index_buffer;
    UInt32 vertex_or_index_count = 0;
    float point_size = 0.0F;
    StageBindings vertex_bindings;
    StageBindings fragment_bindings;
};

struct ValidationResult {
    bool valid = true;
    std::string error;
    operator bool() const noexcept { return valid; }
};

ValidationResult validate(const BufferDesc& desc);
ValidationResult validate(const TextureDesc& desc);
ValidationResult validate(const SamplerDesc& desc);
ValidationResult validate(const ShaderDesc& desc);
ValidationResult validate(const PipelineDesc& desc);
ValidationResult validate(const RenderPassDesc& desc);
ValidationResult validate(const UploadDesc& desc);
ValidationResult validate(const DrawDesc& desc, const PipelineDesc& pipeline);
bool requires_bc_fallback(TextureFormat format, bool backend_supports_format) noexcept;

struct RendererConventions {
    static constexpr CoordinateHandedness handedness = CoordinateHandedness::left_handed;
    static constexpr float depth_minimum = 0.0F;
    static constexpr float depth_maximum = 1.0F;
    static constexpr Winding front_face = Winding::clockwise;
    static constexpr TextureOrigin texture_origin = TextureOrigin::top_left;
    static constexpr float ui_half_pixel_offset = -0.5F;
    static constexpr bool colors_are_argb8 = true;
    static constexpr bool premultiplied_alpha_preserved = true;
    static constexpr bool fog_distance_is_view_space = true;
};

constexpr UInt32 pack_argb8(UInt8 alpha, UInt8 red, UInt8 green, UInt8 blue) noexcept
{
    return (static_cast<UInt32>(alpha) << 24U) | (static_cast<UInt32>(red) << 16U)
        | (static_cast<UInt32>(green) << 8U) | static_cast<UInt32>(blue);
}

enum class ResizePhase : UInt8 { stable, requested, recreating, suspended };

class ResizeState {
public:
    ResizeState(UInt32 width, UInt32 height);
    ValidationResult request(UInt32 width, UInt32 height);
    ValidationResult begin_recreation();
    ValidationResult complete_recreation(bool succeeded);
    ResizePhase phase() const noexcept { return phase_; }
    UInt32 width() const noexcept { return width_; }
    UInt32 height() const noexcept { return height_; }
    UInt64 generation() const noexcept { return generation_; }
private:
    UInt32 width_;
    UInt32 height_;
    UInt32 requested_width_;
    UInt32 requested_height_;
    UInt64 generation_ = 1;
    ResizePhase phase_ = ResizePhase::stable;
};

class GpuDevice {
public:
    virtual ~GpuDevice() = default;
    virtual BufferHandle create_buffer(const BufferDesc& desc, std::string_view label) = 0;
    virtual TextureHandle create_texture(const TextureDesc& desc, std::string_view label) = 0;
    virtual SamplerHandle create_sampler(const SamplerDesc& desc, std::string_view label) = 0;
    virtual ShaderHandle create_shader(const ShaderDesc& desc, std::string_view label) = 0;
    virtual PipelineHandle create_pipeline(const PipelineKey& key, std::string_view label) = 0;
    virtual ValidationResult upload(const UploadDesc& desc, const void* bytes) = 0;
    virtual ValidationResult begin_pass(const RenderPassDesc& desc, std::string_view label) = 0;
    virtual ValidationResult draw(const DrawDesc& desc) = 0;
    virtual ValidationResult end_pass() = 0;
    virtual void destroy(BufferHandle handle) = 0;
    virtual void destroy(TextureHandle handle) = 0;
    virtual void destroy(SamplerHandle handle) = 0;
    virtual void destroy(ShaderHandle handle) = 0;
    virtual void destroy(PipelineHandle handle) = 0;
    virtual const std::string& last_error() const noexcept = 0;
    virtual bool pass_active() const noexcept = 0;
    virtual void record_marker(std::string_view marker) = 0;
};

} // namespace zh::renderer

namespace std {
template <>
struct hash<zh::renderer::PipelineKey> {
    size_t operator()(const zh::renderer::PipelineKey& key) const noexcept { return static_cast<size_t>(key.stable_hash()); }
};
} // namespace std
