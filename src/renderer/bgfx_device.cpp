#include "zh/platform/bgfx_device.h"

#include <bgfx/bgfx.h>

#include <algorithm>
#include <atomic>
#include <cstring>
#include <limits>
#include <stdexcept>
#include <vector>

namespace zh::renderer {
namespace {

std::atomic<bool> runtime_owned{false};
constexpr UInt32 slot_mask = 0xffffU;

template <class HandleType> HandleType encode(std::size_t index, UInt32 generation)
{
    if (index >= slot_mask || generation == 0 || generation > slot_mask) return {};
    return HandleType((generation << 16U) | static_cast<UInt32>(index + 1));
}

template <class Record> struct Slot {
    UInt32 generation = 1;
    bool alive = false;
    Record record{};
};

template <class HandleType, class Record>
Slot<Record>* lookup(std::vector<Slot<Record>>& slots, HandleType handle)
{
    const UInt32 low = handle.value() & slot_mask;
    if (!low || low > slots.size()) return nullptr;
    auto& slot = slots[low - 1];
    return slot.alive && slot.generation == handle.value() >> 16U ? &slot : nullptr;
}

template <class HandleType, class Record>
HandleType insert(std::vector<Slot<Record>>& slots, Record value)
{
    for (std::size_t i = 0; i < slots.size(); ++i) {
        if (!slots[i].alive) {
            slots[i].alive = true;
            slots[i].record = std::move(value);
            return encode<HandleType>(i, slots[i].generation);
        }
    }
    if (slots.size() >= slot_mask) return {};
    slots.push_back({1, true, std::move(value)});
    return encode<HandleType>(slots.size() - 1, 1);
}

template <class Record> void retire(Slot<Record>& slot)
{
    slot.alive = false;
    slot.record = {};
    if (++slot.generation > slot_mask) slot.generation = 1;
}

bgfx::TextureFormat::Enum physical_format(TextureFormat format)
{
    switch (format) {
    case TextureFormat::rgba8: return bgfx::TextureFormat::RGBA8;
    case TextureFormat::bgra8: return bgfx::TextureFormat::BGRA8;
    case TextureFormat::bc1: return bgfx::TextureFormat::BC1;
    case TextureFormat::bc2: return bgfx::TextureFormat::BC2;
    case TextureFormat::bc3: return bgfx::TextureFormat::BC3;
    case TextureFormat::depth16: return bgfx::TextureFormat::D16;
    case TextureFormat::depth24_stencil8: return bgfx::TextureFormat::D24S8;
    case TextureFormat::depth32: return bgfx::TextureFormat::D32F;
    }
    return bgfx::TextureFormat::Unknown;
}

bool is_depth(TextureFormat format)
{
    return format == TextureFormat::depth16 || format == TextureFormat::depth24_stencil8
        || format == TextureFormat::depth32;
}

UInt64 required_texture_bytes(const TextureUploadDesc& upload, TextureFormat format)
{
    if (format == TextureFormat::rgba8 || format == TextureFormat::bgra8)
        return static_cast<UInt64>(upload.height) * upload.row_pitch;
    return 0; // Compressed and depth uploads are admitted only with later format-specific proof.
}

} // namespace

class BgfxGpuDevice::Impl {
public:
    struct BufferRecord { BufferDesc desc; std::vector<UInt8> shadow; };
    struct TextureRecord { TextureDesc desc; bgfx::TextureHandle native = BGFX_INVALID_HANDLE; bool initialized = false; };
    struct SamplerRecord { SamplerDesc desc; };

    explicit Impl(BgfxOptions value) : options(std::move(value))
    {
        if (options.driver != "vulkan") throw std::invalid_argument("bgfx driver must be 'vulkan' for the Linux port");
        if (options.shader_root.empty()) throw std::invalid_argument("bgfx shader root must not be empty");
        bool expected = false;
        if (!runtime_owned.compare_exchange_strong(expected, true))
            throw std::runtime_error("bgfx runtime is already owned by another device");
        bgfx::Init init;
        init.type = bgfx::RendererType::Vulkan;
        init.fallback = false;
        init.swapChain.width = 0;
        init.swapChain.height = 0;
        init.debug = options.debug;
        if (!bgfx::init(init)) {
            runtime_owned.store(false);
            throw std::runtime_error("public bgfx Vulkan initialization failed");
        }
    }

