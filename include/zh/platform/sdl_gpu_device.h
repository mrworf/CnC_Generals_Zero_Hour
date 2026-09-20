#pragma once

#include "zh/renderer/contract.h"

#include <filesystem>
#include <memory>
#include <string>

namespace zh::renderer {

struct SdlGpuOptions {
    std::filesystem::path shader_root;
    bool debug = true;
    std::string driver = "vulkan";
};

struct SdlGpuCapabilities {
    std::string sdl_version;
    std::string backend;
    std::string device_name;
    std::string driver_name;
    std::string driver_version;
    bool debug_enabled = false;
    bool bc1 = false;
    bool bc2 = false;
    bool bc3 = false;
};

class SdlGpuDevice final : public GpuDevice {
public:
    explicit SdlGpuDevice(SdlGpuOptions options);
    ~SdlGpuDevice() override;
    SdlGpuDevice(SdlGpuDevice&&) noexcept;
    SdlGpuDevice& operator=(SdlGpuDevice&&) noexcept;
    SdlGpuDevice(const SdlGpuDevice&) = delete;
    SdlGpuDevice& operator=(const SdlGpuDevice&) = delete;

    BufferHandle create_buffer(const BufferDesc&, std::string_view label) override;
    TextureHandle create_texture(const TextureDesc&, std::string_view label) override;
    SamplerHandle create_sampler(const SamplerDesc&, std::string_view label) override;
    ShaderHandle create_shader(const ShaderDesc&, std::string_view label) override;
    PipelineHandle create_pipeline(const PipelineKey&, std::string_view label) override;
    ValidationResult upload(const UploadDesc&, const void* bytes) override;
    ValidationResult begin_pass(const RenderPassDesc&, std::string_view label) override;
    ValidationResult draw(const DrawDesc&) override;
    ValidationResult end_pass() override;
    void destroy(BufferHandle) override;
    void destroy(TextureHandle) override;
    void destroy(SamplerHandle) override;
    void destroy(ShaderHandle) override;
    void destroy(PipelineHandle) override;
    const std::string& last_error() const noexcept override;
    bool pass_active() const noexcept override;
    void record_marker(std::string_view marker) override;

    const SdlGpuCapabilities& capabilities() const noexcept;

private:
    class Impl;
    std::unique_ptr<Impl> impl_;
};

} // namespace zh::renderer
