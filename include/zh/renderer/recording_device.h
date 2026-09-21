#pragma once

#include "zh/renderer/contract.h"

#include <cstddef>
#include <memory>
#include <string>
#include <vector>

namespace zh::renderer {

struct ResourceCounts {
    std::size_t buffers = 0;
    std::size_t textures = 0;
    std::size_t samplers = 0;
    std::size_t shaders = 0;
    std::size_t pipelines = 0;

    std::size_t total() const noexcept { return buffers + textures + samplers + shaders + pipelines; }
    friend bool operator==(const ResourceCounts& left, const ResourceCounts& right) noexcept
    {
        return left.buffers == right.buffers && left.textures == right.textures
            && left.samplers == right.samplers && left.shaders == right.shaders && left.pipelines == right.pipelines;
    }
    friend bool operator!=(const ResourceCounts& left, const ResourceCounts& right) noexcept { return !(left == right); }
};

class RecordingGpuDevice final : public GpuDevice {
public:
    explicit RecordingGpuDevice(std::size_t pipeline_capacity = 256,
        std::size_t view_capacity = RendererLimits::ordered_views);
    ~RecordingGpuDevice() override;
    RecordingGpuDevice(RecordingGpuDevice&&) noexcept;
    RecordingGpuDevice& operator=(RecordingGpuDevice&&) noexcept;
    RecordingGpuDevice(const RecordingGpuDevice&) = delete;
    RecordingGpuDevice& operator=(const RecordingGpuDevice&) = delete;

    bool supports_texture_format(TextureFormat, TextureDimension, bool sampled, bool render_target) const noexcept override;
    void set_texture_format_supported(TextureFormat format, bool supported);
    void fail_next_texture_create();
    void fail_next_texture_upload();
    void fail_next_sampler_create();
    void fail_next_shader_create();
    void fail_next_pipeline_create();
    void fail_next_buffer_create();
    void fail_next_buffer_upload();
    void fail_buffer_upload_after(unsigned successful_uploads);
    void fail_next_draw();
    void fail_draw_after(unsigned successful_draws);

    BufferHandle create_buffer(const BufferDesc& desc, std::string_view label) override;
    TextureHandle create_texture(const TextureDesc& desc, std::string_view label) override;
    SamplerHandle create_sampler(const SamplerDesc& desc, std::string_view label) override;
    ShaderHandle create_shader(const ShaderDesc& desc, std::string_view label) override;
    PipelineHandle create_pipeline(const PipelineKey& key, std::string_view label) override;
    ValidationResult upload(const UploadDesc& desc, const void* bytes) override;
    ValidationResult upload_texture(const TextureUploadDesc& desc, const void* bytes) override;
    ValidationResult begin_pass(const RenderPassDesc& desc, std::string_view label) override;
    std::pair<UInt32,UInt32> active_pass_extent() const noexcept override;
    ValidationResult set_viewport(const ViewportDesc& desc) override;
    ValidationResult clear_viewport(const ViewportClearDesc& desc) override;
    ValidationResult draw(const DrawDesc& desc) override;
    ValidationResult end_pass() override;
    ValidationResult present(TextureHandle source) override;
    void destroy(BufferHandle handle) override;
    void destroy(TextureHandle handle) override;
    void destroy(SamplerHandle handle) override;
    void destroy(ShaderHandle handle) override;
    void destroy(PipelineHandle handle) override;

    const std::string& last_error() const noexcept override;
    std::string snapshot() const;
    std::size_t pipeline_count() const noexcept;
    ResourceCounts resource_counts() const noexcept;
    bool pass_active() const noexcept override;
    std::vector<UInt8> buffer_bytes(BufferHandle handle) const;
    std::vector<UInt8> last_draw_index_bytes() const;
    std::vector<UInt8> texture_bytes(TextureHandle handle, UInt32 mip_level = 0) const;
    SamplerDesc sampler_descriptor(SamplerHandle handle) const;
    PipelineDesc pipeline_descriptor(PipelineHandle handle) const;

    // Records an engine-level transition in the same ordered stream. This is
    // deliberately backend-neutral and is used for resize/recreation evidence.
    void record_marker(std::string_view marker) override;

private:
    class Impl;
    std::unique_ptr<Impl> impl_;
};

} // namespace zh::renderer
