#include "zh/platform/sdl_gpu_device.h"

#include <SDL3/SDL.h>
#include <SDL3/SDL_gpu.h>

#include <algorithm>
#include <cstring>
#include <fstream>
#include <limits>
#include <stdexcept>
#include <utility>
#include <vector>

namespace zh::renderer {
namespace {

constexpr UInt32 index_mask = 0xffffU;

template <typename HandleType>
HandleType make_handle(std::size_t index, UInt32 generation)
{
    if (index >= index_mask || generation == 0 || generation > index_mask) return {};
    return HandleType((generation << 16U) | static_cast<UInt32>(index + 1));
}

template <typename HandleType>
std::size_t handle_index(HandleType handle)
{
    const auto encoded = handle.value() & index_mask;
    return encoded == 0 ? std::numeric_limits<std::size_t>::max() : encoded - 1U;
}

template <typename HandleType>
UInt32 handle_generation(HandleType handle) { return handle.value() >> 16U; }

template <typename Record>
struct Slot {
    UInt32 generation = 1;
    bool alive = false;
    std::string label;
    Record value{};
};

template <typename HandleType, typename Record>
Slot<Record>* lookup(std::vector<Slot<Record>>& slots, HandleType handle)
{
    const auto index = handle_index(handle);
    if (index >= slots.size()) return nullptr;
    auto& slot = slots[index];
    return slot.alive && slot.generation == handle_generation(handle) ? &slot : nullptr;
}

template <typename HandleType, typename Record>
const Slot<Record>* lookup(const std::vector<Slot<Record>>& slots, HandleType handle)
{
    return lookup(const_cast<std::vector<Slot<Record>>&>(slots), handle);
}

template <typename HandleType, typename Record>
HandleType allocate(std::vector<Slot<Record>>& slots, std::string_view label, Record record)
{
    for (std::size_t index = 0; index < slots.size(); ++index) {
        auto& slot = slots[index];
        if (!slot.alive) {
            slot.alive = true;
            slot.label.assign(label);
            slot.value = std::move(record);
            return make_handle<HandleType>(index, slot.generation);
        }
    }
    if (slots.size() >= index_mask) return {};
    Slot<Record> slot;
    slot.alive = true;
    slot.label.assign(label);
    slot.value = std::move(record);
    slots.push_back(std::move(slot));
    return make_handle<HandleType>(slots.size() - 1, slots.back().generation);
}

template <typename Record>
void retire(Slot<Record>& slot)
{
    slot.alive = false;
    slot.label.clear();
    if (++slot.generation > index_mask) slot.generation = 1;
}

std::string sdl_error(std::string_view operation)
{
    const char* detail = SDL_GetError();
    return std::string(operation) + ": " + (detail && *detail ? detail : "unknown SDL error");
}

SDL_GPUTextureFormat texture_format(TextureFormat format)
{
    switch (format) {
    case TextureFormat::rgba8: return SDL_GPU_TEXTUREFORMAT_R8G8B8A8_UNORM;
    case TextureFormat::bgra8: return SDL_GPU_TEXTUREFORMAT_B8G8R8A8_UNORM;
    case TextureFormat::bc1: return SDL_GPU_TEXTUREFORMAT_BC1_RGBA_UNORM;
    case TextureFormat::bc2: return SDL_GPU_TEXTUREFORMAT_BC2_RGBA_UNORM;
    case TextureFormat::bc3: return SDL_GPU_TEXTUREFORMAT_BC3_RGBA_UNORM;
    case TextureFormat::depth16: return SDL_GPU_TEXTUREFORMAT_D16_UNORM;
    case TextureFormat::depth24_stencil8: return SDL_GPU_TEXTUREFORMAT_D24_UNORM_S8_UINT;
    case TextureFormat::depth32: return SDL_GPU_TEXTUREFORMAT_D32_FLOAT;
    }
    return SDL_GPU_TEXTUREFORMAT_INVALID;
}

bool is_depth(TextureFormat format)
{
    return format == TextureFormat::depth16 || format == TextureFormat::depth24_stencil8
        || format == TextureFormat::depth32;
}

SDL_GPUTextureType texture_type(TextureDimension dimension)
{
    switch (dimension) {
    case TextureDimension::texture_2d: return SDL_GPU_TEXTURETYPE_2D;
    case TextureDimension::cube: return SDL_GPU_TEXTURETYPE_CUBE;
    case TextureDimension::texture_3d: return SDL_GPU_TEXTURETYPE_3D;
    }
    return SDL_GPU_TEXTURETYPE_2D;
}

SDL_GPUFilter filter(Filter value) { return value == Filter::nearest ? SDL_GPU_FILTER_NEAREST : SDL_GPU_FILTER_LINEAR; }
SDL_GPUSamplerMipmapMode mip_filter(Filter value)
{
    return value == Filter::nearest ? SDL_GPU_SAMPLERMIPMAPMODE_NEAREST : SDL_GPU_SAMPLERMIPMAPMODE_LINEAR;
}
SDL_GPUSamplerAddressMode address_mode(AddressMode value)
{
    switch (value) {
    case AddressMode::repeat: return SDL_GPU_SAMPLERADDRESSMODE_REPEAT;
    case AddressMode::mirrored_repeat: return SDL_GPU_SAMPLERADDRESSMODE_MIRRORED_REPEAT;
    case AddressMode::clamp_edge: return SDL_GPU_SAMPLERADDRESSMODE_CLAMP_TO_EDGE;
    case AddressMode::clamp_border: return SDL_GPU_SAMPLERADDRESSMODE_CLAMP_TO_EDGE;
    }
    return SDL_GPU_SAMPLERADDRESSMODE_REPEAT;
}

SDL_GPUPrimitiveType primitive_type(PrimitiveTopology value)
{
    switch (value) {
    case PrimitiveTopology::point_list: return SDL_GPU_PRIMITIVETYPE_POINTLIST;
    case PrimitiveTopology::triangle_list: return SDL_GPU_PRIMITIVETYPE_TRIANGLELIST;
    case PrimitiveTopology::triangle_strip: return SDL_GPU_PRIMITIVETYPE_TRIANGLESTRIP;
    case PrimitiveTopology::triangle_fan: return SDL_GPU_PRIMITIVETYPE_TRIANGLELIST;
    }
    return SDL_GPU_PRIMITIVETYPE_TRIANGLELIST;
}

SDL_GPUCompareOp compare_op(CompareOp value)
{
    switch (value) {
    case CompareOp::never: return SDL_GPU_COMPAREOP_NEVER;
    case CompareOp::less: return SDL_GPU_COMPAREOP_LESS;
    case CompareOp::equal: return SDL_GPU_COMPAREOP_EQUAL;
    case CompareOp::less_equal: return SDL_GPU_COMPAREOP_LESS_OR_EQUAL;
    case CompareOp::greater: return SDL_GPU_COMPAREOP_GREATER;
    case CompareOp::not_equal: return SDL_GPU_COMPAREOP_NOT_EQUAL;
    case CompareOp::greater_equal: return SDL_GPU_COMPAREOP_GREATER_OR_EQUAL;
    case CompareOp::always: return SDL_GPU_COMPAREOP_ALWAYS;
    }
    return SDL_GPU_COMPAREOP_ALWAYS;
}

SDL_GPUBlendFactor blend_factor(BlendFactor value)
{
    switch (value) {
    case BlendFactor::zero: return SDL_GPU_BLENDFACTOR_ZERO;
    case BlendFactor::one: return SDL_GPU_BLENDFACTOR_ONE;
    case BlendFactor::src_color: return SDL_GPU_BLENDFACTOR_SRC_COLOR;
    case BlendFactor::inv_src_color: return SDL_GPU_BLENDFACTOR_ONE_MINUS_SRC_COLOR;
    case BlendFactor::src_alpha: return SDL_GPU_BLENDFACTOR_SRC_ALPHA;
    case BlendFactor::inv_src_alpha: return SDL_GPU_BLENDFACTOR_ONE_MINUS_SRC_ALPHA;
    case BlendFactor::dst_color: return SDL_GPU_BLENDFACTOR_DST_COLOR;
    case BlendFactor::inv_dst_color: return SDL_GPU_BLENDFACTOR_ONE_MINUS_DST_COLOR;
    case BlendFactor::dst_alpha: return SDL_GPU_BLENDFACTOR_DST_ALPHA;
    case BlendFactor::inv_dst_alpha: return SDL_GPU_BLENDFACTOR_ONE_MINUS_DST_ALPHA;
    case BlendFactor::src_alpha_saturate: return SDL_GPU_BLENDFACTOR_SRC_ALPHA_SATURATE;
    }
    return SDL_GPU_BLENDFACTOR_ONE;
}

SDL_GPUBlendOp blend_op(BlendOp value)
{
    switch (value) {
    case BlendOp::add: return SDL_GPU_BLENDOP_ADD;
    case BlendOp::subtract: return SDL_GPU_BLENDOP_SUBTRACT;
    case BlendOp::reverse_subtract: return SDL_GPU_BLENDOP_REVERSE_SUBTRACT;
    case BlendOp::minimum: return SDL_GPU_BLENDOP_MIN;
    case BlendOp::maximum: return SDL_GPU_BLENDOP_MAX;
    }
    return SDL_GPU_BLENDOP_ADD;
}

SDL_GPUCullMode cull_mode(CullMode value)
{
    switch (value) {
    case CullMode::none: return SDL_GPU_CULLMODE_NONE;
    case CullMode::clockwise: return SDL_GPU_CULLMODE_FRONT;
    case CullMode::counter_clockwise: return SDL_GPU_CULLMODE_BACK;
    }
    return SDL_GPU_CULLMODE_NONE;
}

SDL_GPUFillMode fill_mode(FillMode value)
{
    return value == FillMode::wireframe ? SDL_GPU_FILLMODE_LINE : SDL_GPU_FILLMODE_FILL;
}

SDL_GPUFrontFace front_face(Winding value)
{
    return value == Winding::clockwise ? SDL_GPU_FRONTFACE_CLOCKWISE : SDL_GPU_FRONTFACE_COUNTER_CLOCKWISE;
}

SDL_GPUVertexElementFormat vertex_element(VertexElementFormat format)
{
    switch (format) {
    case VertexElementFormat::float1: return SDL_GPU_VERTEXELEMENTFORMAT_FLOAT;
    case VertexElementFormat::float2: return SDL_GPU_VERTEXELEMENTFORMAT_FLOAT2;
    case VertexElementFormat::float3: return SDL_GPU_VERTEXELEMENTFORMAT_FLOAT3;
    case VertexElementFormat::float4: return SDL_GPU_VERTEXELEMENTFORMAT_FLOAT4;
    case VertexElementFormat::ubyte4_norm: return SDL_GPU_VERTEXELEMENTFORMAT_UBYTE4_NORM;
    }
    return SDL_GPU_VERTEXELEMENTFORMAT_INVALID;
}

void vertex_layout(const PipelineDesc& desc, SDL_GPUVertexBufferDescription& buffer,
    std::vector<SDL_GPUVertexAttribute>& attributes)
{
    buffer = {0, 32, SDL_GPU_VERTEXINPUTRATE_VERTEX, 0};
    attributes.clear();
    const auto add = [&](Uint32 location, SDL_GPUVertexElementFormat format, Uint32 offset) {
        attributes.push_back({location, 0, format, offset});
    };
    switch (desc.vertex_layout) {
    case VertexLayout::original_fvf:
        buffer.pitch = desc.original_fvf.stride;
        for (UInt32 i = 0; i < desc.original_fvf.attribute_count; ++i) {
            const auto& attr = desc.original_fvf.attributes[i];
            add(attr.location, vertex_element(attr.format), attr.offset);
        }
        break;
    case VertexLayout::position_color_uv:
        buffer.pitch = 20;
        add(0, SDL_GPU_VERTEXELEMENTFORMAT_FLOAT2, 0);
        add(1, SDL_GPU_VERTEXELEMENTFORMAT_UBYTE4_NORM, 8);
        add(2, SDL_GPU_VERTEXELEMENTFORMAT_FLOAT2, 12);
        break;
    case VertexLayout::water:
        buffer.pitch = 20;
        add(0, SDL_GPU_VERTEXELEMENTFORMAT_FLOAT3, 0);
        add(1, SDL_GPU_VERTEXELEMENTFORMAT_FLOAT2, 12);
        break;
    case VertexLayout::point_sprite:
        buffer.pitch = 16;
        add(0, SDL_GPU_VERTEXELEMENTFORMAT_FLOAT3, 0);
        add(1, SDL_GPU_VERTEXELEMENTFORMAT_UBYTE4_NORM, 12);
        break;
    case VertexLayout::world_mesh:
    case VertexLayout::terrain:
    case VertexLayout::wwshade:
        add(0, SDL_GPU_VERTEXELEMENTFORMAT_FLOAT3, 0);
        add(1, SDL_GPU_VERTEXELEMENTFORMAT_FLOAT3, 12);
        add(2, SDL_GPU_VERTEXELEMENTFORMAT_FLOAT2, 24);
        break;
    }
}

std::vector<UInt8> read_shader(const std::filesystem::path& path)
{
    std::ifstream input(path, std::ios::binary | std::ios::ate);
    if (!input) return {};
    const auto size = input.tellg();
    if (size <= 0) return {};
    std::vector<UInt8> bytes(static_cast<std::size_t>(size));
    input.seekg(0);
    input.read(reinterpret_cast<char*>(bytes.data()), static_cast<std::streamsize>(bytes.size()));
    return input ? bytes : std::vector<UInt8>{};
}

std::string version_string()
{
    const int version = SDL_GetVersion();
    return std::to_string(SDL_VERSIONNUM_MAJOR(version)) + "." + std::to_string(SDL_VERSIONNUM_MINOR(version))
        + "." + std::to_string(SDL_VERSIONNUM_MICRO(version));
}

} // namespace

class SdlGpuDevice::Impl {
public:
    struct BufferRecord { BufferDesc desc; SDL_GPUBuffer* native = nullptr; std::vector<UInt8> shadow; };
    struct TextureRecord { TextureDesc desc; SDL_GPUTexture* native = nullptr; };
    struct SamplerRecord { SamplerDesc desc; SDL_GPUSampler* native = nullptr; };
    struct ShaderRecord { ShaderStage stage{}; UInt32 uniforms = 0; UInt32 samplers = 0; SDL_GPUShader* native = nullptr; };
    struct PipelineRecord { PipelineRecord() : key(PipelineDesc{}) {} PipelineKey key; SDL_GPUGraphicsPipeline* native = nullptr; };

