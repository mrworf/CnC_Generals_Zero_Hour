/*
** Command & Conquer Generals Zero Hour(tm)
** Copyright 2025 Electronic Arts Inc.
** GPL-3.0-or-later
*/

// M28 extraction provenance:
// W3DDisplay::init, W3DDisplay::reset, W3DAssetManager loader ownership and
// DX8Wrapper initialization/destruction remain authoritative. This unit isolates
// their device-independent resource lifetime and binds drawing to the accepted
// renderer::GpuDevice seam. Full GameClient and physical-device work stay in
// M20/M22/M23.

#include "zh/original_resources.h"

#include "zh/original_data.h"
#include "zh/renderer/contract.h"

#include <algorithm>
#include <charconv>
#include <limits>
#include <set>
#include <utility>

namespace zh::original_resources {
namespace {

std::uint32_t parse_count(std::string_view text, std::string_view key, std::uint32_t maximum)
{
    const auto marker = key.find('=') == std::string_view::npos ? std::string(key) + "=" : std::string(key);
    const auto at = text.find(marker);
    if (at == std::string_view::npos) throw Error("missing resource field '" + std::string(key) + "'");
    const auto begin = text.data() + at + marker.size();
    const auto end = text.data() + text.size();
    std::uint32_t value = 0;
    const auto result = std::from_chars(begin, end, value);
    if (result.ec != std::errc{} || value == 0 || value > maximum)
        throw Error("invalid or oversized resource field '" + std::string(key) + "'");
    return value;
}

std::string_view as_text(const std::vector<std::uint8_t>& bytes)
{
    return {reinterpret_cast<const char*>(bytes.data()), bytes.size()};
}

} // namespace

struct CpuPresentation::Impl {
    explicit Impl(const original_data::LogicalFiles& source, renderer::GpuDevice& target)
        : files(source), device(target) {}

    struct Asset {
        ResourceKind kind{};
        std::string name;
        renderer::BufferHandle buffer;
        renderer::TextureHandle texture;
        std::uint32_t vertices = 0;
    };

    const original_data::LogicalFiles& files;
    renderer::GpuDevice& device;
    std::vector<Asset> assets;
    renderer::ShaderHandle vertex_shader;
    renderer::ShaderHandle fragment_shader;
    renderer::PipelineHandle pipeline;
    renderer::TextureHandle color;
    renderer::TextureHandle depth;
    OwnershipCounts ownership;
    std::string error;
    bool initialized = false;

    void destroy_all() noexcept
    {
        if (device.pass_active()) (void)device.end_pass();
        if (depth) device.destroy(depth);
        if (color) device.destroy(color);
        if (pipeline) device.destroy(pipeline);
        if (fragment_shader) device.destroy(fragment_shader);
        if (vertex_shader) device.destroy(vertex_shader);
        for (auto it = assets.rbegin(); it != assets.rend(); ++it) {
            if (it->texture) device.destroy(it->texture);
            if (it->buffer) device.destroy(it->buffer);
        }
        depth = {}; color = {}; pipeline = {}; fragment_shader = {}; vertex_shader = {};
        assets.clear();
        ownership = {};
        initialized = false;
    }

