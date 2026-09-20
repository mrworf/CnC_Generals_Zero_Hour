#pragma once

#include "zh/renderer/contract.h"

#include <cstddef>
#include <memory>
#include <string>
#include <vector>

namespace zh::renderer {

class RecordingGpuDevice final : public GpuDevice {
public:
    explicit RecordingGpuDevice(std::size_t pipeline_capacity = 256);
    ~RecordingGpuDevice() override;
    RecordingGpuDevice(RecordingGpuDevice&&) noexcept;
    RecordingGpuDevice& operator=(RecordingGpuDevice&&) noexcept;
    RecordingGpuDevice(const RecordingGpuDevice&) = delete;
    RecordingGpuDevice& operator=(const RecordingGpuDevice&) = delete;

    BufferHandle create_buffer(const BufferDesc& desc, std::string_view label) override;
    TextureHandle create_texture(const TextureDesc& desc, std::string_view label) override;
    SamplerHandle create_sampler(const SamplerDesc& desc, std::string_view label) override;
    ShaderHandle create_shader(const ShaderDesc& desc, std::string_view label) override;
    PipelineHandle create_pipeline(const PipelineKey& key, std::string_view label) override;
    ValidationResult upload(const UploadDesc& desc, const void* bytes) override;
    ValidationResult begin_pass(const RenderPassDesc& desc, std::string_view label) override;
    ValidationResult draw(const DrawDesc& desc) override;
    ValidationResult end_pass() override;
    void destroy(BufferHandle handle) override;
    void destroy(TextureHandle handle) override;
    void destroy(SamplerHandle handle) override;
    void destroy(ShaderHandle handle) override;
    void destroy(PipelineHandle handle) override;

    const std::string& last_error() const noexcept;
    std::string snapshot() const;
    std::size_t pipeline_count() const noexcept;
    bool pass_active() const noexcept;
    std::vector<UInt8> buffer_bytes(BufferHandle handle) const;

    // Records an engine-level transition in the same ordered stream. This is
    // deliberately backend-neutral and is used for resize/recreation evidence.
    void record_marker(std::string_view marker);

private:
    class Impl;
    std::unique_ptr<Impl> impl_;
};

} // namespace zh::renderer