    explicit Impl(SdlGpuOptions value) : options(std::move(value))
    {
        if (options.driver != "vulkan") throw std::invalid_argument("SDL_GPU driver must be 'vulkan' for the Linux port");
        if (options.shader_root.empty()) throw std::invalid_argument("SDL_GPU shader root must not be empty");
        device = SDL_CreateGPUDevice(SDL_GPU_SHADERFORMAT_SPIRV, options.debug, options.driver.c_str());
        if (!device) throw std::runtime_error(sdl_error("SDL_CreateGPUDevice(vulkan) failed"));
        capabilities.sdl_version = version_string();
        capabilities.backend = SDL_GetGPUDeviceDriver(device) ? SDL_GetGPUDeviceDriver(device) : "unknown";
        capabilities.debug_enabled = options.debug;
        const auto props = SDL_GetGPUDeviceProperties(device);
        capabilities.device_name = SDL_GetStringProperty(props, SDL_PROP_GPU_DEVICE_NAME_STRING, "unknown");
        capabilities.driver_name = SDL_GetStringProperty(props, SDL_PROP_GPU_DEVICE_DRIVER_NAME_STRING, "unknown");
        capabilities.driver_version = SDL_GetStringProperty(props, SDL_PROP_GPU_DEVICE_DRIVER_VERSION_STRING, "unknown");
        capabilities.bc1 = supports(TextureFormat::bc1);
        capabilities.bc2 = supports(TextureFormat::bc2);
        capabilities.bc3 = supports(TextureFormat::bc3);
    }