    bool fail(std::string message)
    {
        error = std::move(message);
        destroy_all();
        return false;
    }
};

CpuPresentation::CpuPresentation(const original_data::LogicalFiles& files, renderer::GpuDevice& recorder)
    : impl_(std::make_unique<Impl>(files, recorder)) {}

CpuPresentation::~CpuPresentation() { shutdown(); }

bool CpuPresentation::initialize(const std::vector<ResourceRequest>& resources, std::size_t fail_after_stage)
{
    if (impl_->initialized) return impl_->fail("CPU presentation is already initialized");
    impl_->error.clear();
    if (resources.empty() || resources.size() > 1024)
        return impl_->fail("CPU presentation requires 1..1024 resources");

    try {
        std::set<std::pair<ResourceKind, std::string>> unique;
        struct Staged { ResourceRequest request; std::vector<std::uint8_t> bytes; };
        std::vector<Staged> staged;
        staged.reserve(resources.size());
        for (const auto& request : resources) {
            if (request.logical_name.empty()) throw Error("resource logical name must not be empty");
            if (!unique.emplace(request.kind, request.logical_name).second)
                throw Error("duplicate original resource '" + request.logical_name + "'");
            staged.push_back({request, impl_->files.read(request.logical_name)});
        }

        std::size_t stage = 0;
        const auto checkpoint = [&] {
            ++stage;
            if (fail_after_stage == stage) throw Error("injected CPU presentation failure at stage " + std::to_string(stage));
        };

        impl_->ownership.loaders = 3;
        checkpoint();
        impl_->device.record_marker("original W3D CPU initialize loaders=model,texture,animation device=recording");
        impl_->ownership.scenes = 1;
        impl_->ownership.lights = 1;
        checkpoint();

        for (const auto& input : staged) {
            Impl::Asset asset{input.request.kind, input.request.logical_name};
            const auto text = as_text(input.bytes);
            switch (input.request.kind) {
            case ResourceKind::model: {
                if (text.rfind("W3DMODEL ", 0) != 0) throw Error("malformed W3D model '" + input.request.logical_name + "'");
                asset.vertices = parse_count(text, "vertices", 1'000'000);
                (void)parse_count(text, "indices", 3'000'000);
                asset.buffer = impl_->device.create_buffer(
                    {static_cast<std::uint64_t>(asset.vertices) * 12U, renderer::BufferUsage::vertex, false},
                    input.request.logical_name);
                if (!asset.buffer) throw Error(impl_->device.last_error());
                break;
            }
            case ResourceKind::texture: {
                if (input.bytes.size() < 8 || text.substr(0, 4) != "ZHWT")
                    throw Error("malformed W3D texture '" + input.request.logical_name + "'");
                const std::uint32_t width = input.bytes[4] | (static_cast<std::uint32_t>(input.bytes[5]) << 8U);
                const std::uint32_t height = input.bytes[6] | (static_cast<std::uint32_t>(input.bytes[7]) << 8U);
                if (width == 0 || height == 0 || width > 4096 || height > 4096 ||
                    input.bytes.size() != 8ULL + static_cast<std::uint64_t>(width) * height * 4ULL)
                    throw Error("invalid or oversized W3D texture '" + input.request.logical_name + "'");
                renderer::TextureDesc desc;
                desc.width = width; desc.height = height;
                asset.texture = impl_->device.create_texture(desc, input.request.logical_name);
                if (!asset.texture) throw Error(impl_->device.last_error());
                renderer::TextureUploadDesc upload{asset.texture, width, height, width * 4U,
                    static_cast<std::uint64_t>(width) * height * 4U};
                if (!impl_->device.upload_texture(upload, input.bytes.data() + 8)) throw Error(impl_->device.last_error());
                break;
            }
            case ResourceKind::animation:
                if (text.rfind("W3DANIM ", 0) != 0) throw Error("malformed W3D animation '" + input.request.logical_name + "'");
                (void)parse_count(text, "frames", 100000);
                break;
            }
            impl_->assets.push_back(std::move(asset));
            impl_->ownership.assets = impl_->assets.size();
            checkpoint();
        }

        impl_->vertex_shader = impl_->device.create_shader(
            {renderer::ShaderStage::vertex, "original-w3d-cpu.vert", 0, 0}, "original W3D CPU vertex");
        impl_->fragment_shader = impl_->device.create_shader(
            {renderer::ShaderStage::fragment, "original-w3d-cpu.frag", 0, 0}, "original W3D CPU fragment");
        if (!impl_->vertex_shader || !impl_->fragment_shader) throw Error(impl_->device.last_error());
        renderer::PipelineDesc pipeline_desc;
        pipeline_desc.vertex_shader = impl_->vertex_shader;
        pipeline_desc.fragment_shader = impl_->fragment_shader;
        impl_->pipeline = impl_->device.create_pipeline(renderer::PipelineKey(pipeline_desc), "original W3D CPU pipeline");
        renderer::TextureDesc color_desc;
        color_desc.width = 32; color_desc.height = 32; color_desc.render_target = true;
        impl_->color = impl_->device.create_texture(color_desc, "original W3D CPU color");
        auto depth_desc = color_desc;
        depth_desc.format = renderer::TextureFormat::depth24_stencil8;
        impl_->depth = impl_->device.create_texture(depth_desc, "original W3D CPU depth");
        if (!impl_->pipeline || !impl_->color || !impl_->depth) throw Error(impl_->device.last_error());
        checkpoint();
        impl_->initialized = true;
        return true;
    } catch (const std::exception& exception) {
        return impl_->fail(exception.what());
    }
}

bool CpuPresentation::record_scene()
{
    if (!impl_->initialized) { impl_->error = "CPU presentation is not initialized"; return false; }
    auto model = std::find_if(impl_->assets.begin(), impl_->assets.end(),
        [](const Impl::Asset& asset) { return asset.kind == ResourceKind::model; });
    if (model == impl_->assets.end()) { impl_->error = "original W3D scene has no model resource"; return false; }
    renderer::RenderPassDesc pass;
    pass.color_targets[0] = impl_->color; pass.color_target_count = 1; pass.depth_target = impl_->depth;
    pass.width = 32; pass.height = 32;
    impl_->device.record_marker("original W3D scene light=default assets=" + std::to_string(impl_->assets.size()));
    if (!impl_->device.begin_pass(pass, "original W3D CPU scene")) { impl_->error = impl_->device.last_error(); return false; }
    renderer::DrawDesc draw;
    draw.pipeline = impl_->pipeline; draw.vertex_buffer = model->buffer; draw.vertex_or_index_count = model->vertices;
    if (!impl_->device.draw(draw)) {
        impl_->error = impl_->device.last_error();
        (void)impl_->device.end_pass();
        return false;
    }
    if (!impl_->device.end_pass()) { impl_->error = impl_->device.last_error(); return false; }
    return true;
}

void CpuPresentation::shutdown() noexcept { impl_->destroy_all(); }
bool CpuPresentation::ready() const noexcept { return impl_->initialized; }
bool CpuPresentation::supports(Capability capability) const noexcept
{
    return capability == Capability::cpu_resources || capability == Capability::recorded_drawing;
}
OwnershipCounts CpuPresentation::counts() const noexcept { return impl_->ownership; }
std::string_view CpuPresentation::last_error() const noexcept { return impl_->error; }
const char* provider_cpu_identity() noexcept { return "OriginalCpuPresentation.cpp"; }

} // namespace zh::original_resources
