#pragma once

#include "zh/renderer/contract.h"

#include <filesystem>
#include <memory>
#include <string>
#include <vector>

struct SDL_Window;

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

    bool supports_texture_format(TextureFormat, TextureDimension, bool sampled, bool render_target) const noexcept override;

    BufferHandle create_buffer(const BufferDesc&, std::string_view label) override;
    TextureHandle create_texture(const TextureDesc&, std::string_view label) override;
    std::optional<TextureFormat> describe_texture_format(TextureHandle) const noexcept override;
    SamplerHandle create_sampler(const SamplerDesc&, std::string_view label) override;
    ShaderHandle create_shader(const ShaderDesc&, std::string_view label) override;
    PipelineHandle create_pipeline(const PipelineKey&, std::string_view label) override;
    ValidationResult upload(const UploadDesc&, const void* bytes) override;
    ValidationResult upload_texture(const TextureUploadDesc&, const void* bytes) override;
    ValidationResult begin_pass(const RenderPassDesc&, std::string_view label) override;
    std::pair<UInt32,UInt32> active_pass_extent() const noexcept override;
    ValidationResult set_viewport(const ViewportDesc&) override;
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
    ValidationResult claim_window(SDL_Window* window);
    ValidationResult present(TextureHandle source) override;
    ValidationResult present_last();
    ValidationResult wait_idle();
    // Bounded diagnostic readback of an owned RGBA8 render target; never an
    // alternative draw path or source of game geometry/material decisions.
    std::vector<UInt8> readback_rgba(TextureHandle source);
    void release_window() noexcept;

private:
    class Impl;
    std::unique_ptr<Impl> impl_;
};

} // namespace zh::renderer