    ~Impl()
    {
        if (window) SDL_ReleaseWindowFromGPUDevice(device, window);
        for (auto& slot : pipelines) if (slot.alive && slot.value.native) SDL_ReleaseGPUGraphicsPipeline(device, slot.value.native);
        for (auto& slot : shaders) if (slot.alive && slot.value.native) SDL_ReleaseGPUShader(device, slot.value.native);
        for (auto& slot : samplers) if (slot.alive && slot.value.native) SDL_ReleaseGPUSampler(device, slot.value.native);
        for (auto& slot : textures) if (slot.alive && slot.value.native) SDL_ReleaseGPUTexture(device, slot.value.native);
        for (auto& slot : buffers) if (slot.alive && slot.value.native) SDL_ReleaseGPUBuffer(device, slot.value.native);
        if (device) SDL_DestroyGPUDevice(device);
    }

    bool supports(TextureFormat format) const
    {
        return SDL_GPUTextureSupportsFormat(device, texture_format(format), SDL_GPU_TEXTURETYPE_2D,
            SDL_GPU_TEXTUREUSAGE_SAMPLER);
    }

    ValidationResult fail(std::string operation, std::string reason, std::string_view label = {})
    {
        last_error = std::move(operation) + ": " + std::move(reason);
        if (!label.empty()) last_error += " [label=" + std::string(label) + "]";
        return {false, last_error};
    }

