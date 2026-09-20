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

PipelineHandle SdlGpuDevice::create_pipeline(const PipelineKey&, std::string_view label)
{
    impl_->fail("create_pipeline", "graphics pipeline support is delivered by M14 slice 02", label);
    return {};
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

ValidationResult SdlGpuDevice::begin_pass(const RenderPassDesc&, std::string_view label)
{
    return impl_->fail("begin_pass", "render pass support is delivered by M14 slice 02", label);
}
ValidationResult SdlGpuDevice::draw(const DrawDesc&) { return impl_->fail("draw", "draw support is delivered by M14 slice 02"); }
ValidationResult SdlGpuDevice::end_pass() { return impl_->fail("end_pass", "render pass support is delivered by M14 slice 02"); }

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
void SdlGpuDevice::record_marker(std::string_view) {}
const SdlGpuCapabilities& SdlGpuDevice::capabilities() const noexcept { return impl_->capabilities; }

} // namespace zh::renderer
