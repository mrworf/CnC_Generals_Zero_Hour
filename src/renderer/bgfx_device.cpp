#include "zh/platform/bgfx_device.h"
#include "zh/renderer/bgfx_uniform_layout.h"
#include "bgfx_transaction_state.h"

#include <bgfx/bgfx.h>
#include <SDL3/SDL.h>

#include <algorithm>
#include <atomic>
#include <cassert>
#include <cmath>
#include <cstring>
#include <fstream>
#include <exception>
#include <limits>
#include <map>
#include <new>
#include <regex>
#include <set>
#include <stdexcept>
#include <type_traits>
#include <vector>

namespace zh::renderer {
namespace {

std::atomic<bool> runtime_owned{false};
std::atomic<UInt64> next_transaction_device{0};
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
HandleType insert(std::vector<Slot<Record>>& slots, Record value, bool append_only = false)
{
    for (std::size_t i = 0; i < slots.size(); ++i) {
        if (!append_only && !slots[i].alive && slots[i].generation) {
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
    case TextureFormat::bgr5a1: return bgfx::TextureFormat::BGR5A1;
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

UInt32 clear_rgba(const std::array<float,4>& value)
{
    const auto channel = [](float component) { return static_cast<UInt32>(std::lround(component * 255.0f)); };
    return (channel(value[0]) << 24U) | (channel(value[1]) << 16U)
        | (channel(value[2]) << 8U) | channel(value[3]);
}

UInt64 required_texture_bytes(const TextureUploadDesc& upload, TextureFormat format)
{
    if (format == TextureFormat::rgba8 || format == TextureFormat::bgra8 ||
        format == TextureFormat::bgr5a1)
        return static_cast<UInt64>(upload.height) * upload.row_pitch;
    return 0; // Compressed and depth uploads are admitted only with later format-specific proof.
}

UInt32 maximum_2d_mip_levels(UInt32 width, UInt32 height)
{
    UInt32 levels = 1;
    while (width > 1 || height > 1) {
        width = std::max(1U, width >> 1U);
        height = std::max(1U, height >> 1U);
        ++levels;
    }
    return levels;
}

bool vertex_layout_for(const PipelineDesc& desc, bgfx::VertexLayout& layout, UInt32& stride)
{
    layout.begin();
    switch (desc.vertex_layout) {
    case VertexLayout::position_color_uv:
        layout.add(bgfx::Attrib::Position, 2, bgfx::AttribType::Float)
            .add(bgfx::Attrib::Color0, 4, bgfx::AttribType::Uint8, true)
            .add(bgfx::Attrib::TexCoord0, 2, bgfx::AttribType::Float);
        stride = 20;
        break;
    case VertexLayout::point_sprite:
        layout.add(bgfx::Attrib::Position, 3, bgfx::AttribType::Float)
            .add(bgfx::Attrib::Color0, 4, bgfx::AttribType::Uint8, true);
        stride = 16;
        break;
    case VertexLayout::world_mesh:
    case VertexLayout::terrain:
    case VertexLayout::wwshade:
        layout.add(bgfx::Attrib::Position, 3, bgfx::AttribType::Float)
            .add(bgfx::Attrib::Normal, 3, bgfx::AttribType::Float)
            .add(bgfx::Attrib::TexCoord0, 2, bgfx::AttribType::Float);
        stride = 32;
        break;
    case VertexLayout::water:
        layout.add(bgfx::Attrib::Position, 3, bgfx::AttribType::Float)
            .add(bgfx::Attrib::TexCoord0, 2, bgfx::AttribType::Float);
        stride = 20;
        break;
    case VertexLayout::original_fvf:
    {
        const auto& source = desc.original_fvf;
        std::vector<VertexAttributeDesc> attributes(source.attributes.begin(),
            source.attributes.begin() + source.attribute_count);
        std::sort(attributes.begin(), attributes.end(), [](const auto& a, const auto& b) {
            return a.offset < b.offset;
        });
        UInt32 cursor = 0;
        for (const auto& attribute : attributes) {
            if (attribute.offset < cursor || attribute.offset - cursor > UINT8_MAX) return false;
            if (attribute.offset != cursor) layout.skip(static_cast<UInt8>(attribute.offset - cursor));
            bgfx::Attrib::Enum semantic;
            switch (attribute.location) {
            case 0: semantic = bgfx::Attrib::Position; break;
            case 1: semantic = bgfx::Attrib::Normal; break;
            case 2: semantic = bgfx::Attrib::Color0; break;
            case 3: semantic = bgfx::Attrib::Color1; break;
            case 4: semantic = bgfx::Attrib::TexCoord0; break;
            case 5: semantic = bgfx::Attrib::TexCoord1; break;
            default: return false;
            }
            const bool bytes = attribute.format == VertexElementFormat::ubyte4_norm;
            const UInt8 count = bytes ? 4 : static_cast<UInt8>(attribute.format) + 1;
            layout.add(semantic, count, bytes ? bgfx::AttribType::Uint8 : bgfx::AttribType::Float, bytes);
            cursor = attribute.offset + (bytes ? 4 : count * 4);
        }
        if (source.stride < cursor || source.stride - cursor > UINT8_MAX) return false;
        if (source.stride != cursor) layout.skip(static_cast<UInt8>(source.stride - cursor));
        stride = source.stride;
        break;
    }
    }
    layout.end();
    return layout.getStride() == stride;
}

bool range_written(const std::vector<UInt8>& written, UInt64 offset, UInt64 size)
{
    return offset <= written.size() && size <= written.size() - offset
        && std::all_of(written.begin() + static_cast<std::size_t>(offset),
            written.begin() + static_cast<std::size_t>(offset + size), [](UInt8 byte) { return byte != 0; });
}

UInt64 depth_compare_state(CompareOp op)
{
    switch (op) {
    case CompareOp::never: return BGFX_STATE_DEPTH_TEST_NEVER;
    case CompareOp::less: return BGFX_STATE_DEPTH_TEST_LESS;
    case CompareOp::equal: return BGFX_STATE_DEPTH_TEST_EQUAL;
    case CompareOp::less_equal: return BGFX_STATE_DEPTH_TEST_LEQUAL;
    case CompareOp::greater: return BGFX_STATE_DEPTH_TEST_GREATER;
    case CompareOp::not_equal: return BGFX_STATE_DEPTH_TEST_NOTEQUAL;
    case CompareOp::greater_equal: return BGFX_STATE_DEPTH_TEST_GEQUAL;
    case CompareOp::always: return BGFX_STATE_DEPTH_TEST_ALWAYS;
    }
    return BGFX_STATE_DEPTH_TEST_NEVER;
}

UInt32 stencil_compare_state(CompareOp op)
{
    switch (op) {
    case CompareOp::never: return BGFX_STENCIL_TEST_NEVER;
    case CompareOp::less: return BGFX_STENCIL_TEST_LESS;
    case CompareOp::equal: return BGFX_STENCIL_TEST_EQUAL;
    case CompareOp::less_equal: return BGFX_STENCIL_TEST_LEQUAL;
    case CompareOp::greater: return BGFX_STENCIL_TEST_GREATER;
    case CompareOp::not_equal: return BGFX_STENCIL_TEST_NOTEQUAL;
    case CompareOp::greater_equal: return BGFX_STENCIL_TEST_GEQUAL;
    case CompareOp::always: return BGFX_STENCIL_TEST_ALWAYS;
    }
    return BGFX_STENCIL_TEST_NEVER;
}

UInt32 stencil_op_code(StencilOp op)
{
    switch (op) {
    case StencilOp::zero: return 0;
    case StencilOp::keep: return 1;
    case StencilOp::replace: return 2;
    case StencilOp::increment_wrap: return 3;
    case StencilOp::increment_clamp: return 4;
    case StencilOp::decrement_wrap: return 5;
    case StencilOp::decrement_clamp: return 6;
    case StencilOp::invert: return 7;
    }
    return 1;
}

struct UniformMetadata { std::string name, source_path; UInt16 offset = 0; };

bool normalize_shader_uniform_identifiers(std::vector<char>& data, std::vector<UniformMetadata>& metadata)
{
    // M29's glslang reflection names include '.' for nested UBO
    // members. They are valid reflection paths but bgfx::createUniform's
    // public identifier grammar rejects them. The container header names are
    // CPU metadata; SPIR-V payload, offsets and manifest remain unchanged.
    if (data.size() < 22) return false;
    std::size_t position = 20;
    const UInt32 count = static_cast<UInt8>(data[position])
        | (static_cast<UInt32>(static_cast<UInt8>(data[position + 1])) << 8U);
    position += 2;
    if (count > 1024) return false;
    std::set<std::string> identifiers;
    const auto alpha = [](char c) { return (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z'); };
    const auto digit = [](char c) { return c >= '0' && c <= '9'; };
    for (UInt32 i = 0; i < count; ++i) {
        if (position >= data.size()) return false;
        const auto length = static_cast<UInt8>(data[position++]);
        if (!length || position + length + 10 > data.size()) return false;
        std::string source_path(data.data() + position, length);
        for (std::size_t j = 0; j < length; ++j) {
            char& c = data[position + j];
            if (j == 0 && !alpha(c) && c != '_') return false;
            if (j != 0 && c == '.') c = '_';
            else if (j != 0 && !alpha(c) && !digit(c) && c != '_') return false;
        }
        std::string name(data.data() + position, length);
        if (!identifiers.emplace(name).second) return false;
        const std::size_t header = position + length;
        const auto offset = static_cast<UInt16>(static_cast<UInt8>(data[header + 2])
            | (static_cast<UInt16>(static_cast<UInt8>(data[header + 3])) << 8U));
        metadata.push_back({std::move(name), std::move(source_path), offset});
        position += length + 10;
    }
    return position + 4 <= data.size();
}

struct ShaderManifest {
    ShaderStage stage = ShaderStage::vertex;
    std::map<std::string, UInt32> block_binding;
    std::map<std::string, std::pair<UInt32, UInt32>> texture_binding;
};

bool read_shader_manifest(const std::filesystem::path& path, ShaderManifest& manifest)
{
    std::ifstream input(path, std::ios::binary | std::ios::ate);
    if (!input || input.tellg() <= 0 || input.tellg() > 65536) return false;
    std::string contents(static_cast<std::size_t>(input.tellg()), '\0');
    input.seekg(0); input.read(contents.data(), static_cast<std::streamsize>(contents.size()));
    if (!input || contents.find("\"uniform_blocks\"") == std::string::npos
        || contents.find("\"textures\"") == std::string::npos) return false;
    if (contents.find("\"stage\":\"vertex\"") != std::string::npos) manifest.stage = ShaderStage::vertex;
    else if (contents.find("\"stage\":\"fragment\"") != std::string::npos) manifest.stage = ShaderStage::fragment;
    else return false;
    const std::regex texture("\\\"bgfx_stage\\\":([0-9]+),\\\"kind\\\":\\\"[^\\\"]+\\\",\\\"name\\\":\\\"([^\\\"]+)\\\",\\\"source_binding\\\":([0-9]+)");
    if (!read_bgfx_uniform_bindings(contents, manifest.block_binding)) return false;
    for (std::sregex_iterator it(contents.begin(), contents.end(), texture), end; it != end; ++it)
        if (!manifest.texture_binding.emplace((*it)[2], std::pair<UInt32, UInt32>{
                static_cast<UInt32>(std::stoul((*it)[3])), static_cast<UInt32>(std::stoul((*it)[1]))}).second)
            return false;
    return manifest.block_binding.size() <= RendererLimits::uniform_buffers_per_stage
        && manifest.texture_binding.size() <= RendererLimits::sampled_textures_per_stage;
}

UInt32 sampler_flags(const SamplerDesc& desc)
{
    const auto address = [](AddressMode mode, UInt32 mirror, UInt32 clamp, UInt32 border) {
        switch (mode) {
        case AddressMode::repeat: return UInt32{0};
        case AddressMode::mirrored_repeat: return mirror;
        case AddressMode::clamp_edge: return clamp;
        case AddressMode::clamp_border: return border;
        }
        return UInt32{0};
    };
    UInt32 flags = address(desc.address_u, BGFX_SAMPLER_U_MIRROR, BGFX_SAMPLER_U_CLAMP, BGFX_SAMPLER_U_BORDER)
        | address(desc.address_v, BGFX_SAMPLER_V_MIRROR, BGFX_SAMPLER_V_CLAMP, BGFX_SAMPLER_V_BORDER)
        | address(desc.address_w, BGFX_SAMPLER_W_MIRROR, BGFX_SAMPLER_W_CLAMP, BGFX_SAMPLER_W_BORDER);
    if (desc.maximum_anisotropy > 1) flags |= BGFX_SAMPLER_MIN_ANISOTROPIC | BGFX_SAMPLER_MAG_ANISOTROPIC;
    else {
        if (desc.min_filter == Filter::nearest) flags |= BGFX_SAMPLER_MIN_POINT;
        if (desc.mag_filter == Filter::nearest) flags |= BGFX_SAMPLER_MAG_POINT;
    }
    if (desc.mip_filter == Filter::nearest) flags |= BGFX_SAMPLER_MIP_POINT;
    return flags;
}

UInt64 blend_factor(BlendFactor factor)
{
    switch (factor) {
    case BlendFactor::zero: return BGFX_STATE_BLEND_ZERO;
    case BlendFactor::one: return BGFX_STATE_BLEND_ONE;
    case BlendFactor::src_color: return BGFX_STATE_BLEND_SRC_COLOR;
    case BlendFactor::inv_src_color: return BGFX_STATE_BLEND_INV_SRC_COLOR;
    case BlendFactor::src_alpha: return BGFX_STATE_BLEND_SRC_ALPHA;
    case BlendFactor::inv_src_alpha: return BGFX_STATE_BLEND_INV_SRC_ALPHA;
    case BlendFactor::dst_color: return BGFX_STATE_BLEND_DST_COLOR;
    case BlendFactor::inv_dst_color: return BGFX_STATE_BLEND_INV_DST_COLOR;
    case BlendFactor::dst_alpha: return BGFX_STATE_BLEND_DST_ALPHA;
    case BlendFactor::inv_dst_alpha: return BGFX_STATE_BLEND_INV_DST_ALPHA;
    case BlendFactor::src_alpha_saturate: return BGFX_STATE_BLEND_SRC_ALPHA_SAT;
    }
    return 0;
}

UInt64 blend_operation(BlendOp operation)
{
    switch (operation) {
    case BlendOp::add: return BGFX_STATE_BLEND_EQUATION_ADD;
    case BlendOp::subtract: return BGFX_STATE_BLEND_EQUATION_SUB;
    case BlendOp::reverse_subtract: return BGFX_STATE_BLEND_EQUATION_REVSUB;
    case BlendOp::minimum: return BGFX_STATE_BLEND_EQUATION_MIN;
    case BlendOp::maximum: return BGFX_STATE_BLEND_EQUATION_MAX;
    }
    return 0;
}

} // namespace

class BgfxGpuDevice::Impl {
public:
    struct BufferRecord { BufferDesc desc; std::vector<UInt8> shadow; std::vector<UInt8> written; };
    struct TextureRecord {
        struct Mip { std::vector<UInt8> bytes; UInt32 pitch = 0; };
        TextureDesc desc;
        bgfx::TextureHandle native = BGFX_INVALID_HANDLE;
        bool color_initialized = false;
        bool depth_initialized = false;
        bool stencil_initialized = false;
        std::vector<Mip> mips;
    };
    struct SamplerRecord { SamplerDesc desc; };
    struct ShaderRecord {
        struct Field { bgfx::UniformHandle handle = BGFX_INVALID_HANDLE; UInt32 binding = 0, offset = 0, size = 0, count = 0; };
        struct TextureField { bgfx::UniformHandle handle = BGFX_INVALID_HANDLE; UInt32 binding = 0, stage = 0; };
        ShaderStage stage{};
        UInt32 uniforms = 0, samplers = 0;
        bgfx::ShaderHandle native = BGFX_INVALID_HANDLE;
        std::vector<Field> fields;
        std::vector<TextureField> textures;
    };
    struct PipelineRecord { PipelineRecord() : key(PipelineDesc{}) {} PipelineKey key; bgfx::ProgramHandle native = BGFX_INVALID_HANDLE; };

    enum class NativeKind : UInt8 { program, shader, texture, vertex, index, framebuffer };
    struct NativeOwned {
        NativeKind kind = NativeKind::texture;
        UInt16 index = UINT16_MAX;
        bool candidate = false, retained = false;
        UInt32 captures = 0;
    };
    struct alignas(16) UniformWord { std::array<UInt8,16> bytes{}; };
    struct FramePacket {
        bgfx::BoundedSubmissionCommand command{};
        std::vector<bgfx::BoundedSubmissionUniform> uniforms;
        std::vector<bgfx::BoundedSubmissionTexture> textures;
        std::vector<UniformWord> payload;
        std::vector<std::size_t> leases;
    };
    static_assert(std::is_nothrow_move_constructible_v<FramePacket>);
    struct Checkpoint {
        std::vector<Slot<BufferRecord>> buffers;
        std::vector<Slot<TextureRecord>> textures;
        std::vector<Slot<SamplerRecord>> samplers;
        std::vector<Slot<ShaderRecord>> shaders;
        std::vector<Slot<PipelineRecord>> pipelines;
        std::vector<NativeOwned> native_owners;
        std::vector<FramePacket> journal;
        std::vector<bgfx::BoundedSubmissionCommand> manifest;
        UInt32 next_view = 0, window_width = 0, window_height = 0;
        UInt32 draws = 0, views = 0;
        bool completed = false;
    };
    struct OperationGuard {
        Impl& owner;
        int exceptions = std::uncaught_exceptions();
        ~OperationGuard() noexcept {
            if (std::uncaught_exceptions() > exceptions) owner.transaction.poison();
        }
    };

    static void destroy_native(NativeOwned value) noexcept
    {
        switch (value.kind) {
        case NativeKind::program: bgfx::destroy(bgfx::ProgramHandle{value.index}); break;
        case NativeKind::shader: bgfx::destroy(bgfx::ShaderHandle{value.index}); break;
        case NativeKind::texture: bgfx::destroy(bgfx::TextureHandle{value.index}); break;
        case NativeKind::vertex: bgfx::destroy(bgfx::VertexBufferHandle{value.index}); break;
        case NativeKind::index: bgfx::destroy(bgfx::IndexBufferHandle{value.index}); break;
        case NativeKind::framebuffer: bgfx::destroy(bgfx::FrameBufferHandle{value.index}); break;
        }
    }
    struct NativeCandidate {
        Impl& owner;
        NativeOwned owned;
        bool released = false;
        NativeCandidate(Impl& value, NativeOwned native) : owner(value), owned(native)
        { ++owner.native_reference_creates; }
        ~NativeCandidate() { if (!released) owner.destroy_reference(owned); }
    };
    void destroy_reference(NativeOwned value) noexcept
    { destroy_native(value); ++native_reference_destroys; }

    void drain_retirements() noexcept
    {
        // Programs before their shaders; borrowed FBO textures are never owned
        // here. Only ordinary boundaries drain, never transaction finish.
        for (auto kind : {NativeKind::framebuffer, NativeKind::vertex, NativeKind::index,
                          NativeKind::program, NativeKind::shader, NativeKind::texture})
            for (const auto value : retirements) if (value.kind == kind) {
                destroy_reference(value); ++retirement_destroys;
            }
        retirements.clear();
    }
    UInt32 advance_frame()
    { ++frame_advances; return bgfx::frame(); }
    bool admit_native_publication(std::string_view operation)
    {
        if (!transaction.active() || publication_fault == UINT32_MAX) return true;
        if (!publication_fault) {
            publication_fault = UINT32_MAX;
            fail(operation, "injected native candidate publication failure"); return false;
        }
        --publication_fault;
        return true;
    }
    void checkpoint_boundary()
    {
        if (checkpoint_copy_fault == UINT32_MAX) return;
        if (!checkpoint_copy_fault) {
            checkpoint_copy_fault = UINT32_MAX;
            throw std::bad_alloc();
        }
        --checkpoint_copy_fault;
    }
    void frame_copy_boundary()
    {
        if (frame_copy_fault == UINT32_MAX) return;
        if (!frame_copy_fault) { frame_copy_fault=UINT32_MAX; throw std::bad_alloc(); }
        --frame_copy_fault;
    }

    bool admit(std::string_view operation, UInt64 bytes = 0, UInt32 resources = 0)
    {
        if (!transaction.active()) { drain_retirements(); return true; }
        if (frame_transaction() && checkpoint->completed && operation.rfind("destroy_",0) != 0) {
            fail(operation, "only retirement metadata is allowed after final present"); return false;
        }
        if (!transaction.charge(bytes, resources)) { fail(operation, "transaction capacity or prior failure"); return false; }
        if (operation_fault != UINT32_MAX) {
            if (!operation_fault) {
                operation_fault = UINT32_MAX;
                fail(operation, "injected transaction operation failure"); return false;
            }
            --operation_fault;
        }
        return true;
    }
    bool reject_live(std::string_view operation)
    {
        if (!transaction.active()) { drain_retirements(); return false; }
        fail(operation, "idle transaction forbids frame or external completion operation");
        return true;
    }
    bool frame_transaction() const noexcept
    { return transaction.active() && transaction.token().mode == DeviceTransactionMode::frame_commands; }
    bool frame_operation(std::string_view operation)
    {
        if (!transaction.active()) { drain_retirements(); return true; }
        if (!frame_transaction()) { fail(operation,"idle transaction forbids frame commands"); return false; }
        return admit(operation);
    }
    bool reserve_packet(std::string_view operation, UInt32 packets = 1, UInt32 views = 0,
                        UInt64 bytes = 0, UInt32 resources = 0)
    {
        if (packets > transaction.descriptor().commands - checkpoint->journal.size()
            || next_view > RendererLimits::ordered_views
            || views > RendererLimits::ordered_views - next_view
            || !transaction.retain_views(views) || !transaction.retain_bytes(bytes)
            || !transaction.retain_resources(resources)) {
            fail(operation,"frame manifest capacity exceeded"); return false;
        }
        return true;
    }
    bool lease(FramePacket& packet, NativeKind kind, UInt16 index)
    {
        for (std::size_t i=0;i<checkpoint->native_owners.size();++i) {
            auto& unit=checkpoint->native_owners[i];
            if (unit.kind == kind && unit.index == index) {
                packet.leases.push_back(i);
                return true;
            }
        }
        fail("frame capture","native ownership unit is unavailable"); return false;
    }
    void append(FramePacket&& packet) noexcept
    {
        for (const auto i : packet.leases) ++checkpoint->native_owners[i].captures;
        if (packet.command.kind == bgfx::BoundedSubmissionCommand::Draw) ++checkpoint->draws;
        if (packet.command.kind == bgfx::BoundedSubmissionCommand::View) ++checkpoint->views;
        checkpoint->journal.push_back(std::move(packet)); // admission reserves capacity
    }
    void restore_frame(const Checkpoint& prior) noexcept
    {
        if (transaction.token().mode != DeviceTransactionMode::frame_commands) return;
        framebuffer=BGFX_INVALID_HANDLE; colors={}; color_count=0; depth={};
        width=height=0; target_generation=0; in_pass=false;
        next_view=prior.next_view; window_width=prior.window_width; window_height=prior.window_height;
    }
    ValidationResult capture_draw(const DrawDesc& desc, const PipelineRecord& pipeline,
        const ShaderRecord& vs, const ShaderRecord& fs, const BufferRecord& vertex,
        const BufferRecord* index, const bgfx::VertexLayout& layout, UInt32 stride)
    {
        FramePacket packet;
        const auto fields=vs.fields.size()+fs.fields.size();
        const auto samples=vs.textures.size()+fs.textures.size();
        UInt64 payload_bytes=0;
        for (const auto* shader : {&vs,&fs}) for (const auto& field : shader->fields)
            payload_bytes+=field.size;
        const UInt64 retained=payload_bytes+fields*sizeof(bgfx::BoundedSubmissionUniform)
            +samples*sizeof(bgfx::BoundedSubmissionTexture)+(samples+6)*sizeof(std::size_t)
            +vertex.shadow.size()+(index ? index->shadow.size() : 0);
        // All source/provider validation, alias comparison and byte copies
        // finish before candidates or checkpointed frame state are published.
        if (!reserve_packet("draw",1,0,retained,index ? 2 : 1)) return {false,last_error};
        frame_copy_boundary(); packet.uniforms.reserve(fields);
        frame_copy_boundary(); packet.textures.reserve(samples);
        frame_copy_boundary();
        packet.payload.reserve(static_cast<std::size_t>(payload_bytes/16));
        frame_copy_boundary();
        packet.leases.reserve(samples+6);
        std::vector<std::size_t> offsets;
        std::vector<SamplerHandle> sampler_owners;
        frame_copy_boundary(); offsets.reserve(fields);
        frame_copy_boundary(); sampler_owners.reserve(samples);
        const auto capture_stage=[&](const ShaderRecord& shader,const StageBindings& bindings) {
            for (const auto& field : shader.fields) {
                const auto& binding=bindings.uniforms[field.binding];
                const auto* buffer=lookup(buffers,binding.buffer);
                if (!field.count || field.count > UINT16_MAX || !field.size || field.size%16
                    || (UInt64(binding.offset)+field.offset)%16)
                    return fail("draw","reflected uniform alignment/count is not representable");
                const auto* bytes=buffer->record.shadow.data()+static_cast<std::size_t>(UInt64(binding.offset)+field.offset);
                std::size_t duplicate=packet.uniforms.size();
                for (std::size_t i=0;i<packet.uniforms.size();++i)
                    if (packet.uniforms[i].handle.idx == field.handle.idx) { duplicate=i; break; }
                if (duplicate != packet.uniforms.size()) {
                    const auto& prior=packet.uniforms[duplicate];
                    if (!detail::same_uniform_payload(prior.count,prior.bytes,packet.payload.data()+offsets[duplicate],
                        field.count,field.size,bytes))
                        return fail("draw","conflicting reflected uniform alias");
                    continue;
                }
                const auto offset=packet.payload.size();
                packet.payload.resize(offset+field.size/16);
                std::memcpy(packet.payload.data()+offset,bytes,field.size);
                bgfx::BoundedSubmissionUniform value;
                value.handle=field.handle; value.count=static_cast<UInt16>(field.count); value.bytes=field.size;
                packet.uniforms.push_back(value); offsets.push_back(offset);
            }
            for (const auto& field : shader.textures) {
                const auto* image=lookup(textures,bindings.textures[field.binding]);
                const auto owner=bindings.samplers[field.binding];
                const auto* sampler=lookup(samplers,owner);
                const auto flags=sampler_flags(sampler->record.desc);
                if (field.stage >= bgfx::getCaps()->limits.maxTextureSamplers || field.stage >= 32)
                    return fail("draw","reflected texture stage is not representable");
                bool duplicate=false;
                for (std::size_t i=0;i<packet.textures.size();++i) {
                    const auto& prior=packet.textures[i];
                    const auto alias=detail::sampler_alias(
                        {prior.stage,prior.sampler.idx,prior.texture.idx,sampler_owners[i].value(),prior.flags,prior.firstMip,prior.numMips},
                        {field.stage,field.handle.idx,image->record.native.idx,owner.value(),flags,0,image->record.desc.mip_levels});
                    if (alias == detail::BgfxSamplerAlias::conflict)
                        return fail("draw","conflicting shared source texture-stage alias");
                    if (alias == detail::BgfxSamplerAlias::identical) duplicate=true;
                }
                if (duplicate) continue;
                bgfx::BoundedSubmissionTexture value;
                value.sampler=field.handle; value.texture=image->record.native;
                value.flags=flags; value.stage=static_cast<UInt8>(field.stage);
                value.firstMip=0; value.numMips=static_cast<UInt8>(image->record.desc.mip_levels);
                packet.textures.push_back(value); sampler_owners.push_back(owner);
                if (!lease(packet,NativeKind::texture,image->record.native.idx)) return ValidationResult{false,last_error};
            }
            return ValidationResult{};
        };
        if (auto result=capture_stage(vs,desc.vertex_bindings); !result) return result;
        if (auto result=capture_stage(fs,desc.fragment_bindings); !result) return result;
        if (!lease(packet,NativeKind::program,pipeline.native.idx)
            || !lease(packet,NativeKind::shader,vs.native.idx)
            || !lease(packet,NativeKind::shader,fs.native.idx)
            || !lease(packet,NativeKind::framebuffer,framebuffer.idx)) return {false,last_error};
        if (vertex.shadow.size() > UINT32_MAX || (index && index->shadow.size() > UINT32_MAX)
            || packet.uniforms.size() > UINT16_MAX || packet.textures.size() > UINT16_MAX)
            return fail("draw","native geometry or manifest capacity exceeded");
        for (std::size_t i=0;i<packet.uniforms.size();++i)
            packet.uniforms[i].data=packet.payload.data()+offsets[i];
        auto native_vertex=bgfx::createVertexBuffer(bgfx::copy(vertex.shadow.data(),
            static_cast<UInt32>(vertex.shadow.size())),layout);
        if (!bgfx::isValid(native_vertex)) return fail("draw","native candidate vertices unavailable");
        NativeCandidate vertex_candidate{*this,{NativeKind::vertex,native_vertex.idx}};
        bgfx::IndexBufferHandle native_index=BGFX_INVALID_HANDLE;
        if (index) native_index=bgfx::createIndexBuffer(bgfx::copy(index->shadow.data(),
            static_cast<UInt32>(index->shadow.size())),
            desc.index_element_size == IndexElementSize::uint32 ? BGFX_BUFFER_INDEX32 : BGFX_BUFFER_NONE);
        if (index && !bgfx::isValid(native_index)) return fail("draw","native candidate indices unavailable");
        // The optional index guard cannot represent an invalid ownership unit.
        std::optional<NativeCandidate> index_candidate;
        if (index) index_candidate.emplace(*this,NativeOwned{NativeKind::index,native_index.idx});
        if (!admit_native_publication("draw")) return {false,last_error};
        const auto& state=pipeline.key.descriptor();
        auto& cmd=packet.command;
        cmd.kind=bgfx::BoundedSubmissionCommand::Draw; cmd.view=static_cast<bgfx::ViewId>(next_view-1);
        cmd.program=pipeline.native; cmd.vertex=native_vertex; cmd.index=native_index;
        cmd.firstVertex=static_cast<UInt32>(desc.base_vertex);
        cmd.vertices=index ? static_cast<UInt32>(vertex.shadow.size()/stride)-cmd.firstVertex : desc.vertex_or_index_count;
        cmd.firstIndex=index ? desc.first_index : 0; cmd.indices=index ? desc.vertex_or_index_count : 0;
        cmd.uniforms=packet.uniforms.data(); cmd.uniformCount=static_cast<UInt16>(packet.uniforms.size());
        cmd.textures=packet.textures.data(); cmd.textureCount=static_cast<UInt16>(packet.textures.size());
        if (state.blend.color_write_mask&1) cmd.state|=BGFX_STATE_WRITE_R;
        if (state.blend.color_write_mask&2) cmd.state|=BGFX_STATE_WRITE_G;
        if (state.blend.color_write_mask&4) cmd.state|=BGFX_STATE_WRITE_B;
        if (state.blend.color_write_mask&8) cmd.state|=BGFX_STATE_WRITE_A;
        if (state.depth_stencil.depth_test) cmd.state|=depth_compare_state(state.depth_stencil.depth_compare);
        if (state.depth_stencil.depth_write) cmd.state|=BGFX_STATE_WRITE_Z;
        if (state.raster.cull == CullMode::clockwise) cmd.state|=BGFX_STATE_CULL_CW;
        if (state.raster.cull == CullMode::counter_clockwise) cmd.state|=BGFX_STATE_CULL_CCW;
        if (state.topology == PrimitiveTopology::triangle_strip) cmd.state|=BGFX_STATE_PT_TRISTRIP;
        if (state.topology == PrimitiveTopology::point_list)
            cmd.state|=BGFX_STATE_PT_POINTS|BGFX_STATE_POINT_SIZE(static_cast<UInt32>(desc.point_size));
        if (state.blend.enabled) cmd.state|=BGFX_STATE_BLEND_FUNC_SEPARATE(
            blend_factor(state.blend.source_color),blend_factor(state.blend.destination_color),
            blend_factor(state.blend.source_alpha),blend_factor(state.blend.destination_alpha))
            |BGFX_STATE_BLEND_EQUATION_SEPARATE(blend_operation(state.blend.color_operation),blend_operation(state.blend.alpha_operation));
        cmd.depthBias=static_cast<Int32>(state.raster.depth_bias);
        if (state.depth_stencil.stencil_test) {
            const auto& stencil=state.depth_stencil;
            cmd.stencilFront=stencil_compare_state(stencil.stencil_compare)
                |BGFX_STENCIL_FUNC_REF(stencil.stencil_reference)|BGFX_STENCIL_FUNC_RMASK(stencil.stencil_read_mask)
                |(stencil_op_code(stencil.stencil_fail)<<BGFX_STENCIL_OP_FAIL_S_SHIFT)
                |(stencil_op_code(stencil.depth_fail)<<BGFX_STENCIL_OP_FAIL_Z_SHIFT)
                |(stencil_op_code(stencil.depth_pass)<<BGFX_STENCIL_OP_PASS_Z_SHIFT);
            cmd.stencilBack=(cmd.stencilFront&~BGFX_STENCIL_FUNC_RMASK_MASK)|BGFX_STENCIL_FUNC_RMASK(stencil.stencil_write_mask);
        }
        track_native(vertex_candidate.owned); vertex_candidate.released=true;
        if (index) { track_native(index_candidate->owned); index_candidate->released=true; }
        if (!lease(packet,NativeKind::vertex,native_vertex.idx)
            || (index && !lease(packet,NativeKind::index,native_index.idx))) return {false,last_error};
        append(std::move(packet));
        return {};
    }
    void track_native(NativeOwned value) noexcept
    {
        if (checkpoint) {
            value.candidate = true;
            checkpoint->native_owners.push_back(value); // admission reserved capacity
        }
    }
    bool retain_native_unit(NativeKind kind, UInt16 index) noexcept
    {
        for (auto& owner : checkpoint->native_owners) {
            if (!owner.retained && owner.kind == kind && owner.index == index) {
                owner.retained = true;
                return true;
            }
        }
        return false;
    }
    template<class Record>
    static void restore_slots(std::vector<Slot<Record>>& current,
                              std::vector<Slot<Record>>& baseline) noexcept
    {
        static_assert(std::is_nothrow_move_constructible_v<Slot<Record>>);
        const auto old_count = baseline.size();
        current.swap(baseline);
        for (std::size_t i = old_count; i < baseline.size(); ++i) {
            const auto generation = baseline[i].generation;
            Slot<Record> tombstone;
            tombstone.generation = generation < slot_mask ? generation + 1 : 0;
            current.push_back(std::move(tombstone)); // reserved at admission
        }
    }
    UInt64 slot_count() const noexcept
    { return buffers.size()+textures.size()+samplers.size()+shaders.size()+pipelines.size(); }
    UInt64 baseline_bytes() const noexcept
    {
        UInt64 bytes = buffers.size()*sizeof(Slot<BufferRecord>)
            + textures.size()*sizeof(Slot<TextureRecord>) + samplers.size()*sizeof(Slot<SamplerRecord>)
            + shaders.size()*sizeof(Slot<ShaderRecord>) + pipelines.size()*sizeof(Slot<PipelineRecord>);
        for (const auto& slot : buffers) bytes += slot.record.shadow.size()+slot.record.written.size();
        for (const auto& slot : textures) {
            bytes += slot.record.mips.size()*sizeof(TextureRecord::Mip);
            for (const auto& mip : slot.record.mips) bytes += mip.bytes.size();
        }
        for (const auto& slot : shaders)
            bytes += slot.record.fields.size()*sizeof(ShaderRecord::Field)
                + slot.record.textures.size()*sizeof(ShaderRecord::TextureField);
        return bytes;
    }

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
        if (const char* driver = SDL_GetCurrentVideoDriver(); driver && std::strcmp(driver, "wayland") == 0)
            init.platformData.type = bgfx::NativeWindowHandleType::Wayland;
        native_type = init.platformData.type;
        if (!bgfx::init(init)) {
            runtime_owned.store(false);
            throw std::runtime_error("public bgfx Vulkan initialization failed");
        }
    }

    ~Impl()
    {
        if (checkpoint) {
            for (const auto value : checkpoint->native_owners)
                if (value.candidate) retirements.push_back(value);
            restore_slots(buffers, checkpoint->buffers);
            restore_slots(textures, checkpoint->textures);
            restore_slots(samplers, checkpoint->samplers);
            restore_slots(shaders, checkpoint->shaders);
            restore_slots(pipelines, checkpoint->pipelines);
            restore_frame(*checkpoint);
            checkpoint.reset();
        }
        drain_retirements();
        if (bgfx::isValid(framebuffer)) bgfx::destroy(framebuffer);
        if (bgfx::isValid(window_framebuffer)) bgfx::destroy(window_framebuffer);
        if (bgfx::isValid(present_program)) bgfx::destroy(present_program);
        for (auto& slot : pipelines)
            if (slot.alive && bgfx::isValid(slot.record.native)) destroy_reference({NativeKind::program,slot.record.native.idx});
        for (auto& slot : shaders)
            if (slot.alive && bgfx::isValid(slot.record.native)) destroy_reference({NativeKind::shader,slot.record.native.idx});
        for (auto& slot : textures)
            if (slot.alive && bgfx::isValid(slot.record.native)) destroy_reference({NativeKind::texture,slot.record.native.idx});
        assert(native_reference_creates == native_reference_destroys);
        advance_frame();
        bgfx::shutdown();
        runtime_owned.store(false);
    }

    ValidationResult fail(std::string_view operation, std::string_view reason)
    {
        transaction.poison(); // diagnostics may allocate or throw
        if (transaction.active() && diagnostic_fault) {
            diagnostic_fault = false;
            throw std::bad_alloc();
        }
        last_error = std::string(operation) + ": " + std::string(reason);
        return {false, last_error};
    }

    bool query_window_pixels(SDL_Window* window, int* width, int* height) const
    {
        return options.pixel_extent_query
            ? options.pixel_extent_query(window, width, height)
            : SDL_GetWindowSizeInPixels(window, width, height);
    }

    BgfxOptions options;
    const UInt64 transaction_device = ++next_transaction_device;
    detail::BgfxTransactionState transaction;
    std::unique_ptr<Checkpoint> checkpoint;
    std::vector<NativeOwned> retirements;
    UInt32 operation_fault = UINT32_MAX;
    UInt32 publication_fault = UINT32_MAX;
    UInt32 checkpoint_copy_fault = UINT32_MAX;
    UInt32 frame_copy_fault = UINT32_MAX;
    UInt64 retirement_destroys = 0, frame_advances = 0;
    UInt64 native_reference_creates = 0, native_reference_destroys = 0;
    bool checkpoint_fault = false, diagnostic_fault = false;
    bool submission_fault = false;
    UInt64 bounded_submissions = 0;
    std::string last_error;
    std::vector<Slot<BufferRecord>> buffers;
    std::vector<Slot<TextureRecord>> textures;
    std::vector<Slot<SamplerRecord>> samplers;
    std::vector<Slot<ShaderRecord>> shaders;
    std::vector<Slot<PipelineRecord>> pipelines;
    bgfx::FrameBufferHandle framebuffer = BGFX_INVALID_HANDLE;
    std::array<TextureHandle, RendererLimits::color_targets> colors{};
    UInt32 color_count = 0;
    TextureHandle depth;
    UInt32 width = 0, height = 0;
    UInt64 target_generation = 0;
    UInt32 next_view = 0;
    bool in_pass = false;
    SDL_Window* window = nullptr;
    bgfx::FrameBufferHandle window_framebuffer = BGFX_INVALID_HANDLE;
    bgfx::ProgramHandle present_program = BGFX_INVALID_HANDLE;
    ShaderHandle present_vertex;
    ShaderHandle present_fragment;
    bgfx::UniformHandle present_viewport = BGFX_INVALID_HANDLE;
    bgfx::UniformHandle present_texture = BGFX_INVALID_HANDLE;
    UInt32 window_width = 0, window_height = 0;
    void* window_nwh = nullptr;
    void* window_ndt = nullptr;
    bgfx::NativeWindowHandleType::Enum native_type = bgfx::NativeWindowHandleType::Default;
};

BgfxGpuDevice::BgfxGpuDevice(BgfxOptions options) : impl_(std::make_unique<Impl>(std::move(options))) {}
BgfxGpuDevice::~BgfxGpuDevice() = default;

bool BgfxGpuDevice::supports_device_transactions(DeviceTransactionMode mode) const noexcept
{ return mode == DeviceTransactionMode::idle_preparation || mode == DeviceTransactionMode::frame_commands; }

ValidationResult BgfxGpuDevice::begin_device_transaction(const DeviceTransactionDesc& desc,
                                                       DeviceTransactionToken& token)
{
    // Reserving slot capacity is private, but all observable ownership remains
    // unchanged until every checkpoint allocation succeeds.
    const UInt64 reserved = UInt64(desc.resources) * 2 *
        (sizeof(Slot<Impl::BufferRecord>)+sizeof(Slot<Impl::TextureRecord>)+sizeof(Slot<Impl::SamplerRecord>)
         +sizeof(Slot<Impl::ShaderRecord>)+sizeof(Slot<Impl::PipelineRecord>)+sizeof(Impl::NativeOwned))
        + (desc.mode == DeviceTransactionMode::frame_commands ? UInt64(desc.commands)*
            (sizeof(Impl::FramePacket)+sizeof(bgfx::BoundedSubmissionCommand)) : 0);
    const auto baseline = impl_->baseline_bytes();
    if (impl_->transaction.active() || impl_->in_pass
        || baseline > device_transaction_byte_limit(desc)
        || !detail::BgfxTransactionState::valid(desc,
            impl_->slot_count()+impl_->retirements.size(), baseline*2+reserved))
        return {false,"transaction: invalid mode, baseline, capacity or overlapping owner"};
    if (impl_->checkpoint_fault) {
        impl_->checkpoint_fault = false;
        return {false,"transaction: injected checkpoint allocation failure"};
    }
    auto candidate = std::make_unique<Impl::Checkpoint>();
    candidate->next_view=impl_->next_view;
    candidate->window_width=impl_->window_width; candidate->window_height=impl_->window_height;
    if (desc.mode == DeviceTransactionMode::frame_commands) {
        candidate->journal.reserve(desc.commands); candidate->manifest.reserve(desc.commands);
        impl_->checkpoint_boundary();
    }
    impl_->checkpoint_boundary();
    candidate->buffers = impl_->buffers;
    impl_->checkpoint_boundary();
    candidate->textures = impl_->textures;
    impl_->checkpoint_boundary();
    candidate->samplers = impl_->samplers;
    impl_->checkpoint_boundary();
    candidate->shaders = impl_->shaders;
    impl_->checkpoint_boundary();
    candidate->pipelines = impl_->pipelines;
    impl_->checkpoint_boundary();
    const auto reserve = [&](auto& current, auto& prior) {
        current.reserve(current.size()+desc.resources);
        prior.reserve(prior.size()+desc.resources);
        impl_->checkpoint_boundary();
    };
    reserve(impl_->buffers, candidate->buffers);
    reserve(impl_->textures, candidate->textures);
    reserve(impl_->samplers, candidate->samplers);
    reserve(impl_->shaders, candidate->shaders);
    reserve(impl_->pipelines, candidate->pipelines);
    candidate->native_owners.reserve(desc.resources);
    const auto record_owners = [&](const auto& slots, auto kind) {
        for (const auto& slot : slots)
            if (slot.alive) candidate->native_owners.push_back({kind,slot.record.native.idx});
    };
    record_owners(candidate->pipelines, Impl::NativeKind::program);
    record_owners(candidate->shaders, Impl::NativeKind::shader);
    record_owners(candidate->textures, Impl::NativeKind::texture);
    impl_->retirements.reserve(desc.resources);
    impl_->checkpoint_boundary();
    if (!impl_->transaction.begin(desc, impl_->transaction_device,
            impl_->slot_count()+impl_->retirements.size(), baseline*2+reserved))
        return {false,"transaction: sequence exhausted"};
    impl_->checkpoint = std::move(candidate);
    token = impl_->transaction.token();
    return {};
}

bool BgfxGpuDevice::commit_device_transaction(const DeviceTransactionToken& token) noexcept
{
    if (!impl_->transaction.matches(token) || impl_->transaction.failed()) return false;
    auto& checkpoint = *impl_->checkpoint;
    for (auto& owner : checkpoint.native_owners) owner.retained = false;
    const auto keep_live = [&](const auto& slots, auto kind) {
        for (const auto& slot : slots) {
            if (!slot.alive) continue;
            if (!impl_->retain_native_unit(kind, slot.record.native.idx)) return false;
        }
        return true;
    };
    if (!keep_live(impl_->pipelines, Impl::NativeKind::program)
        || !keep_live(impl_->shaders, Impl::NativeKind::shader)
        || !keep_live(impl_->textures, Impl::NativeKind::texture)) {
        impl_->transaction.poison(); return false;
    }
    if (impl_->frame_transaction()) {
        if (!checkpoint.completed || impl_->in_pass) return false;
        if (impl_->submission_fault) { impl_->submission_fault=false; return false; }
        checkpoint.manifest.clear();
        for (const auto& packet : checkpoint.journal) checkpoint.manifest.push_back(packet.command);
        const auto& bounds=impl_->transaction.descriptor();
        bgfx::BoundedSubmissionDesc native;
        native.generation=token.generation; native.commands=bounds.commands;
        native.draws=checkpoint.draws; native.views=checkpoint.views;
        native.resources=bounds.resources; native.bytes=static_cast<UInt32>(bounds.bytes);
        bgfx::BoundedSubmissionReceipt receipt;
        if (!bgfx::submitBounded(native,checkpoint.manifest.data(),
            static_cast<UInt32>(checkpoint.manifest.size()),receipt)) return false;
        ++impl_->bounded_submissions; ++impl_->frame_advances;
    }
    if (!impl_->transaction.finish(token, true)) return false;
    for (const auto value : checkpoint.native_owners)
        if (!value.retained) impl_->retirements.push_back(value);
    impl_->checkpoint.reset();
    return true;
}

bool BgfxGpuDevice::abort_device_transaction(const DeviceTransactionToken& token) noexcept
{
    if (!impl_->transaction.finish(token, false)) return false;
    auto& checkpoint = *impl_->checkpoint;
    for (const auto value : checkpoint.native_owners)
        if (value.candidate) impl_->retirements.push_back(value);
    Impl::restore_slots(impl_->buffers, checkpoint.buffers);
    Impl::restore_slots(impl_->textures, checkpoint.textures);
    Impl::restore_slots(impl_->samplers, checkpoint.samplers);
    Impl::restore_slots(impl_->shaders, checkpoint.shaders);
    Impl::restore_slots(impl_->pipelines, checkpoint.pipelines);
    impl_->restore_frame(checkpoint);
    impl_->checkpoint.reset();
    return true;
}
void BgfxGpuDevice::fail_next_transaction_checkpoint() noexcept { impl_->checkpoint_fault = true; }
void BgfxGpuDevice::fail_transaction_checkpoint_copy_after(UInt32 count) noexcept { impl_->checkpoint_copy_fault = count; }
void BgfxGpuDevice::fail_transaction_operation_after(UInt32 count) noexcept { impl_->operation_fault = count; }
void BgfxGpuDevice::fail_next_transaction_diagnostic_allocation() noexcept { impl_->diagnostic_fault = true; }
void BgfxGpuDevice::fail_transaction_native_publication_after(UInt32 count) noexcept { impl_->publication_fault = count; }
std::size_t BgfxGpuDevice::pending_native_retirement_count() const noexcept { return impl_->retirements.size(); }
UInt64 BgfxGpuDevice::native_retirement_destroy_count() const noexcept { return impl_->retirement_destroys; }
UInt64 BgfxGpuDevice::native_frame_advance_count() const noexcept { return impl_->frame_advances; }
UInt64 BgfxGpuDevice::bounded_submission_count() const noexcept { return impl_->bounded_submissions; }
UInt64 BgfxGpuDevice::staged_frame_command_count() const noexcept
{ return impl_->checkpoint ? impl_->checkpoint->journal.size() : 0; }
void BgfxGpuDevice::fail_next_transaction_submission() noexcept { impl_->submission_fault=true; }
void BgfxGpuDevice::fail_transaction_frame_copy_after(UInt32 count) noexcept { impl_->frame_copy_fault=count; }
UInt64 BgfxGpuDevice::live_owned_native_reference_count() const noexcept
{ return impl_->native_reference_creates-impl_->native_reference_destroys; }

bool BgfxGpuDevice::supports_texture_format(TextureFormat format, TextureDimension dimension,
    bool sampled, bool render_target) const noexcept
{
    if (format == TextureFormat::bgr5a1 && render_target) return false;
    if (dimension != TextureDimension::texture_2d || (!sampled && !render_target)) return false;
    const auto native = physical_format(format);
    if (native == bgfx::TextureFormat::Unknown) return false;
    const UInt64 flags = render_target ? BGFX_TEXTURE_RT : BGFX_TEXTURE_NONE;
    return bgfx::isTextureValid(1, false, 1, native, flags);
}

BufferHandle BgfxGpuDevice::create_buffer(const BufferDesc& desc, std::string_view label)
{
    Impl::OperationGuard guard{*impl_};
    if (impl_->transaction.active() && desc.size > detail::BgfxTransactionState::maximum_bytes) {
        impl_->fail("create_buffer", "transaction buffer exceeds byte capacity"); return {};
    }
    if (!impl_->admit("create_buffer", desc.size*2+label.size(), 1)) return {};
    if (auto result = validate(desc); !result) { impl_->fail("create_buffer", result.error); return {}; }
    // Vertex layout and index width are supplied only by the later draw. Keep
    // a bounded upload shadow and materialize the correctly typed native buffer
    // at the draw boundary instead of guessing its layout here.
    auto handle = insert<BufferHandle>(impl_->buffers, {desc,
        std::vector<UInt8>(static_cast<std::size_t>(desc.size)),
        std::vector<UInt8>(static_cast<std::size_t>(desc.size))}, impl_->transaction.active());
    if (!handle) impl_->fail("create_buffer", "opaque resource slot budget exhausted");
    return handle;
}

TextureHandle BgfxGpuDevice::create_texture(const TextureDesc& desc, std::string_view label)
{
    Impl::OperationGuard guard{*impl_};
    if (!impl_->admit("create_texture", UInt64(desc.mip_levels)*sizeof(Impl::TextureRecord::Mip)+label.size(), 1)) return {};
    if (auto result = validate(desc); !result) { impl_->fail("create_texture", result.error); return {}; }
    if (desc.dimension != TextureDimension::texture_2d || desc.width > UINT16_MAX || desc.height > UINT16_MAX
        || desc.mip_levels > maximum_2d_mip_levels(desc.width, desc.height)
        || (desc.render_target && desc.mip_levels != 1)
        || !supports_texture_format(desc.format, desc.dimension, desc.sampled, desc.render_target)) {
        impl_->fail("create_texture", "unsupported public bgfx texture dimension, extent, mip or format/usage");
        return {};
    }
    const auto flags = desc.render_target ? BGFX_TEXTURE_RT : BGFX_TEXTURE_NONE;
    auto native = bgfx::createTexture2D(static_cast<UInt16>(desc.width), static_cast<UInt16>(desc.height),
        desc.mip_levels > 1, 1, physical_format(desc.format), flags);
    if (!bgfx::isValid(native)) { impl_->fail("create_texture", "public bgfx texture allocation failed"); return {}; }
    Impl::NativeCandidate candidate{*impl_, {Impl::NativeKind::texture, native.idx}};
    Impl::TextureRecord record;
    record.desc = desc; record.native = native; record.mips.resize(desc.mip_levels);
    if (!impl_->admit_native_publication("create_texture")) return {};
    auto handle = insert<TextureHandle>(impl_->textures, std::move(record), impl_->transaction.active());
    if (!handle) impl_->fail("create_texture", "opaque resource slot budget exhausted");
    else { impl_->track_native(candidate.owned); candidate.released = true; }
    return handle;
}

std::optional<TextureFormat> BgfxGpuDevice::describe_texture_format(TextureHandle handle) const noexcept
{
    const auto* slot=lookup(impl_->textures,handle);
    return slot ? std::optional<TextureFormat>(slot->record.desc.format) : std::nullopt;
}

SamplerHandle BgfxGpuDevice::create_sampler(const SamplerDesc& desc, std::string_view label)
{
    Impl::OperationGuard guard{*impl_};
    if (!impl_->admit("create_sampler", label.size(), 1)) return {};
    if (auto result = validate(desc); !result) { impl_->fail("create_sampler", result.error); return {}; }
    // bgfx encodes samplers as validated flags at the setTexture binding edge.
    auto handle = insert<SamplerHandle>(impl_->samplers, {desc}, impl_->transaction.active());
    if (!handle) impl_->fail("create_sampler", "opaque resource slot budget exhausted");
    return handle;
}

ValidationResult BgfxGpuDevice::upload(const UploadDesc& desc, const void* bytes)
{
    Impl::OperationGuard guard{*impl_};
    if (!impl_->admit("upload", desc.size)) return {false, impl_->last_error};
    if (auto result = validate(desc); !result) return impl_->fail("upload", result.error);
    auto* slot = lookup(impl_->buffers, desc.destination);
    if (!slot) return impl_->fail("upload", "stale or foreign buffer handle");
    if (!bytes || desc.destination_size != slot->record.desc.size)
        return impl_->fail("upload", "null bytes or mismatched destination size");
    std::memcpy(slot->record.shadow.data() + desc.offset, bytes, static_cast<std::size_t>(desc.size));
    std::fill(slot->record.written.begin() + static_cast<std::size_t>(desc.offset),
        slot->record.written.begin() + static_cast<std::size_t>(desc.offset + desc.size), 1);
    return {};
}

ValidationResult BgfxGpuDevice::upload_texture(const TextureUploadDesc& desc, const void* bytes)
{
    Impl::OperationGuard guard{*impl_};
    if (!impl_->admit("upload_texture", desc.size)) return {false, impl_->last_error};
    auto* slot = lookup(impl_->textures, desc.destination);
    if (!slot) return impl_->fail("upload_texture", "stale or foreign texture handle");
    const auto& texture = slot->record.desc;
    const UInt64 required = required_texture_bytes(desc, texture.format);
    const UInt32 mip_width = desc.mip_level < texture.mip_levels
        ? std::max(1U, texture.width >> desc.mip_level) : 0;
    const UInt32 mip_height = desc.mip_level < texture.mip_levels
        ? std::max(1U, texture.height >> desc.mip_level) : 0;
    if (!bytes || desc.mip_level >= texture.mip_levels || !desc.width || !desc.height
        || desc.width != mip_width || desc.height != mip_height
        || desc.row_pitch < static_cast<UInt64>(desc.width) *
            (texture.format == TextureFormat::bgr5a1 ? 2U : 4U) || desc.row_pitch > UINT16_MAX
        || !required || desc.size != required || desc.size > RendererLimits::maximum_upload_bytes)
        return impl_->fail("upload_texture", "unsupported or out-of-bounds texture upload");
    if (impl_->transaction.active() && texture.render_target)
        return impl_->fail("upload_texture", "transaction upload cannot reconstruct GPU-owned render-target pixels");
    Impl::TextureRecord::Mip mip;
    mip.bytes.assign(static_cast<const UInt8*>(bytes), static_cast<const UInt8*>(bytes)+desc.size);
    mip.pitch = desc.row_pitch;
    if (impl_->transaction.active()) {
        UInt64 cloned = 0;
        for (const auto& prior : slot->record.mips) cloned += prior.bytes.size();
        if (!impl_->transaction.retain_resources(1) || !impl_->transaction.retain_bytes(cloned+desc.size))
            return impl_->fail("upload_texture", "COW native version or byte capacity exhausted");
        auto native = bgfx::createTexture2D(static_cast<UInt16>(texture.width), static_cast<UInt16>(texture.height),
            texture.mip_levels > 1, 1, physical_format(texture.format), BGFX_TEXTURE_NONE);
        if (!bgfx::isValid(native)) return impl_->fail("upload_texture", "COW native texture allocation failed");
        Impl::NativeCandidate candidate{*impl_, {Impl::NativeKind::texture, native.idx}};
        for (UInt32 level = 0; level < texture.mip_levels; ++level) {
            const auto& source = level == desc.mip_level ? mip : slot->record.mips[level];
            if (source.bytes.empty()) continue;
            bgfx::updateTexture2D(native, 0, static_cast<UInt8>(level), 0, 0,
                static_cast<UInt16>(std::max(1U, texture.width >> level)),
                static_cast<UInt16>(std::max(1U, texture.height >> level)),
                bgfx::copy(source.bytes.data(), static_cast<UInt32>(source.bytes.size())),
                static_cast<UInt16>(source.pitch));
        }
        if (!impl_->admit_native_publication("upload_texture")) return {false, impl_->last_error};
        impl_->track_native(candidate.owned);
        slot->record.native = native;
        candidate.released = true;
    } else bgfx::updateTexture2D(slot->record.native, 0, static_cast<UInt8>(desc.mip_level), 0, 0,
        static_cast<UInt16>(desc.width), static_cast<UInt16>(desc.height),
        bgfx::copy(bytes, static_cast<UInt32>(desc.size)), static_cast<UInt16>(desc.row_pitch));
    slot->record.mips[desc.mip_level] = std::move(mip);
    slot->record.color_initialized = true;
    return {};
}

void BgfxGpuDevice::destroy(BufferHandle handle)
{
    Impl::OperationGuard guard{*impl_};
    if (!impl_->admit("destroy_buffer")) return;
    if (auto* slot = lookup(impl_->buffers, handle)) retire(*slot);
}
void BgfxGpuDevice::destroy(TextureHandle handle)
{
    Impl::OperationGuard guard{*impl_};
    if (!impl_->admit("destroy_texture")) return;
    if (auto* slot = lookup(impl_->textures, handle)) {
        if (impl_->in_pass && (handle == impl_->depth
            || std::find(impl_->colors.begin(), impl_->colors.begin() + impl_->color_count, handle)
                != impl_->colors.begin() + impl_->color_count)) {
            impl_->fail("destroy_texture", "active render attachment cannot be destroyed");
            return;
        }
        if (!impl_->transaction.active()) impl_->destroy_reference({Impl::NativeKind::texture,slot->record.native.idx});
        retire(*slot);
    }
}
void BgfxGpuDevice::destroy(SamplerHandle handle)
{
    Impl::OperationGuard guard{*impl_};
    if (!impl_->admit("destroy_sampler")) return;
    if (auto* slot = lookup(impl_->samplers, handle)) retire(*slot);
}

ShaderHandle BgfxGpuDevice::create_shader(const ShaderDesc& desc, std::string_view label)
{
    Impl::OperationGuard guard{*impl_};
    if (!impl_->admit("create_shader", label.size()+desc.name.size(), 1)) return {};
    if (auto result = validate(desc); !result) { impl_->fail("create_shader", result.error); return {}; }
    const std::filesystem::path relative(desc.name);
    if (label.empty() || relative.empty() || relative.is_absolute()) {
        impl_->fail("create_shader", "label and project-relative shader name are required"); return {};
    }
    for (const auto& part : relative)
        if (part == "..") { impl_->fail("create_shader", "shader path escapes owned root"); return {}; }
    const auto path = impl_->options.shader_root / (relative.string() + ".bin");
    std::ifstream input(path, std::ios::binary | std::ios::ate);
    if (!input || input.tellg() < 28 || input.tellg() > 4 * 1024 * 1024) {
        impl_->fail("create_shader", "missing, empty or oversized pinned bgfx shader binary"); return {};
    }
    std::vector<char> data(static_cast<std::size_t>(input.tellg()));
    if (!impl_->transaction.retain_bytes(data.size())) {
        impl_->fail("create_shader", "shader byte capacity exhausted"); return {};
    }
    input.seekg(0);
    input.read(data.data(), static_cast<std::streamsize>(data.size()));
    const char* expected = desc.stage == ShaderStage::vertex ? "VSH" : "FSH";
    if (!input || std::memcmp(data.data(), expected, 3) != 0 || static_cast<unsigned char>(data[3]) != 12) {
        impl_->fail("create_shader", "wrong bgfx shader stage/version envelope"); return {};
    }
    std::vector<UniformMetadata> metadata;
    std::vector<BgfxUniformBlockLayout> layout;
    if (auto result = decode_bgfx_uniform_layout(data, desc.stage, layout); !result) {
        impl_->fail("create_shader", result.error); return {};
    }
    if (!normalize_shader_uniform_identifiers(data, metadata)) {
        impl_->fail("create_shader", "invalid or colliding reflected shader uniform identifiers"); return {};
    }
    ShaderManifest manifest;
    if (!read_shader_manifest(impl_->options.shader_root / (relative.string() + ".json"), manifest)
        || manifest.stage != desc.stage) {
        impl_->fail("create_shader", "missing or invalid project-owned bgfx shader manifest"); return {};
    }
    if (!associate_bgfx_uniform_bindings(layout, manifest.block_binding)) {
        impl_->fail("create_shader", "compiled source uniform binding mismatch"); return {};
    }
    std::ifstream layout_input(impl_->options.shader_root / (relative.string() + ".layout"),
        std::ios::binary | std::ios::ate);
    if (!layout_input || layout_input.tellg() <= 0 || layout_input.tellg() > 4096) {
        impl_->fail("create_shader", "missing or oversized uniform layout metadata"); return {};
    }
    std::string sidecar(static_cast<std::size_t>(layout_input.tellg()), '\0');
    layout_input.seekg(0); layout_input.read(sidecar.data(), static_cast<std::streamsize>(sidecar.size()));
    if (!layout_input || !validate_bgfx_uniform_layout_sidecar(sidecar, desc.stage, layout)) {
        impl_->fail("create_shader", "stale or malformed uniform layout metadata"); return {};
    }
    std::set<UInt32> source_bindings;
    for (const auto& block : manifest.block_binding) {
        if (block.second >= desc.uniform_buffers || !source_bindings.emplace(block.second).second) {
            impl_->fail("create_shader", "duplicate or out-of-range source uniform binding"); return {};
        }
    }
    UInt32 previous_binding = 0;
    bool first_block = true;
    for (const auto& block : layout) {
        const auto binding = manifest.block_binding.find(block.instance);
        if (binding == manifest.block_binding.end() || (!first_block && binding->second <= previous_binding)) {
            impl_->fail("create_shader", "compiled source block/manifest binding mismatch"); return {};
        }
        first_block = false; previous_binding = binding->second;
    }
    for (const auto& item : metadata) {
        if (item.source_path.rfind("ZhStageUniforms.", 0) != 0) continue;
        bool admitted = false;
        for (const auto& block : manifest.block_binding)
            if (item.source_path.rfind("ZhStageUniforms." + block.first + ".", 0) == 0) admitted = true;
        if (!admitted) { impl_->fail("create_shader", "unknown reflected source block"); return {}; }
    }
    auto native = bgfx::createShader(bgfx::copy(data.data(), static_cast<UInt32>(data.size())));
    if (!bgfx::isValid(native)) { impl_->fail("create_shader", "public bgfx shader creation failed"); return {}; }
    Impl::NativeCandidate candidate{*impl_, {Impl::NativeKind::shader,native.idx}};
    const auto uniform_count = bgfx::getShaderUniforms(native);
    std::vector<bgfx::UniformHandle> uniforms(uniform_count);
    bgfx::getShaderUniforms(native, uniforms.data(), uniform_count);
    for (const auto uniform : uniforms) {
        if (!bgfx::isValid(uniform)) {
            impl_->fail("create_shader", "public bgfx rejected a reflected uniform identifier");
            return {};
        }
    }
    Impl::ShaderRecord record;
    record.stage = desc.stage; record.uniforms = desc.uniform_buffers;
    record.samplers = desc.samplers; record.native = native;
    std::map<std::string, bgfx::UniformHandle> by_name;
    for (const auto uniform : uniforms) {
        bgfx::UniformInfo info;
        bgfx::getUniformInfo(uniform, info);
        by_name.emplace(info.name, uniform);
    }
    for (const auto& item : metadata) {
        const auto found = by_name.find(item.name);
        if (found == by_name.end()) continue; // Optimized-out or non-public metadata.
        bgfx::UniformInfo info;
        bgfx::getUniformInfo(found->second, info);
        bool classified = false;
        for (const auto& block : layout) {
            const auto prefix = std::string("ZhStageUniforms.") + block.instance + ".";
            if (item.source_path.rfind(prefix, 0) != 0) continue;
            UInt32 size = 0;
            switch (info.type) {
            case bgfx::UniformType::Vec4: size = 16; break;
            case bgfx::UniformType::Mat3: size = 48; break;
            case bgfx::UniformType::Mat4: size = 64; break;
            default: break;
            }
            const auto binding = manifest.block_binding.at(block.instance);
            const auto field = std::find_if(block.fields.begin(), block.fields.end(), [&](const auto& candidate) {
                return item.source_path == prefix + candidate.name;
            });
            if (!size || field == block.fields.end() ||
                size != (field->kind == 2 ? 16U : field->kind == 3 ? 48U : 64U) || item.offset < block.origin ||
                item.offset != field->offset || info.num != field->count ||
                UInt64(item.offset) + size * info.num > UInt64(block.origin) + block.extent) {
                impl_->fail("create_shader", "reflected block type/binding mismatch"); return {};
            }
            record.fields.push_back({found->second, binding, item.offset - block.origin, size * info.num, info.num});
            classified = true;
            break;
        }
        if (classified) continue;
        if (layout.empty() && item.source_path.rfind("ZhStageUniforms.", 0) == 0)
            continue; // The actual compiler removed the entire algebraically dead UBO.
        for (const auto& texture : manifest.texture_binding) {
            if (item.name != texture.first + "_image") continue;
            if (info.type != bgfx::UniformType::Sampler || texture.second.first >= desc.samplers
                || texture.second.second >= RendererLimits::sampled_textures_per_stage) {
                impl_->fail("create_shader", "reflected texture type/binding mismatch"); return {};
            }
            record.textures.push_back({found->second, texture.second.first, texture.second.second});
            classified = true;
            break;
        }
        if (!classified) {
            impl_->fail("create_shader", "unclassified public bgfx uniform: " + item.name); return {};
        }
    }
    if (!impl_->transaction.retain_bytes(record.fields.size()*sizeof(Impl::ShaderRecord::Field)
        +record.textures.size()*sizeof(Impl::ShaderRecord::TextureField))) {
        impl_->fail("create_shader", "reflection byte capacity exhausted"); return {};
    }
    if (!impl_->admit_native_publication("create_shader")) return {};
    auto handle = insert<ShaderHandle>(impl_->shaders, std::move(record), impl_->transaction.active());
    if (!handle) impl_->fail("create_shader", "opaque shader slot budget exhausted");
    else {
        impl_->track_native({Impl::NativeKind::shader, native.idx});
        candidate.released = true;
    }
    return handle;
}

PipelineHandle BgfxGpuDevice::create_pipeline(const PipelineKey& key, std::string_view label)
{
    Impl::OperationGuard guard{*impl_};
    if (!impl_->admit("create_pipeline", label.size())) return {};
    const auto& desc = key.descriptor();
    if (auto result = validate(desc); !result) { impl_->fail("create_pipeline", result.error); return {}; }
    if (label.empty()) { impl_->fail("create_pipeline", "label must not be empty"); return {}; }
    for (std::size_t i = 0; i < impl_->pipelines.size(); ++i)
        if (impl_->pipelines[i].alive && impl_->pipelines[i].record.key == key)
            return encode<PipelineHandle>(i, impl_->pipelines[i].generation);
    if (!impl_->transaction.retain_resources(1)) {
        impl_->fail("create_pipeline", "transaction resource capacity exhausted"); return {};
    }
    auto* vertex = lookup(impl_->shaders, desc.vertex_shader);
    auto* fragment = lookup(impl_->shaders, desc.fragment_shader);
    if (!vertex || vertex->record.stage != ShaderStage::vertex
        || !fragment || fragment->record.stage != ShaderStage::fragment) {
        impl_->fail("create_pipeline", "stale or wrong-stage shader handles"); return {};
    }
    if (desc.topology == PrimitiveTopology::triangle_fan || desc.raster.fill == FillMode::point) {
        impl_->fail("create_pipeline", "unsupported primitive/fill state requires source-side expansion"); return {};
    }
    auto native = bgfx::createProgram(vertex->record.native, fragment->record.native, false);
    if (!bgfx::isValid(native)) { impl_->fail("create_pipeline", "public bgfx shader program creation failed"); return {}; }
    Impl::NativeCandidate candidate{*impl_, {Impl::NativeKind::program, native.idx}};
    Impl::PipelineRecord record;
    record.key = key; record.native = native;
    if (!impl_->admit_native_publication("create_pipeline")) return {};
    auto handle = insert<PipelineHandle>(impl_->pipelines, std::move(record), impl_->transaction.active());
    if (!handle) impl_->fail("create_pipeline", "opaque pipeline slot budget exhausted");
    else { impl_->track_native(candidate.owned); candidate.released = true; }
    return handle;
}
ValidationResult BgfxGpuDevice::begin_pass(const RenderPassDesc& desc, std::string_view)
{
    Impl::OperationGuard guard{*impl_};
    if (!impl_->frame_operation("begin_pass")) return {false, impl_->last_error};
    if (impl_->in_pass) return impl_->fail("begin_pass", "a pass is already active");
    if (auto result = validate(desc); !result) return impl_->fail("begin_pass", result.error);
    if (impl_->frame_transaction() && desc.target_generation != impl_->transaction.token().generation)
        return impl_->fail("begin_pass","target and transaction generations differ");
    if (desc.width > UINT16_MAX || desc.height > UINT16_MAX || impl_->next_view >= RendererLimits::ordered_views)
        return impl_->fail("begin_pass", "target extent or ordered view budget exceeded");
    std::array<bgfx::TextureHandle, RendererLimits::color_targets + 1> attachments{};
    for (UInt32 i = 0; i < desc.color_target_count; ++i) {
        auto* color = lookup(impl_->textures, desc.color_targets[i]);
        if (!color || !color->record.desc.render_target || is_depth(color->record.desc.format)
            || color->record.desc.width != desc.width || color->record.desc.height != desc.height)
            return impl_->fail("begin_pass", "invalid color attachment or extent");
        if (desc.color_load == AttachmentLoad::load && !color->record.color_initialized)
            return impl_->fail("begin_pass", "color LOAD requires prior completed write");
        attachments[i] = color->record.native;
    }
    auto* depth = lookup(impl_->textures, desc.depth_target);
    if (!depth || !depth->record.desc.render_target || !is_depth(depth->record.desc.format)
        || depth->record.desc.width != desc.width || depth->record.desc.height != desc.height)
        return impl_->fail("begin_pass", "invalid depth attachment or extent");
    if (desc.depth_load == AttachmentLoad::load && (!depth->record.depth_initialized
        || (depth->record.desc.format == TextureFormat::depth24_stencil8 && !depth->record.stencil_initialized)))
        return impl_->fail("begin_pass", "depth LOAD requires prior completed write");
    attachments[desc.color_target_count] = depth->record.native;
    Impl::FramePacket packet;
    if (impl_->frame_transaction()) {
        if (desc.width > bgfx::getCaps()->limits.maxTextureSize || desc.height > bgfx::getCaps()->limits.maxTextureSize
            || !impl_->reserve_packet("begin_pass",1,1,
                (desc.color_target_count+2)*sizeof(std::size_t),1))
            return impl_->fail("begin_pass","native extent or candidate admission failed");
        impl_->frame_copy_boundary(); packet.leases.reserve(desc.color_target_count+2);
        for (UInt32 i=0;i<=desc.color_target_count;++i)
            if (!impl_->lease(packet,Impl::NativeKind::texture,attachments[i].idx)) return {false,impl_->last_error};
    }
    auto framebuffer = bgfx::createFrameBuffer(static_cast<UInt8>(desc.color_target_count + 1), attachments.data(), false);
    if (!bgfx::isValid(framebuffer)) return impl_->fail("begin_pass", "public bgfx framebuffer creation failed");
    const auto view = static_cast<bgfx::ViewId>(impl_->next_view);
    const UInt16 flags = (desc.color_load == AttachmentLoad::clear ? BGFX_CLEAR_COLOR : 0)
        | (desc.depth_load == AttachmentLoad::clear ? BGFX_CLEAR_DEPTH | BGFX_CLEAR_STENCIL : 0);
    if (impl_->frame_transaction()) {
        Impl::NativeCandidate candidate{*impl_,{Impl::NativeKind::framebuffer,framebuffer.idx}};
        if (!impl_->admit_native_publication("begin_pass")) return {false,impl_->last_error};
        packet.command.kind=bgfx::BoundedSubmissionCommand::View;
        packet.command.view=view; packet.command.framebuffer=framebuffer;
        packet.command.width=static_cast<UInt16>(desc.width); packet.command.height=static_cast<UInt16>(desc.height);
        packet.command.clearFlags=flags; packet.command.clearRgba=clear_rgba(desc.clear_color);
        packet.command.clearDepth=desc.clear_depth; packet.command.touch=true;
        impl_->track_native(candidate.owned); candidate.released=true;
        if (!impl_->lease(packet,Impl::NativeKind::framebuffer,framebuffer.idx)) return {false,impl_->last_error};
        impl_->append(std::move(packet));
    } else {
        bgfx::setViewFrameBuffer(view, framebuffer);
        bgfx::setViewMode(view, bgfx::ViewMode::Sequential);
        bgfx::setViewRect(view, 0, 0, static_cast<UInt16>(desc.width), static_cast<UInt16>(desc.height));
        bgfx::setViewClear(view, flags, clear_rgba(desc.clear_color), desc.clear_depth, 0);
        bgfx::touch(view);
    }
    ++impl_->next_view;
    impl_->framebuffer = framebuffer;
    impl_->colors = desc.color_targets;
    impl_->color_count = desc.color_target_count;
    impl_->depth = desc.depth_target;
    impl_->width = desc.width;
    impl_->height = desc.height;
    impl_->target_generation = desc.target_generation;
    impl_->in_pass = true;
    if (desc.color_load == AttachmentLoad::clear)
        for (UInt32 i = 0; i < desc.color_target_count; ++i)
            lookup(impl_->textures, desc.color_targets[i])->record.color_initialized = true;
    if (desc.depth_load == AttachmentLoad::clear) {
        depth->record.depth_initialized = true;
        if (depth->record.desc.format == TextureFormat::depth24_stencil8)
            depth->record.stencil_initialized = true;
    }
    return {};
}

std::pair<UInt32,UInt32> BgfxGpuDevice::active_pass_extent() const noexcept
{ return impl_->in_pass ? std::pair<UInt32,UInt32>{impl_->width, impl_->height} : std::pair<UInt32,UInt32>{0,0}; }

ValidationResult BgfxGpuDevice::set_viewport(const ViewportDesc& desc)
{
    Impl::OperationGuard guard{*impl_};
    if (!impl_->frame_operation("set_viewport")) return {false, impl_->last_error};
    if (!impl_->in_pass) return impl_->fail("set_viewport", "no active pass");
    if (auto result = validate(desc, impl_->width, impl_->height); !result)
        return impl_->fail("set_viewport", result.error);
    if (std::trunc(desc.x) != desc.x || std::trunc(desc.y) != desc.y
        || std::trunc(desc.width) != desc.width || std::trunc(desc.height) != desc.height
        || desc.min_depth != 0.0f || desc.max_depth != 1.0f)
        return impl_->fail("set_viewport", "public bgfx view requires integer rectangle and full depth range");
    if (impl_->next_view >= RendererLimits::ordered_views)
        return impl_->fail("set_viewport", "ordered view budget exhausted");
    if (impl_->frame_transaction()) {
        if (!detail::bounded_view_rectangle(desc.x,desc.y,desc.width,desc.height))
            return impl_->fail("set_viewport","native signed view coordinate is not representable");
        if (!impl_->reserve_packet("set_viewport",1,1,sizeof(std::size_t))) return {false,impl_->last_error};
        Impl::FramePacket packet;
        impl_->frame_copy_boundary(); packet.leases.reserve(1);
        if (!impl_->lease(packet,Impl::NativeKind::framebuffer,impl_->framebuffer.idx)) return {false,impl_->last_error};
        packet.command.kind=bgfx::BoundedSubmissionCommand::View;
        packet.command.view=static_cast<bgfx::ViewId>(impl_->next_view);
        packet.command.framebuffer=impl_->framebuffer;
        packet.command.x=static_cast<std::int16_t>(desc.x); packet.command.y=static_cast<std::int16_t>(desc.y);
        packet.command.width=static_cast<UInt16>(desc.width); packet.command.height=static_cast<UInt16>(desc.height);
        impl_->append(std::move(packet)); ++impl_->next_view;
        return {};
    }
    // View state is frame-global, not an immediate command. Mutating the
    // previous view would retroactively clip its full-target clear/draws.
    const auto view = static_cast<bgfx::ViewId>(impl_->next_view++);
    bgfx::setViewFrameBuffer(view, impl_->framebuffer);
    bgfx::setViewMode(view, bgfx::ViewMode::Sequential);
    bgfx::setViewRect(view,
        static_cast<UInt16>(desc.x), static_cast<UInt16>(desc.y),
        static_cast<UInt16>(desc.width), static_cast<UInt16>(desc.height));
    bgfx::setViewClear(view, BGFX_CLEAR_NONE);
    return {};
}

ValidationResult BgfxGpuDevice::clear_viewport(const ViewportClearDesc& desc)
{
    Impl::OperationGuard guard{*impl_};
    if (!impl_->frame_operation("clear_viewport")) return {false, impl_->last_error};
    if (!impl_->in_pass) return impl_->fail("clear_viewport", "no active pass");
    if (auto result = validate(desc, impl_->width, impl_->height); !result)
        return impl_->fail("clear_viewport", result.error);
    if (desc.target_generation != impl_->target_generation
        || (desc.color && (impl_->color_count != 1 || desc.color_target != impl_->colors[0]))
        || ((desc.depth || desc.stencil) && desc.depth_target != impl_->depth))
        return impl_->fail("clear_viewport", "stale generation, attachment or unsupported MRT selection");
    if (desc.stencil && lookup(impl_->textures, impl_->depth)->record.desc.format != TextureFormat::depth24_stencil8)
        return impl_->fail("clear_viewport", "stencil clear requires D24S8");
    if (impl_->next_view >= RendererLimits::ordered_views)
        return impl_->fail("clear_viewport", "ordered view budget exhausted");
    const auto left = std::max<Int32>(0, desc.x);
    const auto top = std::max<Int32>(0, desc.y);
    const auto right = std::min<std::int64_t>(impl_->width, static_cast<std::int64_t>(desc.x) + desc.width);
    const auto bottom = std::min<std::int64_t>(impl_->height, static_cast<std::int64_t>(desc.y) + desc.height);
    const auto view = static_cast<bgfx::ViewId>(impl_->next_view);
    const UInt16 flags = (desc.color ? BGFX_CLEAR_COLOR : 0) | (desc.depth ? BGFX_CLEAR_DEPTH : 0)
        | (desc.stencil ? BGFX_CLEAR_STENCIL : 0);
    if (impl_->frame_transaction()) {
        if (!detail::bounded_view_rectangle(left,top,right-left,bottom-top))
            return impl_->fail("clear_viewport","native signed clear coordinate is not representable");
        if (!impl_->reserve_packet("clear_viewport",1,1,sizeof(std::size_t))) return {false,impl_->last_error};
        Impl::FramePacket packet;
        impl_->frame_copy_boundary(); packet.leases.reserve(1);
        if (!impl_->lease(packet,Impl::NativeKind::framebuffer,impl_->framebuffer.idx)) return {false,impl_->last_error};
        packet.command.kind=bgfx::BoundedSubmissionCommand::View; packet.command.view=view;
        packet.command.framebuffer=impl_->framebuffer;
        packet.command.x=static_cast<std::int16_t>(left); packet.command.y=static_cast<std::int16_t>(top);
        packet.command.width=static_cast<UInt16>(right-left); packet.command.height=static_cast<UInt16>(bottom-top);
        packet.command.clearFlags=flags; packet.command.clearRgba=clear_rgba(desc.color_value);
        packet.command.clearDepth=desc.depth_value; packet.command.clearStencil=desc.stencil_value;
        packet.command.touch=true; impl_->append(std::move(packet));
    } else {
        bgfx::setViewFrameBuffer(view, impl_->framebuffer);
        bgfx::setViewMode(view, bgfx::ViewMode::Sequential);
        bgfx::setViewRect(view, static_cast<UInt16>(left), static_cast<UInt16>(top),
            static_cast<UInt16>(right-left), static_cast<UInt16>(bottom-top));
        bgfx::setViewClear(view, flags, clear_rgba(desc.color_value), desc.depth_value, desc.stencil_value);
        bgfx::touch(view);
    }
    ++impl_->next_view;
    if (desc.color) lookup(impl_->textures, impl_->colors[0])->record.color_initialized = true;
    if (desc.depth) lookup(impl_->textures, impl_->depth)->record.depth_initialized = true;
    if (desc.stencil) lookup(impl_->textures, impl_->depth)->record.stencil_initialized = true;
    return {};
}
ValidationResult BgfxGpuDevice::draw(const DrawDesc& desc)
{
    Impl::OperationGuard guard{*impl_};
    if (!impl_->frame_operation("draw")) return {false, impl_->last_error};
    if (!impl_->in_pass) return impl_->fail("draw", "draw requires an active pass");
    auto* pipeline = lookup(impl_->pipelines, desc.pipeline);
    if (!pipeline) return impl_->fail("draw", "stale or foreign pipeline handle");
    const auto& state = pipeline->record.key.descriptor();
    if (auto result = validate(desc, state); !result) return impl_->fail("draw", result.error);
    if (impl_->color_count != 1 || !lookup(impl_->textures, impl_->colors[0])
        || lookup(impl_->textures, impl_->colors[0])->record.desc.format != state.color_format
        || !lookup(impl_->textures, impl_->depth)
        || lookup(impl_->textures, impl_->depth)->record.desc.format != state.depth_format)
        return impl_->fail("draw", "pipeline and active target formats differ");
    if (state.raster.fill != FillMode::solid ||
        (state.raster.depth_bias != 0.0f && state.raster.depth_bias != -8.0f)
        || state.topology == PrimitiveTopology::triangle_fan)
        return impl_->fail("draw", "required pipeline or binding state is not yet mapped to public bgfx");
    if (state.depth_stencil.stencil_test && state.depth_format != TextureFormat::depth24_stencil8)
        return impl_->fail("draw","stencil test requires D24S8 target");
    if (state.topology == PrimitiveTopology::point_list
        && (desc.point_size > 15.0f || std::floor(desc.point_size) != desc.point_size))
        return impl_->fail("draw", "public bgfx point size must be an integer in 1..15");
    auto* vertex_shader = lookup(impl_->shaders, state.vertex_shader);
    auto* fragment_shader = lookup(impl_->shaders, state.fragment_shader);
    if (!vertex_shader || !fragment_shader)
        return impl_->fail("draw", "pipeline shader was destroyed");
    const auto validate_stage = [&](const Impl::ShaderRecord& shader, const StageBindings& bindings) -> ValidationResult {
        if (bindings.uniform_count != shader.uniforms || bindings.texture_count != shader.samplers)
            return impl_->fail("draw", "stage binding counts differ from declared shader resources");
        for (const auto& field : shader.fields) {
            if (field.binding >= bindings.uniform_count)
                return impl_->fail("draw", "reflected uniform binding exceeds stage inputs");
            const auto& binding = bindings.uniforms[field.binding];
            auto* buffer = lookup(impl_->buffers, binding.buffer);
            if (!buffer || buffer->record.desc.usage != BufferUsage::uniform
                || field.offset > binding.size || field.size > binding.size - field.offset
                || !range_written(buffer->record.written, UInt64(binding.offset) + field.offset, field.size))
                return impl_->fail("draw", "missing, stale or uninitialized reflected uniform field");
        }
        for (const auto& texture : shader.textures) {
            if (texture.binding >= bindings.texture_count)
                return impl_->fail("draw", "reflected texture binding exceeds stage inputs");
            auto* image = lookup(impl_->textures, bindings.textures[texture.binding]);
            auto* sampler = lookup(impl_->samplers, bindings.samplers[texture.binding]);
            if (!image || !image->record.desc.sampled || !image->record.color_initialized || !sampler
                || !detail::sampled_mip_range(0,image->record.desc.mip_levels,image->record.desc.mip_levels)
                || (sampler->record.desc.maximum_lod != 1000.0f
                    && !(sampler->record.desc.maximum_lod == 0.0f
                        && image->record.desc.mip_levels == 1)))
                return impl_->fail("draw", "missing/stale texture, sampler or unsupported sampler LOD");
            // Unknown CPU backing stays unknown. Render targets instead become
            // ready through completed writes and are constrained to one mip.
            if (!image->record.desc.render_target) {
                if (image->record.mips.size() != image->record.desc.mip_levels)
                    return impl_->fail("draw", "sampled mip backing differs from declared range");
                for (const auto& mip : image->record.mips)
                    if (mip.bytes.empty())
                        return impl_->fail("draw", "sampled range contains an uninitialized mip");
            }
        }
        return {};
    };
    if (auto result = validate_stage(vertex_shader->record, desc.vertex_bindings); !result) return result;
    if (auto result = validate_stage(fragment_shader->record, desc.fragment_bindings); !result) return result;
    auto* vertex = lookup(impl_->buffers, desc.vertex_buffer);
    if (!vertex || vertex->record.desc.usage != BufferUsage::vertex)
        return impl_->fail("draw", "stale or wrong-usage vertex buffer");
    bgfx::VertexLayout layout;
    UInt32 stride = 0;
    if (!vertex_layout_for(state, layout, stride) || !stride || vertex->record.shadow.size() % stride)
        return impl_->fail("draw", "unsupported or misaligned vertex layout");
    UInt64 start_vertex = 0;
    if (desc.base_vertex < 0) return impl_->fail("draw", "negative base vertex is not representable by public bgfx binding");
    start_vertex = static_cast<UInt32>(desc.base_vertex);
    const auto available_vertices = vertex->record.shadow.size() / stride;
    auto* index = desc.index_buffer ? lookup(impl_->buffers, desc.index_buffer) : nullptr;
    if (desc.index_buffer && (!index || index->record.desc.usage != BufferUsage::index))
        return impl_->fail("draw", "stale or wrong-usage index buffer");
    if (index) {
        const UInt64 width = static_cast<UInt8>(desc.index_element_size);
        if (impl_->frame_transaction() && index->record.shadow.size()%width)
            return impl_->fail("draw","native index buffer element alignment mismatch");
        const UInt64 offset = static_cast<UInt64>(desc.first_index) * width;
        const UInt64 bytes = static_cast<UInt64>(desc.vertex_or_index_count) * width;
        if (!range_written(index->record.written, offset, bytes))
            return impl_->fail("draw", "indexed range is out of bounds or uninitialized");
        for (UInt64 position = offset; position < offset + bytes; position += width) {
            UInt32 value = 0;
            std::memcpy(&value, index->record.shadow.data() + position, static_cast<std::size_t>(width));
            if (start_vertex + value >= available_vertices
                || !range_written(vertex->record.written, (start_vertex + value) * stride, stride))
                return impl_->fail("draw", "indexed vertex exceeds initialized source buffer");
        }
    } else if (start_vertex || desc.vertex_or_index_count > available_vertices
        || !range_written(vertex->record.written, 0, static_cast<UInt64>(desc.vertex_or_index_count) * stride))
        return impl_->fail("draw", "non-indexed vertex range is out of bounds or uninitialized");
    if (impl_->frame_transaction())
        return impl_->capture_draw(desc,pipeline->record,vertex_shader->record,fragment_shader->record,
            vertex->record,index ? &index->record : nullptr,layout,stride);
    const auto native_vertex = bgfx::createVertexBuffer(
        bgfx::copy(vertex->record.shadow.data(), static_cast<UInt32>(vertex->record.shadow.size())), layout);
    if (!bgfx::isValid(native_vertex)) return impl_->fail("draw", "public bgfx vertex buffer creation failed");
    bgfx::IndexBufferHandle native_index = BGFX_INVALID_HANDLE;
    if (index) {
        const UInt16 flags = desc.index_element_size == IndexElementSize::uint32 ? BGFX_BUFFER_INDEX32 : BGFX_BUFFER_NONE;
        native_index = bgfx::createIndexBuffer(
            bgfx::copy(index->record.shadow.data(), static_cast<UInt32>(index->record.shadow.size())), flags);
        if (!bgfx::isValid(native_index)) {
            bgfx::destroy(native_vertex);
            return impl_->fail("draw", "public bgfx index buffer creation failed");
        }
    }
    bgfx::setVertexBuffer(0, native_vertex, static_cast<UInt32>(start_vertex), UINT32_MAX);
    if (index) bgfx::setIndexBuffer(native_index, desc.first_index, desc.vertex_or_index_count);
    const auto bind_stage = [&](const Impl::ShaderRecord& shader, const StageBindings& bindings) {
        for (const auto& field : shader.fields) {
            const auto& binding = bindings.uniforms[field.binding];
            const auto* buffer = lookup(impl_->buffers, binding.buffer);
            bgfx::setUniform(field.handle,
                buffer->record.shadow.data() + static_cast<std::size_t>(binding.offset + field.offset),
                static_cast<UInt16>(field.count));
        }
        for (const auto& texture : shader.textures) {
            const auto* image = lookup(impl_->textures, bindings.textures[texture.binding]);
            const auto* sampler = lookup(impl_->samplers, bindings.samplers[texture.binding]);
            bgfx::setTexture(static_cast<UInt8>(texture.stage), texture.handle, image->record.native,
                0,UINT16_MAX,0,static_cast<UInt8>(image->record.desc.mip_levels),sampler_flags(sampler->record.desc));
        }
    };
    bind_stage(vertex_shader->record, desc.vertex_bindings);
    bind_stage(fragment_shader->record, desc.fragment_bindings);
    UInt64 flags = 0;
    if (state.blend.color_write_mask & 0x1) flags |= BGFX_STATE_WRITE_R;
    if (state.blend.color_write_mask & 0x2) flags |= BGFX_STATE_WRITE_G;
    if (state.blend.color_write_mask & 0x4) flags |= BGFX_STATE_WRITE_B;
    if (state.blend.color_write_mask & 0x8) flags |= BGFX_STATE_WRITE_A;
    if (state.depth_stencil.depth_test) flags |= depth_compare_state(state.depth_stencil.depth_compare);
    if (state.depth_stencil.depth_write) flags |= BGFX_STATE_WRITE_Z;
    if (state.raster.cull == CullMode::clockwise) flags |= BGFX_STATE_CULL_CW;
    else if (state.raster.cull == CullMode::counter_clockwise) flags |= BGFX_STATE_CULL_CCW;
    if (state.topology == PrimitiveTopology::triangle_strip) flags |= BGFX_STATE_PT_TRISTRIP;
    if (state.topology == PrimitiveTopology::point_list)
        flags |= BGFX_STATE_PT_POINTS | BGFX_STATE_POINT_SIZE(static_cast<UInt32>(desc.point_size));
    if (state.blend.enabled) {
        flags |= BGFX_STATE_BLEND_FUNC_SEPARATE(
            blend_factor(state.blend.source_color), blend_factor(state.blend.destination_color),
            blend_factor(state.blend.source_alpha), blend_factor(state.blend.destination_alpha));
        flags |= BGFX_STATE_BLEND_EQUATION_SEPARATE(
            blend_operation(state.blend.color_operation), blend_operation(state.blend.alpha_operation));
    }
    bgfx::setState(flags);
    // Pinned public bgfx provides per-draw depth control.  Reset it on every
    // draw so a source decal's D3D8 ZBIAS=8 cannot affect a later mesh.
    bgfx::setDepthControl(static_cast<Int32>(state.raster.depth_bias), 0.0f);
    if (state.depth_stencil.stencil_test) {
        if (state.depth_format != TextureFormat::depth24_stencil8) {
            if (index) bgfx::destroy(native_index);
            bgfx::destroy(native_vertex);
            return impl_->fail("draw", "stencil test requires D24S8 target");
        }
        const auto& stencil = state.depth_stencil;
        const UInt32 front = stencil_compare_state(stencil.stencil_compare)
            | BGFX_STENCIL_FUNC_REF(stencil.stencil_reference)
            | BGFX_STENCIL_FUNC_RMASK(stencil.stencil_read_mask)
            | (stencil_op_code(stencil.stencil_fail) << BGFX_STENCIL_OP_FAIL_S_SHIFT)
            | (stencil_op_code(stencil.depth_fail) << BGFX_STENCIL_OP_FAIL_Z_SHIFT)
            | (stencil_op_code(stencil.depth_pass) << BGFX_STENCIL_OP_PASS_Z_SHIFT);
        // The public second stencil word supplies the write mask to pinned
        // bgfx's Vulkan path while retaining the same front/back operations.
        const UInt32 back = (front & ~BGFX_STENCIL_FUNC_RMASK_MASK)
            | BGFX_STENCIL_FUNC_RMASK(stencil.stencil_write_mask);
        bgfx::setStencil(front, back);
    } else {
        bgfx::setStencil(BGFX_STENCIL_NONE);
    }
    bgfx::submit(static_cast<bgfx::ViewId>(impl_->next_view - 1), pipeline->record.native);
    if (index) bgfx::destroy(native_index);
    bgfx::destroy(native_vertex);
    return {};
}
ValidationResult BgfxGpuDevice::end_pass()
{
    Impl::OperationGuard guard{*impl_};
    if (!impl_->frame_operation("end_pass")) return {false, impl_->last_error};
    if (!impl_->in_pass) return impl_->fail("end_pass", "no active pass");
    if (!impl_->frame_transaction()) bgfx::destroy(impl_->framebuffer);
    impl_->framebuffer = BGFX_INVALID_HANDLE;
    impl_->in_pass = false;
    impl_->color_count = 0;
    impl_->colors = {};
    impl_->depth = {};
    impl_->width = impl_->height = 0;
    impl_->target_generation = 0;
    return {};
}
ValidationResult BgfxGpuDevice::present(TextureHandle source)
{
    Impl::OperationGuard guard{*impl_};
    if (!impl_->frame_operation("present")) return {false, impl_->last_error};
    if (impl_->in_pass) return impl_->fail("present", "cannot present during an active pass");
    if (!impl_->window || !bgfx::isValid(impl_->window_framebuffer))
        return impl_->fail("present", "no SDL3 window is claimed");
    auto* image = lookup(impl_->textures, source);
    if (!image || !image->record.desc.render_target || !image->record.desc.sampled
        || !image->record.color_initialized || is_depth(image->record.desc.format))
        return impl_->fail("present", "stale, uninitialized or unsampleable color source");
    int width = 0, height = 0;
    if (!impl_->query_window_pixels(impl_->window, &width, &height) || width < 0 || height < 0)
        return impl_->fail("present", "SDL3 window pixel size is unavailable");
    if (impl_->frame_transaction()) {
        if (width > UINT16_MAX || height > UINT16_MAX
            || width > bgfx::getCaps()->limits.maxTextureSize || height > bgfx::getCaps()->limits.maxTextureSize)
            return impl_->fail("present","window extent exceeds native bounds");
        const bool suspended=width == 0 || height == 0;
        const bool resize=!suspended && (impl_->window_width != static_cast<UInt32>(width)
            || impl_->window_height != static_cast<UInt32>(height));
        if (!bgfx::isValid(impl_->present_program) || !bgfx::isValid(impl_->present_viewport)
            || !bgfx::isValid(impl_->present_texture)
            || !lookup(impl_->shaders,impl_->present_vertex) || !lookup(impl_->shaders,impl_->present_fragment))
            return impl_->fail("present","presentation provider ownership is unavailable");
        if (!impl_->reserve_packet("present",suspended ? 1 : (resize ? 4 : 3),suspended ? 0 : 1,
            suspended ? 0 : 16+sizeof(bgfx::BoundedSubmissionUniform)+sizeof(bgfx::BoundedSubmissionTexture)
                +4*sizeof(std::size_t)+60,suspended ? 0 : 1)) return {false,impl_->last_error};
        std::array<Impl::FramePacket,4> packets;
        std::size_t count=0;
        if (resize) {
            auto& cmd=packets[count++].command;
            cmd.kind=bgfx::BoundedSubmissionCommand::ResizeSwapChain; cmd.framebuffer=impl_->window_framebuffer;
            cmd.swapChain.nwh=impl_->window_nwh; cmd.swapChain.ndt=impl_->window_ndt;
            cmd.swapChain.width=static_cast<UInt32>(width); cmd.swapChain.height=static_cast<UInt32>(height);
            cmd.swapChain.formatColor=bgfx::TextureFormat::BGRA8;
            cmd.swapChain.formatDepthStencil=bgfx::TextureFormat::Count;
        }
        if (!suspended) {
            auto& view=packets[count++].command;
            view.kind=bgfx::BoundedSubmissionCommand::View; view.view=static_cast<bgfx::ViewId>(impl_->next_view);
            view.framebuffer=impl_->window_framebuffer; view.width=static_cast<UInt16>(width);
            view.height=static_cast<UInt16>(height); view.clearFlags=BGFX_CLEAR_COLOR; view.clearRgba=0x000000ff;
            auto& packet=packets[count++];
            impl_->frame_copy_boundary(); packet.payload.resize(1);
            impl_->frame_copy_boundary(); packet.uniforms.resize(1);
            impl_->frame_copy_boundary(); packet.textures.resize(1);
            impl_->frame_copy_boundary(); packet.leases.reserve(4);
            const std::array<float,4> viewport{static_cast<float>(width),static_cast<float>(height),0,0};
            std::memcpy(packet.payload.data(),viewport.data(),sizeof(viewport));
            packet.uniforms[0].handle=impl_->present_viewport; packet.uniforms[0].count=1;
            packet.uniforms[0].bytes=sizeof(viewport); packet.uniforms[0].data=packet.payload.data();
            packet.textures[0].sampler=impl_->present_texture; packet.textures[0].texture=image->record.native;
            packet.textures[0].stage=8; packet.textures[0].flags=BGFX_SAMPLER_U_CLAMP|BGFX_SAMPLER_V_CLAMP;
            if (!impl_->lease(packet,Impl::NativeKind::texture,image->record.native.idx)
                || !impl_->lease(packet,Impl::NativeKind::shader,lookup(impl_->shaders,impl_->present_vertex)->record.native.idx)
                || !impl_->lease(packet,Impl::NativeKind::shader,lookup(impl_->shaders,impl_->present_fragment)->record.native.idx))
                return {false,impl_->last_error};
            bgfx::VertexLayout layout;
            layout.begin().add(bgfx::Attrib::Position,2,bgfx::AttribType::Float)
                .add(bgfx::Attrib::Color0,4,bgfx::AttribType::Uint8,true)
                .add(bgfx::Attrib::TexCoord0,2,bgfx::AttribType::Float).end();
            struct Vertex { float x,y; UInt32 color; float u,v; };
            const std::array<Vertex,3> triangle{{{0,0,0xffffffffU,0,0},
                {2.0f*width,0,0xffffffffU,2,0},{0,2.0f*height,0xffffffffU,0,2}}};
            const auto native=bgfx::createVertexBuffer(bgfx::copy(triangle.data(),sizeof(triangle)),layout);
            if (!bgfx::isValid(native)) return impl_->fail("present","typed presentation vertices unavailable");
            Impl::NativeCandidate candidate{*impl_,{Impl::NativeKind::vertex,native.idx}};
            if (!impl_->admit_native_publication("present")) return {false,impl_->last_error};
            auto& draw=packet.command;
            draw.kind=bgfx::BoundedSubmissionCommand::Draw; draw.view=static_cast<bgfx::ViewId>(impl_->next_view);
            draw.program=impl_->present_program; draw.vertex=native; draw.vertices=3;
            draw.uniforms=packet.uniforms.data(); draw.uniformCount=1;
            draw.textures=packet.textures.data(); draw.textureCount=1;
            draw.state=BGFX_STATE_WRITE_RGB|BGFX_STATE_WRITE_A;
            impl_->track_native(candidate.owned); candidate.released=true;
            if (!impl_->lease(packet,Impl::NativeKind::vertex,native.idx)) return {false,impl_->last_error};
        }
        packets[count++].command.kind=bgfx::BoundedSubmissionCommand::CompleteFrame;
        // Window-owned FBO/program identities are borrowed under the exclusive
        // device/window lifetime; claim/release/shutdown cannot intervene.
        for (std::size_t i=0;i<count;++i) impl_->append(std::move(packets[i]));
        impl_->checkpoint->completed=true;
        if (!suspended) { impl_->window_width=static_cast<UInt32>(width); impl_->window_height=static_cast<UInt32>(height); }
        impl_->next_view=0;
        return {};
    }
    if (width == 0 || height == 0) {
        impl_->advance_frame();
        impl_->next_view = 0;
        return {}; // Suspended/minimized surface; source resources remain live.
    }
    if (width > UINT16_MAX || height > UINT16_MAX || impl_->next_view >= RendererLimits::ordered_views)
        return impl_->fail("present", "window extent or ordered view budget exceeded");
    if (impl_->window_width != static_cast<UInt32>(width)
        || impl_->window_height != static_cast<UInt32>(height)) {
        bgfx::SwapChain swapchain;
        swapchain.nwh = impl_->window_nwh;
        swapchain.ndt = impl_->window_ndt;
        swapchain.width = static_cast<UInt32>(width);
        swapchain.height = static_cast<UInt32>(height);
        swapchain.formatColor = bgfx::TextureFormat::BGRA8;
        swapchain.formatDepthStencil = bgfx::TextureFormat::Count;
        bgfx::updateSwapChain(impl_->window_framebuffer, swapchain);
        impl_->window_width = static_cast<UInt32>(width);
        impl_->window_height = static_cast<UInt32>(height);
    }
    bgfx::VertexLayout layout;
    layout.begin().add(bgfx::Attrib::Position, 2, bgfx::AttribType::Float)
        .add(bgfx::Attrib::Color0, 4, bgfx::AttribType::Uint8, true)
        .add(bgfx::Attrib::TexCoord0, 2, bgfx::AttribType::Float).end();
    if (layout.getStride() != 20 || !bgfx::getAvailTransientVertexBuffer(3, layout))
        return impl_->fail("present", "public bgfx transient presentation vertices unavailable");
    bgfx::TransientVertexBuffer vertices;
    bgfx::allocTransientVertexBuffer(&vertices, 3, layout);
    struct Vertex { float x, y; UInt32 color; float u, v; };
    const std::array<Vertex,3> triangle{{
        {0,0,0xffffffffU,0,0},
        {2.0f * width,0,0xffffffffU,2,0},
        {0,2.0f * height,0xffffffffU,0,2},
    }};
    std::memcpy(vertices.data, triangle.data(), sizeof(triangle));
    const auto view = static_cast<bgfx::ViewId>(impl_->next_view++);
    bgfx::setViewFrameBuffer(view, impl_->window_framebuffer);
    bgfx::setViewMode(view, bgfx::ViewMode::Sequential);
    bgfx::setViewRect(view, 0, 0, static_cast<UInt16>(width), static_cast<UInt16>(height));
    bgfx::setViewClear(view, BGFX_CLEAR_COLOR, 0x000000ff);
    bgfx::setVertexBuffer(0, &vertices);
    const std::array<float,4> viewport{static_cast<float>(width),static_cast<float>(height),0,0};
    bgfx::setUniform(impl_->present_viewport, viewport.data());
    bgfx::setTexture(8, impl_->present_texture, image->record.native,
        BGFX_SAMPLER_U_CLAMP | BGFX_SAMPLER_V_CLAMP);
    bgfx::setState(BGFX_STATE_WRITE_RGB | BGFX_STATE_WRITE_A);
    bgfx::submit(view, impl_->present_program);
    impl_->advance_frame();
    impl_->next_view = 0;
    return {};
}
void BgfxGpuDevice::destroy(ShaderHandle handle)
{
    Impl::OperationGuard guard{*impl_};
    if (!impl_->admit("destroy_shader")) return;
    auto* slot = lookup(impl_->shaders, handle);
    if (!slot) return;
    for (const auto& pipeline : impl_->pipelines)
        if (pipeline.alive && (pipeline.record.key.descriptor().vertex_shader == handle
            || pipeline.record.key.descriptor().fragment_shader == handle)) {
            impl_->fail("destroy_shader", "shader is referenced by a live pipeline"); return;
        }
    if (!impl_->transaction.active()) impl_->destroy_reference({Impl::NativeKind::shader,slot->record.native.idx});
    retire(*slot);
}
void BgfxGpuDevice::destroy(PipelineHandle handle)
{
    Impl::OperationGuard guard{*impl_};
    if (!impl_->admit("destroy_pipeline")) return;
    if (auto* slot = lookup(impl_->pipelines, handle)) {
        if (!impl_->transaction.active()) impl_->destroy_reference({Impl::NativeKind::program,slot->record.native.idx});
        retire(*slot);
    }
}
const std::string& BgfxGpuDevice::last_error() const noexcept { return impl_->last_error; }
bool BgfxGpuDevice::pass_active() const noexcept { return impl_->in_pass; }
void BgfxGpuDevice::record_marker(std::string_view label)
{
    Impl::OperationGuard guard{*impl_};
    (void)impl_->admit("record_marker", label.size());
}
ValidationResult BgfxGpuDevice::claim_window(SDL_Window* window)
{
    Impl::OperationGuard guard{*impl_};
    if (impl_->reject_live("claim_window")) return {false, impl_->last_error};
    if (!window) return impl_->fail("claim_window", "SDL3 window is null");
    if (impl_->window) return impl_->fail("claim_window", "an SDL3 window is already claimed");
    if (impl_->in_pass) return impl_->fail("claim_window", "cannot claim during an active pass");
    if (!(bgfx::getCaps()->supported & BGFX_CAPS_SWAP_CHAIN))
        return impl_->fail("claim_window", "public bgfx Vulkan swap chains are unavailable");
    const auto properties = SDL_GetWindowProperties(window);
    if (!properties) return impl_->fail("claim_window", "invalid SDL3 window properties");
    void* nwh = nullptr;
    void* ndt = nullptr;
    bgfx::NativeWindowHandleType::Enum type = bgfx::NativeWindowHandleType::Default;
    const char* driver = SDL_GetCurrentVideoDriver();
    if (driver && std::strcmp(driver, "wayland") == 0) {
        nwh = SDL_GetPointerProperty(properties, SDL_PROP_WINDOW_WAYLAND_SURFACE_POINTER, nullptr);
        ndt = SDL_GetPointerProperty(properties, SDL_PROP_WINDOW_WAYLAND_DISPLAY_POINTER, nullptr);
        type = bgfx::NativeWindowHandleType::Wayland;
    } else if (driver && std::strcmp(driver, "x11") == 0) {
        nwh = reinterpret_cast<void*>(static_cast<std::uintptr_t>(
            SDL_GetNumberProperty(properties, SDL_PROP_WINDOW_X11_WINDOW_NUMBER, 0)));
        ndt = SDL_GetPointerProperty(properties, SDL_PROP_WINDOW_X11_DISPLAY_POINTER, nullptr);
    }
    if (!nwh || !ndt || type != impl_->native_type)
        return impl_->fail("claim_window", "SDL3 native window/display or video driver is unsupported");
    int width = 0, height = 0;
    if (!impl_->query_window_pixels(window, &width, &height) || width <= 0 || height <= 0
        || width > UINT16_MAX || height > UINT16_MAX)
        return impl_->fail("claim_window", "SDL3 window has unsupported pixel extent");
    auto vertex = create_shader({ShaderStage::vertex,"renderer/video.vert",1,0}, "presentation vertex");
    auto fragment = create_shader({ShaderStage::fragment,"renderer/video.frag",0,1}, "presentation fragment");
    if (!vertex || !fragment) {
        destroy(vertex); destroy(fragment);
        return impl_->fail("claim_window", "presentation shaders are unavailable");
    }
    auto* vertex_slot = lookup(impl_->shaders, vertex);
    auto* fragment_slot = lookup(impl_->shaders, fragment);
    if (vertex_slot->record.fields.size() != 1 || fragment_slot->record.textures.size() != 1) {
        destroy(vertex); destroy(fragment);
        return impl_->fail("claim_window", "presentation shader reflection differs");
    }
    auto program = bgfx::createProgram(vertex_slot->record.native, fragment_slot->record.native, false);
    if (!bgfx::isValid(program)) {
        destroy(vertex); destroy(fragment);
        return impl_->fail("claim_window", "public bgfx presentation program failed");
    }
    bgfx::SwapChain swapchain;
    swapchain.nwh = nwh; swapchain.ndt = ndt;
    swapchain.width = static_cast<UInt32>(width);
    swapchain.height = static_cast<UInt32>(height);
    swapchain.formatColor = bgfx::TextureFormat::BGRA8;
    swapchain.formatDepthStencil = bgfx::TextureFormat::Count;
    auto framebuffer = bgfx::createFrameBuffer(swapchain);
    if (!bgfx::isValid(framebuffer)) {
        bgfx::destroy(program); destroy(vertex); destroy(fragment);
        return impl_->fail("claim_window", "public bgfx swap-chain framebuffer failed");
    }
    impl_->window = window;
    impl_->window_framebuffer = framebuffer;
    impl_->present_program = program;
    impl_->present_vertex = vertex;
    impl_->present_fragment = fragment;
    impl_->present_viewport = vertex_slot->record.fields.front().handle;
    impl_->present_texture = fragment_slot->record.textures.front().handle;
    impl_->window_width = static_cast<UInt32>(width);
    impl_->window_height = static_cast<UInt32>(height);
    impl_->window_nwh = nwh; impl_->window_ndt = ndt;
    return {};
}
ValidationResult BgfxGpuDevice::wait_idle()
{
    Impl::OperationGuard guard{*impl_};
    if (impl_->reject_live("wait_idle")) return {false, impl_->last_error};
    if (impl_->in_pass) return impl_->fail("wait_idle", "pass remains active");
    impl_->advance_frame();
    impl_->next_view = 0;
    return {};
}

std::size_t BgfxGpuDevice::live_resource_count() const noexcept
{
    const auto live=[](const auto& slots) {
        return static_cast<std::size_t>(std::count_if(slots.begin(),slots.end(),
            [](const auto& slot) { return slot.alive; }));
    };
    return live(impl_->buffers)+live(impl_->textures)+live(impl_->samplers)+
        live(impl_->shaders)+live(impl_->pipelines);
}

std::vector<UInt8> BgfxGpuDevice::readback_rgba(TextureHandle source)
{
    Impl::OperationGuard guard{*impl_};
    if (impl_->reject_live("readback_rgba")) return {};
    if (impl_->in_pass) { impl_->fail("readback_rgba", "pass remains active"); return {}; }
    auto* slot = lookup(impl_->textures, source);
    if (!slot || !slot->record.color_initialized || is_depth(slot->record.desc.format)
        || impl_->next_view >= RendererLimits::ordered_views) {
        impl_->fail("readback_rgba", "invalid/uninitialized color target or view budget exhausted");
        return {};
    }
    const auto& desc = slot->record.desc;
    if (static_cast<UInt64>(desc.width) * desc.height * 4U > RendererLimits::maximum_upload_bytes) {
        impl_->fail("readback_rgba", "diagnostic readback exceeds byte limit"); return {};
    }
    if (desc.format != TextureFormat::rgba8 && desc.format != TextureFormat::bgra8) {
        impl_->fail("readback_rgba", "unsupported color readback format"); return {};
    }
    auto target = bgfx::createTexture2D(static_cast<UInt16>(desc.width), static_cast<UInt16>(desc.height),
        false, 1, physical_format(desc.format), BGFX_TEXTURE_BLIT_DST | BGFX_TEXTURE_READ_BACK);
    if (!bgfx::isValid(target)) { impl_->fail("readback_rgba", "readback texture allocation failed"); return {}; }
    bgfx::TextureRegion destination{};
    destination.handle = target;
    bgfx::TextureRegion origin{};
    origin.handle = slot->record.native;
    bgfx::blit(static_cast<bgfx::ViewId>(impl_->next_view++), destination, origin);
    auto current = impl_->advance_frame();
    std::vector<UInt8> pixels(static_cast<std::size_t>(desc.width) * desc.height * 4U);
    const auto ready = bgfx::read(destination, pixels.data());
    while (current < ready) current = impl_->advance_frame();
    // bgfx::frame() returns the frame it just submitted after waiting for the
    // preceding render frame. The read command retains pixels.data(), so the
    // ready frame must finish before we inspect or release this vector.
    impl_->advance_frame();
    bgfx::destroy(target);
    impl_->next_view = 0;
    if (desc.format == TextureFormat::bgra8)
        for (std::size_t i = 0; i < pixels.size(); i += 4) std::swap(pixels[i], pixels[i + 2]);
    return pixels;
}
void BgfxGpuDevice::release_window() noexcept
{
    if (impl_->transaction.active()) { impl_->transaction.poison(); return; }
    impl_->drain_retirements();
    if (impl_->in_pass) return;
    if (bgfx::isValid(impl_->window_framebuffer)) bgfx::destroy(impl_->window_framebuffer);
    if (bgfx::isValid(impl_->present_program)) bgfx::destroy(impl_->present_program);
    destroy(impl_->present_vertex); destroy(impl_->present_fragment);
    impl_->window = nullptr;
    impl_->window_framebuffer = BGFX_INVALID_HANDLE;
    impl_->present_program = BGFX_INVALID_HANDLE;
    impl_->present_vertex = {}; impl_->present_fragment = {};
    impl_->present_viewport = BGFX_INVALID_HANDLE;
    impl_->present_texture = BGFX_INVALID_HANDLE;
    impl_->window_width = impl_->window_height = 0;
    impl_->window_nwh = impl_->window_ndt = nullptr;
}

} // namespace zh::renderer