    SdlGpuOptions options;
    SDL_GPUDevice* device = nullptr;
    SdlGpuCapabilities capabilities;
    std::string last_error;
    bool in_pass = false;
    SDL_Window* window = nullptr;
    SDL_GPUCommandBuffer* command = nullptr;
    SDL_GPURenderPass* render_pass = nullptr;
    TextureHandle last_color;
    UInt32 active_width = 0;
    UInt32 active_height = 0;
    std::vector<Slot<BufferRecord>> buffers;
    std::vector<Slot<TextureRecord>> textures;
    std::vector<Slot<SamplerRecord>> samplers;
    std::vector<Slot<ShaderRecord>> shaders;
    std::vector<Slot<PipelineRecord>> pipelines;
};

SdlGpuDevice::SdlGpuDevice(SdlGpuOptions options) : impl_(std::make_unique<Impl>(std::move(options))) {}
SdlGpuDevice::~SdlGpuDevice() = default;
SdlGpuDevice::SdlGpuDevice(SdlGpuDevice&&) noexcept = default;
SdlGpuDevice& SdlGpuDevice::operator=(SdlGpuDevice&&) noexcept = default;

bool SdlGpuDevice::supports_texture_format(TextureFormat format, TextureDimension dimension, bool sampled, bool render_target) const noexcept
{
    if (texture_format(format)==SDL_GPU_TEXTUREFORMAT_INVALID ||
        (dimension!=TextureDimension::texture_2d && dimension!=TextureDimension::cube &&
         dimension!=TextureDimension::texture_3d)) return false;
    SDL_GPUTextureUsageFlags usage=0;
    if (sampled) usage|=SDL_GPU_TEXTUREUSAGE_SAMPLER;
    if (render_target) usage|=is_depth(format) ? SDL_GPU_TEXTUREUSAGE_DEPTH_STENCIL_TARGET : SDL_GPU_TEXTUREUSAGE_COLOR_TARGET;
    if (!usage) return false;
    return SDL_GPUTextureSupportsFormat(impl_->device, texture_format(format), texture_type(dimension), usage);
}

BufferHandle SdlGpuDevice::create_buffer(const BufferDesc& desc, std::string_view label)
{
    if (auto result = validate(desc); !result) { impl_->fail("create_buffer", result.error, label); return {}; }
    if (label.empty()) { impl_->fail("create_buffer", "label must not be empty"); return {}; }
    if (desc.size > std::numeric_limits<Uint32>::max()) { impl_->fail("create_buffer", "size exceeds SDL_GPU Uint32 limit", label); return {}; }
    Impl::BufferRecord record;
    record.desc = desc;
    record.shadow.resize(static_cast<std::size_t>(desc.size));
    if (desc.usage != BufferUsage::uniform) {
        SDL_GPUBufferCreateInfo info{};
        info.size = static_cast<Uint32>(desc.size);
        info.usage = desc.usage == BufferUsage::index ? SDL_GPU_BUFFERUSAGE_INDEX : SDL_GPU_BUFFERUSAGE_VERTEX;
        record.native = SDL_CreateGPUBuffer(impl_->device, &info);
        if (!record.native) { impl_->fail("create_buffer", sdl_error("SDL_CreateGPUBuffer"), label); return {}; }
        const std::string owned(label);
        SDL_SetGPUBufferName(impl_->device, record.native, owned.c_str());
    }
    auto handle = allocate<BufferHandle>(impl_->buffers, label, std::move(record));
    if (!handle) impl_->fail("create_buffer", "resource table exhausted", label);
    return handle;
}

TextureHandle SdlGpuDevice::create_texture(const TextureDesc& desc, std::string_view label)
{
    if (auto result = validate(desc); !result) { impl_->fail("create_texture", result.error, label); return {}; }
    if (label.empty()) { impl_->fail("create_texture", "label must not be empty"); return {}; }
    const auto format = texture_format(desc.format);
    SDL_GPUTextureUsageFlags usage = 0;
    if (desc.sampled) usage |= SDL_GPU_TEXTUREUSAGE_SAMPLER;
    if (desc.render_target) usage |= is_depth(desc.format) ? SDL_GPU_TEXTUREUSAGE_DEPTH_STENCIL_TARGET : SDL_GPU_TEXTUREUSAGE_COLOR_TARGET;
    if (!SDL_GPUTextureSupportsFormat(impl_->device, format, texture_type(desc.dimension), usage)) {
        impl_->fail("create_texture", "format/usage combination is unsupported by SDL_GPU", label); return {};
    }
    SDL_GPUTextureCreateInfo info{};
    info.type = texture_type(desc.dimension); info.format = format; info.usage = usage;
    info.width = desc.width; info.height = desc.height; info.layer_count_or_depth = desc.depth_or_layers;
    info.num_levels = desc.mip_levels; info.sample_count = SDL_GPU_SAMPLECOUNT_1;
    auto* native = SDL_CreateGPUTexture(impl_->device, &info);
    if (!native) { impl_->fail("create_texture", sdl_error("SDL_CreateGPUTexture"), label); return {}; }
    const std::string owned(label);
    SDL_SetGPUTextureName(impl_->device, native, owned.c_str());
    auto handle = allocate<TextureHandle>(impl_->textures, label, Impl::TextureRecord{desc, native});
    if (!handle) { SDL_ReleaseGPUTexture(impl_->device, native); impl_->fail("create_texture", "resource table exhausted", label); }
    return handle;
}

SamplerHandle SdlGpuDevice::create_sampler(const SamplerDesc& desc, std::string_view label)
{
    if (auto result = validate(desc); !result) { impl_->fail("create_sampler", result.error, label); return {}; }
    if (label.empty()) { impl_->fail("create_sampler", "label must not be empty"); return {}; }
    SDL_GPUSamplerCreateInfo info{};
    info.min_filter = filter(desc.min_filter); info.mag_filter = filter(desc.mag_filter); info.mipmap_mode = mip_filter(desc.mip_filter);
    info.address_mode_u = address_mode(desc.address_u); info.address_mode_v = address_mode(desc.address_v);
    info.address_mode_w = address_mode(desc.address_w); info.max_anisotropy = static_cast<float>(desc.maximum_anisotropy);
    info.enable_anisotropy = desc.maximum_anisotropy > 1;
    info.max_lod = desc.maximum_lod;
    auto* native = SDL_CreateGPUSampler(impl_->device, &info);
    if (!native) { impl_->fail("create_sampler", sdl_error("SDL_CreateGPUSampler"), label); return {}; }
    auto handle = allocate<SamplerHandle>(impl_->samplers, label, Impl::SamplerRecord{desc, native});
    if (!handle) { SDL_ReleaseGPUSampler(impl_->device, native); impl_->fail("create_sampler", "resource table exhausted", label); }
    return handle;
}

ShaderHandle SdlGpuDevice::create_shader(const ShaderDesc& desc, std::string_view label)
{
    if (auto result = validate(desc); !result) { impl_->fail("create_shader", result.error, label); return {}; }
    if (label.empty()) { impl_->fail("create_shader", "label must not be empty"); return {}; }
    const auto path = impl_->options.shader_root / (std::string(desc.name) + ".spv");
    const auto code = read_shader(path);
    if (code.empty()) { impl_->fail("create_shader", "SPIR-V file is missing or empty: " + path.filename().string(), label); return {}; }
    SDL_GPUShaderCreateInfo info{};
    info.code_size = code.size(); info.code = code.data(); info.entrypoint = "main"; info.format = SDL_GPU_SHADERFORMAT_SPIRV;
    info.stage = desc.stage == ShaderStage::vertex ? SDL_GPU_SHADERSTAGE_VERTEX : SDL_GPU_SHADERSTAGE_FRAGMENT;
    info.num_samplers = desc.samplers; info.num_uniform_buffers = desc.uniform_buffers;
    auto* native = SDL_CreateGPUShader(impl_->device, &info);
    if (!native) { impl_->fail("create_shader", sdl_error("SDL_CreateGPUShader"), label); return {}; }
    auto handle = allocate<ShaderHandle>(impl_->shaders, label, Impl::ShaderRecord{desc.stage, desc.uniform_buffers, desc.samplers, native});
    if (!handle) { SDL_ReleaseGPUShader(impl_->device, native); impl_->fail("create_shader", "resource table exhausted", label); }
    return handle;
}

PipelineHandle SdlGpuDevice::create_pipeline(const PipelineKey& key, std::string_view label)
{
    const auto& desc = key.descriptor();
    if (auto result = validate(desc); !result) { impl_->fail("create_pipeline", result.error, label); return {}; }
    if (label.empty()) { impl_->fail("create_pipeline", "label must not be empty"); return {}; }
    for (std::size_t index = 0; index < impl_->pipelines.size(); ++index) {
        const auto& slot = impl_->pipelines[index];
        if (slot.alive && slot.value.key == key) return make_handle<PipelineHandle>(index, slot.generation);
    }
    if (desc.topology == PrimitiveTopology::triangle_fan) {
        impl_->fail("create_pipeline", "triangle-fan input must be CPU-expanded to triangle-list before SDL_GPU", label);
        return {};
    }
    if (desc.raster.fill == FillMode::point) {
        impl_->fail("create_pipeline", "point fill mode is not representable; use point-list topology", label);
        return {};
    }
    auto* vertex = lookup(impl_->shaders, desc.vertex_shader);
    auto* fragment = lookup(impl_->shaders, desc.fragment_shader);
    if (!vertex || vertex->value.stage != ShaderStage::vertex || !fragment || fragment->value.stage != ShaderStage::fragment) {
        impl_->fail("create_pipeline", "shader handles are stale or have the wrong stage", label); return {};
    }
    SDL_GPUVertexBufferDescription vertex_buffer{};
    std::vector<SDL_GPUVertexAttribute> attributes;
    vertex_layout(desc, vertex_buffer, attributes);
    SDL_GPUColorTargetDescription color{};
    color.format = texture_format(desc.color_format);
    color.blend_state.src_color_blendfactor = blend_factor(desc.blend.source_color);
    color.blend_state.dst_color_blendfactor = blend_factor(desc.blend.destination_color);
    color.blend_state.color_blend_op = blend_op(desc.blend.color_operation);
    color.blend_state.src_alpha_blendfactor = blend_factor(desc.blend.source_alpha);
    color.blend_state.dst_alpha_blendfactor = blend_factor(desc.blend.destination_alpha);
    color.blend_state.alpha_blend_op = blend_op(desc.blend.alpha_operation);
    color.blend_state.color_write_mask = desc.blend.color_write_mask;
    color.blend_state.enable_blend = desc.blend.enabled;
    color.blend_state.enable_color_write_mask = true;
    SDL_GPUGraphicsPipelineCreateInfo info{};
    info.vertex_shader = vertex->value.native;
    info.fragment_shader = fragment->value.native;
    info.vertex_input_state = {&vertex_buffer, 1, attributes.data(), static_cast<Uint32>(attributes.size())};
    info.primitive_type = primitive_type(desc.topology);
    info.rasterizer_state.fill_mode = fill_mode(desc.raster.fill);
    info.rasterizer_state.cull_mode = cull_mode(desc.raster.cull);
    info.rasterizer_state.front_face = front_face(desc.raster.front_face);
    info.rasterizer_state.depth_bias_constant_factor = desc.raster.depth_bias;
    info.rasterizer_state.enable_depth_bias = desc.raster.depth_bias != 0.0F;
    info.rasterizer_state.enable_depth_clip = true;
    info.multisample_state.sample_count = SDL_GPU_SAMPLECOUNT_1;
    info.depth_stencil_state.compare_op = compare_op(desc.depth_stencil.depth_compare);
    info.depth_stencil_state.compare_mask = desc.depth_stencil.stencil_read_mask;
    info.depth_stencil_state.write_mask = desc.depth_stencil.stencil_write_mask;
    info.depth_stencil_state.enable_depth_test = desc.depth_stencil.depth_test;
    info.depth_stencil_state.enable_depth_write = desc.depth_stencil.depth_write;
    info.depth_stencil_state.enable_stencil_test = desc.depth_stencil.stencil_test;
    info.target_info = {&color, 1, texture_format(desc.depth_format), true, 0, 0, 0};
    auto* native = SDL_CreateGPUGraphicsPipeline(impl_->device, &info);
    if (!native) { impl_->fail("create_pipeline", sdl_error("SDL_CreateGPUGraphicsPipeline"), label); return {}; }
    Impl::PipelineRecord record;
    record.key = key;
    record.native = native;
    auto handle = allocate<PipelineHandle>(impl_->pipelines, label, std::move(record));
    if (!handle) { SDL_ReleaseGPUGraphicsPipeline(impl_->device, native); impl_->fail("create_pipeline", "resource table exhausted", label); }
    return handle;
}

ValidationResult SdlGpuDevice::upload(const UploadDesc& desc, const void* bytes)
{
    if (auto result = validate(desc); !result) return impl_->fail("upload", result.error);
    auto* buffer = lookup(impl_->buffers, desc.destination);
    if (!buffer) return impl_->fail("upload", "destination buffer handle is stale or destroyed");
    if (!buffer->value.desc.dynamic) return impl_->fail("upload", "destination buffer is not dynamic", buffer->label);
    if (desc.destination_size != buffer->value.desc.size || desc.offset > buffer->value.desc.size
        || desc.size > buffer->value.desc.size - desc.offset)
        return impl_->fail("upload", "range does not match destination buffer", buffer->label);
    if (!bytes) return impl_->fail("upload", "source bytes are null", buffer->label);
    std::memcpy(buffer->value.shadow.data() + static_cast<std::size_t>(desc.offset), bytes, static_cast<std::size_t>(desc.size));
    if (!buffer->value.native) return {};
    if (desc.size > std::numeric_limits<Uint32>::max()) return impl_->fail("upload", "size exceeds SDL_GPU Uint32 limit", buffer->label);
    SDL_GPUTransferBufferCreateInfo transfer_info{SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD, static_cast<Uint32>(desc.size), 0};
    auto* transfer = SDL_CreateGPUTransferBuffer(impl_->device, &transfer_info);
    if (!transfer) return impl_->fail("upload", sdl_error("SDL_CreateGPUTransferBuffer"), buffer->label);
    void* mapped = SDL_MapGPUTransferBuffer(impl_->device, transfer, false);
    if (!mapped) { SDL_ReleaseGPUTransferBuffer(impl_->device, transfer); return impl_->fail("upload", sdl_error("SDL_MapGPUTransferBuffer"), buffer->label); }
    std::memcpy(mapped, bytes, static_cast<std::size_t>(desc.size));
    SDL_UnmapGPUTransferBuffer(impl_->device, transfer);
    auto* command = SDL_AcquireGPUCommandBuffer(impl_->device);
    if (!command) { SDL_ReleaseGPUTransferBuffer(impl_->device, transfer); return impl_->fail("upload", sdl_error("SDL_AcquireGPUCommandBuffer"), buffer->label); }
    auto* pass = SDL_BeginGPUCopyPass(command);
    SDL_GPUTransferBufferLocation source{transfer, 0};
    SDL_GPUBufferRegion destination{buffer->value.native, static_cast<Uint32>(desc.offset), static_cast<Uint32>(desc.size)};
    SDL_UploadToGPUBuffer(pass, &source, &destination, buffer->value.desc.dynamic);
    SDL_EndGPUCopyPass(pass);
    if (!SDL_SubmitGPUCommandBuffer(command)) { SDL_ReleaseGPUTransferBuffer(impl_->device, transfer); return impl_->fail("upload", sdl_error("SDL_SubmitGPUCommandBuffer"), buffer->label); }
    SDL_ReleaseGPUTransferBuffer(impl_->device, transfer);
    return {};
}

ValidationResult SdlGpuDevice::upload_texture(const TextureUploadDesc& desc, const void* bytes)
{
    auto* texture = lookup(impl_->textures, desc.destination);
    if (!texture) return impl_->fail("upload_texture", "destination texture handle is stale or destroyed");
    if (!bytes) return impl_->fail("upload_texture", "source bytes are null", texture->label);
    if (texture->value.desc.dimension != TextureDimension::texture_2d ||
        desc.mip_level >= texture->value.desc.mip_levels)
        return impl_->fail("upload_texture", "unsupported dimension or mip level", texture->label);
    const auto format=texture->value.desc.format;
    const bool compressed=format==TextureFormat::bc1 || format==TextureFormat::bc2 || format==TextureFormat::bc3;
    if (format!=TextureFormat::rgba8 && format!=TextureFormat::bgra8 && !compressed)
        return impl_->fail("upload_texture", "texture format has no color upload path", texture->label);
    if (desc.width != std::max(1U,texture->value.desc.width >> desc.mip_level) ||
        desc.height != std::max(1U,texture->value.desc.height >> desc.mip_level))
        return impl_->fail("upload_texture", "extent does not match destination texture", texture->label);
    const UInt32 block_extent=compressed ? 4U : 1U;
    const UInt32 block_size=SDL_GPUTextureFormatTexelBlockSize(texture_format(format));
    if (!block_size) return impl_->fail("upload_texture", "texture texel block size unavailable", texture->label);
    const UInt64 minimum_pitch=static_cast<UInt64>((desc.width+block_extent-1)/block_extent)*block_size;
    const UInt32 rows=(desc.height+block_extent-1)/block_extent;
    if (desc.row_pitch < minimum_pitch || desc.row_pitch%block_size ||
        desc.size != static_cast<UInt64>(desc.row_pitch)*rows ||
        desc.size > std::numeric_limits<Uint32>::max())
        return impl_->fail("upload_texture", "row pitch or byte count is invalid", texture->label);
    SDL_GPUTransferBufferCreateInfo transfer_info{SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD, static_cast<Uint32>(desc.size), 0};
    auto* transfer = SDL_CreateGPUTransferBuffer(impl_->device, &transfer_info);
    if (!transfer) return impl_->fail("upload_texture", sdl_error("SDL_CreateGPUTransferBuffer"), texture->label);
    void* mapped = SDL_MapGPUTransferBuffer(impl_->device, transfer, false);
    if (!mapped) { SDL_ReleaseGPUTransferBuffer(impl_->device, transfer); return impl_->fail("upload_texture", sdl_error("SDL_MapGPUTransferBuffer"), texture->label); }
    std::memcpy(mapped, bytes, static_cast<std::size_t>(desc.size));
    SDL_UnmapGPUTransferBuffer(impl_->device, transfer);
    auto* command = SDL_AcquireGPUCommandBuffer(impl_->device);
    if (!command) { SDL_ReleaseGPUTransferBuffer(impl_->device, transfer); return impl_->fail("upload_texture", sdl_error("SDL_AcquireGPUCommandBuffer"), texture->label); }
    auto* pass = SDL_BeginGPUCopyPass(command);
    SDL_GPUTextureTransferInfo source{transfer, 0, desc.row_pitch / block_size * block_extent, rows * block_extent};
    SDL_GPUTextureRegion destination{texture->value.native, desc.mip_level, 0, 0, 0, 0, desc.width, desc.height, 1};
    SDL_UploadToGPUTexture(pass, &source, &destination, true);
    SDL_EndGPUCopyPass(pass);
    if (!SDL_SubmitGPUCommandBuffer(command)) { SDL_ReleaseGPUTransferBuffer(impl_->device, transfer); return impl_->fail("upload_texture", sdl_error("SDL_SubmitGPUCommandBuffer"), texture->label); }
    SDL_ReleaseGPUTransferBuffer(impl_->device, transfer);
    return {};
}

ValidationResult SdlGpuDevice::begin_pass(const RenderPassDesc& desc, std::string_view label)
{
    if (impl_->in_pass) return impl_->fail("begin_pass", "render pass is already active", label);
    if (auto result = validate(desc); !result) return impl_->fail("begin_pass", result.error, label);
    std::array<SDL_GPUColorTargetInfo, RendererLimits::color_targets> colors{};
    for (UInt32 index = 0; index < desc.color_target_count; ++index) {
        auto* target = lookup(impl_->textures, desc.color_targets[index]);
        if (!target || !target->value.desc.render_target || is_depth(target->value.desc.format))
            return impl_->fail("begin_pass", "color target is stale, destroyed, or not color-renderable", label);
        if (target->value.desc.width != desc.width || target->value.desc.height != desc.height)
            return impl_->fail("begin_pass", "color target extent does not match pass", target->label);
        colors[index].texture = target->value.native;
        colors[index].clear_color = {0.02F, 0.02F, 0.04F, 1.0F};
        colors[index].load_op = SDL_GPU_LOADOP_CLEAR;
        colors[index].store_op = SDL_GPU_STOREOP_STORE;
        colors[index].cycle = true;
    }
    auto* depth = lookup(impl_->textures, desc.depth_target);
    if (!depth || !depth->value.desc.render_target || !is_depth(depth->value.desc.format))
        return impl_->fail("begin_pass", "depth target is stale, destroyed, or not depth-renderable", label);
    if (depth->value.desc.width != desc.width || depth->value.desc.height != desc.height)
        return impl_->fail("begin_pass", "depth target extent does not match pass", depth->label);
    SDL_GPUDepthStencilTargetInfo depth_info{};
    depth_info.texture = depth->value.native;
    depth_info.clear_depth = 1.0F;
    depth_info.load_op = SDL_GPU_LOADOP_CLEAR;
    depth_info.store_op = SDL_GPU_STOREOP_STORE;
    depth_info.stencil_load_op = SDL_GPU_LOADOP_CLEAR;
    depth_info.stencil_store_op = SDL_GPU_STOREOP_STORE;
    depth_info.cycle = true;
    impl_->command = SDL_AcquireGPUCommandBuffer(impl_->device);
    if (!impl_->command) return impl_->fail("begin_pass", sdl_error("SDL_AcquireGPUCommandBuffer"), label);
    const std::string owned(label);
    SDL_PushGPUDebugGroup(impl_->command, owned.c_str());
    impl_->render_pass = SDL_BeginGPURenderPass(impl_->command, colors.data(), desc.color_target_count, &depth_info);
    if (!impl_->render_pass) {
        SDL_PopGPUDebugGroup(impl_->command);
        SDL_CancelGPUCommandBuffer(impl_->command);
        impl_->command = nullptr;
        return impl_->fail("begin_pass", sdl_error("SDL_BeginGPURenderPass"), label);
    }
    SDL_GPUViewport viewport{0.0F, 0.0F, static_cast<float>(desc.width), static_cast<float>(desc.height), 0.0F, 1.0F};
    SDL_SetGPUViewport(impl_->render_pass, &viewport);
    impl_->in_pass = true;
    impl_->last_color = desc.color_targets[0];
    impl_->active_width = desc.width;
    impl_->active_height = desc.height;
    return {};
}

ValidationResult SdlGpuDevice::draw(const DrawDesc& desc)
{
    if (!impl_->in_pass) return impl_->fail("draw", "draw requires an active render pass");
    auto* pipeline = lookup(impl_->pipelines, desc.pipeline);
    if (!pipeline) return impl_->fail("draw", "pipeline handle is stale or destroyed");
    if (auto result = validate(desc, pipeline->value.key.descriptor()); !result) return impl_->fail("draw", result.error);
    auto* vertex = lookup(impl_->buffers, desc.vertex_buffer);
    if (!vertex || !vertex->value.native || vertex->value.desc.usage != BufferUsage::vertex)
        return impl_->fail("draw", "vertex buffer is stale, destroyed, or has wrong usage");
    if (!desc.index_buffer) {
        if (auto result = validate_original_fvf_indexed_vertices(desc, pipeline->value.key.descriptor(),
                vertex->value.desc.size, nullptr, 0); !result) return impl_->fail("draw", result.error);
    } else {
        auto* index = lookup(impl_->buffers, desc.index_buffer);
        if (!index || index->value.desc.usage != BufferUsage::index)
            return impl_->fail("draw", "index buffer is stale, destroyed, or has wrong usage");
        if (auto result = validate_original_fvf_indexed_vertices(desc, pipeline->value.key.descriptor(),
                vertex->value.desc.size, index->value.shadow.data(), index->value.shadow.size()); !result)
            return impl_->fail("draw", result.error);
    }
    SDL_BindGPUGraphicsPipeline(impl_->render_pass, pipeline->value.native);
    SDL_GPUBufferBinding vertex_binding{vertex->value.native, 0};
    SDL_BindGPUVertexBuffers(impl_->render_pass, 0, &vertex_binding, 1);
    const auto bind_stage = [&](const StageBindings& bindings, bool vertex_stage) -> ValidationResult {
        for (UInt32 index = 0; index < bindings.uniform_count; ++index) {
            auto* uniform = lookup(impl_->buffers, bindings.uniforms[index].buffer);
            if (!uniform || uniform->value.desc.usage != BufferUsage::uniform)
                return impl_->fail("draw", "uniform binding is stale, destroyed, or has wrong usage");
            const auto& binding = bindings.uniforms[index];
            if (binding.offset > uniform->value.shadow.size() || binding.size > uniform->value.shadow.size() - binding.offset)
                return impl_->fail("draw", "uniform range exceeds uploaded shadow");
            const void* bytes = uniform->value.shadow.data() + static_cast<std::size_t>(binding.offset);
            if (vertex_stage) SDL_PushGPUVertexUniformData(impl_->command, index, bytes, static_cast<Uint32>(binding.size));
            else SDL_PushGPUFragmentUniformData(impl_->command, index, bytes, static_cast<Uint32>(binding.size));
        }
        std::array<SDL_GPUTextureSamplerBinding, RendererLimits::sampled_textures_per_stage> native{};
        for (UInt32 index = 0; index < bindings.texture_count; ++index) {
            auto* texture = lookup(impl_->textures, bindings.textures[index]);
            auto* sampler = lookup(impl_->samplers, bindings.samplers[index]);
            if (!texture || !texture->value.desc.sampled || !sampler)
                return impl_->fail("draw", "texture/sampler binding is stale, destroyed, or not sampleable");
            native[index] = {texture->value.native, sampler->value.native};
        }
        if (bindings.texture_count != 0) {
            if (vertex_stage) SDL_BindGPUVertexSamplers(impl_->render_pass, 0, native.data(), bindings.texture_count);
            else SDL_BindGPUFragmentSamplers(impl_->render_pass, 0, native.data(), bindings.texture_count);
        }
        return {};
    };
    if (auto result = bind_stage(desc.vertex_bindings, true); !result) return result;
    if (auto result = bind_stage(desc.fragment_bindings, false); !result) return result;
    if (desc.index_buffer) {
        auto* index = lookup(impl_->buffers, desc.index_buffer);
        if (!index || !index->value.native || index->value.desc.usage != BufferUsage::index)
            return impl_->fail("draw", "index buffer is stale, destroyed, or has wrong usage");
        const UInt64 element_size = static_cast<UInt8>(desc.index_element_size);
        if (static_cast<UInt64>(desc.first_index) + desc.vertex_or_index_count > index->value.desc.size / element_size)
            return impl_->fail("draw", "indexed draw range exceeds index buffer");
        SDL_GPUBufferBinding index_binding{index->value.native, 0};
        SDL_BindGPUIndexBuffer(impl_->render_pass, &index_binding,
            desc.index_element_size == IndexElementSize::uint16 ? SDL_GPU_INDEXELEMENTSIZE_16BIT : SDL_GPU_INDEXELEMENTSIZE_32BIT);
        SDL_DrawGPUIndexedPrimitives(impl_->render_pass, desc.vertex_or_index_count, 1,
            desc.first_index, desc.base_vertex, 0);
    } else {
        SDL_DrawGPUPrimitives(impl_->render_pass, desc.vertex_or_index_count, 1, 0, 0);
    }
    return {};
}

ValidationResult SdlGpuDevice::end_pass()
{
    if (!impl_->in_pass) return impl_->fail("end_pass", "no render pass is active");
    SDL_EndGPURenderPass(impl_->render_pass);
    SDL_PopGPUDebugGroup(impl_->command);
    impl_->render_pass = nullptr;
    impl_->in_pass = false;
    if (!SDL_SubmitGPUCommandBuffer(impl_->command)) {
        impl_->command = nullptr;
        return impl_->fail("end_pass", sdl_error("SDL_SubmitGPUCommandBuffer"));
    }
    impl_->command = nullptr;
    return {};
}

void SdlGpuDevice::destroy(BufferHandle handle)
{
    auto* slot = lookup(impl_->buffers, handle);
    if (!slot) { impl_->fail("destroy", "stale or destroyed buffer handle"); return; }
    if (slot->value.native) SDL_ReleaseGPUBuffer(impl_->device, slot->value.native);
    retire(*slot);
}
void SdlGpuDevice::destroy(TextureHandle handle)
{
    auto* slot = lookup(impl_->textures, handle);
    if (!slot) { impl_->fail("destroy", "stale or destroyed texture handle"); return; }
    if (slot->value.native) SDL_ReleaseGPUTexture(impl_->device, slot->value.native);
    retire(*slot);
}
void SdlGpuDevice::destroy(SamplerHandle handle)
{
    auto* slot = lookup(impl_->samplers, handle);
    if (!slot) { impl_->fail("destroy", "stale or destroyed sampler handle"); return; }
    if (slot->value.native) SDL_ReleaseGPUSampler(impl_->device, slot->value.native);
    retire(*slot);
}
void SdlGpuDevice::destroy(ShaderHandle handle)
{
    auto* slot = lookup(impl_->shaders, handle);
    if (!slot) { impl_->fail("destroy", "stale or destroyed shader handle"); return; }
    if (slot->value.native) SDL_ReleaseGPUShader(impl_->device, slot->value.native);
    retire(*slot);
}
void SdlGpuDevice::destroy(PipelineHandle handle)
{
    auto* slot = lookup(impl_->pipelines, handle);
    if (!slot) { impl_->fail("destroy", "stale or destroyed pipeline handle"); return; }
    if (slot->value.native) SDL_ReleaseGPUGraphicsPipeline(impl_->device, slot->value.native);
    retire(*slot);
}

const std::string& SdlGpuDevice::last_error() const noexcept { return impl_->last_error; }
bool SdlGpuDevice::pass_active() const noexcept { return impl_->in_pass; }
void SdlGpuDevice::record_marker(std::string_view marker)
{
    if (impl_->command) {
        const std::string owned(marker);
        SDL_InsertGPUDebugLabel(impl_->command, owned.c_str());
    }
}
const SdlGpuCapabilities& SdlGpuDevice::capabilities() const noexcept { return impl_->capabilities; }

ValidationResult SdlGpuDevice::claim_window(SDL_Window* window)
{
    if (!window) return impl_->fail("claim_window", "SDL window is null");
    if (impl_->window) return impl_->fail("claim_window", "a window is already claimed");
    if (!SDL_ClaimWindowForGPUDevice(impl_->device, window))
        return impl_->fail("claim_window", sdl_error("SDL_ClaimWindowForGPUDevice"));
    impl_->window = window;
    return {};
}

ValidationResult SdlGpuDevice::present(TextureHandle source_handle)
{
    if (impl_->in_pass) return impl_->fail("present", "cannot present while a render pass is active");
    if (!impl_->window) return impl_->fail("present", "no SDL window is claimed");
    auto* source = lookup(impl_->textures, source_handle);
    if (!source || is_depth(source->value.desc.format)) return impl_->fail("present", "source texture is stale or not color-renderable");
    auto* command = SDL_AcquireGPUCommandBuffer(impl_->device);
    if (!command) return impl_->fail("present", sdl_error("SDL_AcquireGPUCommandBuffer"));
    SDL_GPUTexture* swapchain = nullptr;
    Uint32 width = 0, height = 0;
    if (!SDL_WaitAndAcquireGPUSwapchainTexture(command, impl_->window, &swapchain, &width, &height)) {
        SDL_CancelGPUCommandBuffer(command);
        return impl_->fail("present", sdl_error("SDL_WaitAndAcquireGPUSwapchainTexture"));
    }
    if (!swapchain) {
        if (!SDL_SubmitGPUCommandBuffer(command)) return impl_->fail("present", sdl_error("SDL_SubmitGPUCommandBuffer"));
        return {};
    }
    SDL_GPUBlitInfo blit{};
    blit.source = {source->value.native, 0, 0, 0, 0, source->value.desc.width, source->value.desc.height};
    blit.destination = {swapchain, 0, 0, 0, 0, width, height};
    blit.load_op = SDL_GPU_LOADOP_DONT_CARE;
    blit.filter = SDL_GPU_FILTER_LINEAR;
    SDL_BlitGPUTexture(command, &blit);
    if (!SDL_SubmitGPUCommandBuffer(command)) return impl_->fail("present", sdl_error("SDL_SubmitGPUCommandBuffer"));
    return {};
}

ValidationResult SdlGpuDevice::present_last()
{
    if (!impl_->last_color) return impl_->fail("present", "no completed color target is available");
    return present(impl_->last_color);
}

ValidationResult SdlGpuDevice::wait_idle()
{
    if (impl_->in_pass) return impl_->fail("wait_idle", "cannot wait while a render pass is active");
    if (!SDL_WaitForGPUIdle(impl_->device)) return impl_->fail("wait_idle", sdl_error("SDL_WaitForGPUIdle"));
    return {};
}

std::vector<UInt8> SdlGpuDevice::readback_rgba(TextureHandle source)
{
    if (impl_->in_pass) { impl_->fail("readback_rgba", "cannot read during a render pass"); return {}; }
    auto* texture = lookup(impl_->textures, source);
    if (!texture || !texture->value.desc.render_target || texture->value.desc.format != TextureFormat::rgba8)
    { impl_->fail("readback_rgba", "source must be a live RGBA8 color target"); return {}; }
    const auto width = texture->value.desc.width;
    const auto height = texture->value.desc.height;
    const UInt64 size = static_cast<UInt64>(width) * height * 4;
    if (size > RendererLimits::maximum_upload_bytes || size > std::numeric_limits<Uint32>::max())
    { impl_->fail("readback_rgba", "color target exceeds bounded diagnostic readback"); return {}; }
    SDL_GPUTransferBufferCreateInfo info{SDL_GPU_TRANSFERBUFFERUSAGE_DOWNLOAD, static_cast<Uint32>(size), 0};
    auto* transfer = SDL_CreateGPUTransferBuffer(impl_->device, &info);
    if (!transfer) { impl_->fail("readback_rgba", sdl_error("SDL_CreateGPUTransferBuffer")); return {}; }
    auto* command = SDL_AcquireGPUCommandBuffer(impl_->device);
    if (!command) {
        impl_->fail("readback_rgba", sdl_error("SDL_AcquireGPUCommandBuffer"));
        SDL_ReleaseGPUTransferBuffer(impl_->device, transfer); return {};
    }
    auto* pass = SDL_BeginGPUCopyPass(command);
    SDL_GPUTextureRegion region{texture->value.native, 0, 0, 0, 0, 0, width, height, 1};
    SDL_GPUTextureTransferInfo destination{transfer, 0, width, height};
    SDL_DownloadFromGPUTexture(pass, &region, &destination);
    SDL_EndGPUCopyPass(pass);
    auto* fence = SDL_SubmitGPUCommandBufferAndAcquireFence(command);
    if (!fence) {
        impl_->fail("readback_rgba", sdl_error("SDL_SubmitGPUCommandBufferAndAcquireFence"));
        SDL_ReleaseGPUTransferBuffer(impl_->device, transfer); return {};
    }
    std::vector<UInt8> pixels;
    if (!SDL_WaitForGPUFences(impl_->device, true, &fence, 1))
        impl_->fail("readback_rgba", sdl_error("SDL_WaitForGPUFences"));
    else if (void* mapped = SDL_MapGPUTransferBuffer(impl_->device, transfer, false)) {
        const auto* bytes = static_cast<const UInt8*>(mapped);
        pixels.assign(bytes, bytes + size);
        SDL_UnmapGPUTransferBuffer(impl_->device, transfer);
    } else impl_->fail("readback_rgba", sdl_error("SDL_MapGPUTransferBuffer"));
    SDL_ReleaseGPUFence(impl_->device, fence);
    SDL_ReleaseGPUTransferBuffer(impl_->device, transfer);
    return pixels;
}

void SdlGpuDevice::release_window() noexcept
{
    if (impl_->window) {
        SDL_ReleaseWindowFromGPUDevice(impl_->device, impl_->window);
        impl_->window = nullptr;
    }
}

} // namespace zh::renderer