    ~Impl()
    {
        for (auto& slot : textures)
            if (slot.alive && bgfx::isValid(slot.record.native)) bgfx::destroy(slot.record.native);
        bgfx::frame();
        bgfx::shutdown();
        runtime_owned.store(false);
    }

    ValidationResult fail(std::string_view operation, std::string_view reason)
    {
        last_error = std::string(operation) + ": " + std::string(reason);
        return {false, last_error};
    }

    BgfxOptions options;
    std::string last_error;
    std::vector<Slot<BufferRecord>> buffers;
    std::vector<Slot<TextureRecord>> textures;
    std::vector<Slot<SamplerRecord>> samplers;
};

BgfxGpuDevice::BgfxGpuDevice(BgfxOptions options) : impl_(std::make_unique<Impl>(std::move(options))) {}
BgfxGpuDevice::~BgfxGpuDevice() = default;

bool BgfxGpuDevice::supports_texture_format(TextureFormat format, TextureDimension dimension,
    bool sampled, bool render_target) const noexcept
{
    if (dimension != TextureDimension::texture_2d || (!sampled && !render_target)) return false;
    const auto native = physical_format(format);
    if (native == bgfx::TextureFormat::Unknown) return false;
    const UInt64 flags = render_target ? BGFX_TEXTURE_RT : BGFX_TEXTURE_NONE;
    return bgfx::isTextureValid(1, false, 1, native, flags);
}

BufferHandle BgfxGpuDevice::create_buffer(const BufferDesc& desc, std::string_view)
{
    if (auto result = validate(desc); !result) { impl_->fail("create_buffer", result.error); return {}; }
    // Vertex layout and index width are supplied only by the later draw. Keep
    // a bounded upload shadow and materialize the correctly typed native buffer
    // at the draw boundary instead of guessing its layout here.
    auto handle = insert<BufferHandle>(impl_->buffers, {desc, std::vector<UInt8>(static_cast<std::size_t>(desc.size))});
    if (!handle) impl_->fail("create_buffer", "opaque resource slot budget exhausted");
    return handle;
}

TextureHandle BgfxGpuDevice::create_texture(const TextureDesc& desc, std::string_view)
{
    if (auto result = validate(desc); !result) { impl_->fail("create_texture", result.error); return {}; }
    if (desc.dimension != TextureDimension::texture_2d || desc.width > UINT16_MAX || desc.height > UINT16_MAX
        || desc.mip_levels != 1 || !supports_texture_format(desc.format, desc.dimension, desc.sampled, desc.render_target)) {
        impl_->fail("create_texture", "unsupported public bgfx texture dimension, extent, mip or format/usage");
        return {};
    }
    const auto flags = desc.render_target ? BGFX_TEXTURE_RT : BGFX_TEXTURE_NONE;
    auto native = bgfx::createTexture2D(static_cast<UInt16>(desc.width), static_cast<UInt16>(desc.height),
        false, 1, physical_format(desc.format), flags);
    if (!bgfx::isValid(native)) { impl_->fail("create_texture", "public bgfx texture allocation failed"); return {}; }
    auto handle = insert<TextureHandle>(impl_->textures, {desc, native, false});
    if (!handle) { bgfx::destroy(native); impl_->fail("create_texture", "opaque resource slot budget exhausted"); }
    return handle;
}

SamplerHandle BgfxGpuDevice::create_sampler(const SamplerDesc& desc, std::string_view)
{
    if (auto result = validate(desc); !result) { impl_->fail("create_sampler", result.error); return {}; }
    // bgfx encodes samplers as validated flags at the setTexture binding edge.
    auto handle = insert<SamplerHandle>(impl_->samplers, {desc});
    if (!handle) impl_->fail("create_sampler", "opaque resource slot budget exhausted");
    return handle;
}

ValidationResult BgfxGpuDevice::upload(const UploadDesc& desc, const void* bytes)
{
    if (auto result = validate(desc); !result) return impl_->fail("upload", result.error);
    auto* slot = lookup(impl_->buffers, desc.destination);
    if (!slot) return impl_->fail("upload", "stale or foreign buffer handle");
    if (!bytes || desc.destination_size != slot->record.desc.size)
        return impl_->fail("upload", "null bytes or mismatched destination size");
    std::memcpy(slot->record.shadow.data() + desc.offset, bytes, static_cast<std::size_t>(desc.size));
    return {};
}

ValidationResult BgfxGpuDevice::upload_texture(const TextureUploadDesc& desc, const void* bytes)
{
    auto* slot = lookup(impl_->textures, desc.destination);
    if (!slot) return impl_->fail("upload_texture", "stale or foreign texture handle");
    const auto& texture = slot->record.desc;
    const UInt64 required = required_texture_bytes(desc, texture.format);
    if (!bytes || desc.mip_level != 0 || !desc.width || !desc.height
        || desc.width != texture.width || desc.height != texture.height
        || desc.row_pitch < static_cast<UInt64>(desc.width) * 4U || desc.row_pitch > UINT16_MAX
        || !required || desc.size < required || desc.size > RendererLimits::maximum_upload_bytes)
        return impl_->fail("upload_texture", "unsupported or out-of-bounds texture upload");
    bgfx::updateTexture2D(slot->record.native, 0, 0, 0, 0,
        static_cast<UInt16>(desc.width), static_cast<UInt16>(desc.height),
        bgfx::copy(bytes, static_cast<UInt32>(desc.size)), static_cast<UInt16>(desc.row_pitch));
    slot->record.initialized = true;
    return {};
}

void BgfxGpuDevice::destroy(BufferHandle handle)
{
    if (auto* slot = lookup(impl_->buffers, handle)) retire(*slot);
}
void BgfxGpuDevice::destroy(TextureHandle handle)
{
    if (auto* slot = lookup(impl_->textures, handle)) { bgfx::destroy(slot->record.native); retire(*slot); }
}
void BgfxGpuDevice::destroy(SamplerHandle handle)
{
    if (auto* slot = lookup(impl_->samplers, handle)) retire(*slot);
}

// Later M30 slices install physical pass and draw behavior. These explicit
// failures prevent accidental success while the resource slice is reviewable.
ShaderHandle BgfxGpuDevice::create_shader(const ShaderDesc&, std::string_view)
{ impl_->fail("create_shader", "shader submission is not installed"); return {}; }
PipelineHandle BgfxGpuDevice::create_pipeline(const PipelineKey&, std::string_view)
{ impl_->fail("create_pipeline", "pipeline submission is not installed"); return {}; }
ValidationResult BgfxGpuDevice::begin_pass(const RenderPassDesc&, std::string_view)
{ return impl_->fail("begin_pass", "ordered target submission is not installed"); }
std::pair<UInt32,UInt32> BgfxGpuDevice::active_pass_extent() const noexcept { return {0,0}; }
ValidationResult BgfxGpuDevice::set_viewport(const ViewportDesc&)
{ return impl_->fail("set_viewport", "ordered target submission is not installed"); }
ValidationResult BgfxGpuDevice::clear_viewport(const ViewportClearDesc&)
{ return impl_->fail("clear_viewport", "ordered target submission is not installed"); }
ValidationResult BgfxGpuDevice::draw(const DrawDesc&)
{ return impl_->fail("draw", "shader draw submission is not installed"); }
ValidationResult BgfxGpuDevice::end_pass()
{ return impl_->fail("end_pass", "ordered target submission is not installed"); }
ValidationResult BgfxGpuDevice::present(TextureHandle)
{ return impl_->fail("present", "window presentation is not installed"); }
void BgfxGpuDevice::destroy(ShaderHandle) {}
void BgfxGpuDevice::destroy(PipelineHandle) {}
const std::string& BgfxGpuDevice::last_error() const noexcept { return impl_->last_error; }
bool BgfxGpuDevice::pass_active() const noexcept { return false; }
void BgfxGpuDevice::record_marker(std::string_view) {}
ValidationResult BgfxGpuDevice::claim_window(SDL_Window*)
{ return impl_->fail("claim_window", "window presentation is not installed"); }
ValidationResult BgfxGpuDevice::wait_idle() { bgfx::frame(); return {}; }
void BgfxGpuDevice::release_window() noexcept {}

} // namespace zh::renderer
